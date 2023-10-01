#include <metrics/serialize.h>
#include <metrics/sink.h>

#include <asio.hpp>

#include <iostream>
#include <sstream>
#include <functional>
#include <string_view>

using namespace std;
using asio::ip::udp;
using asio::ip::tcp;

namespace Metrics
{
    void write(ostream& os, const string& name, const Labels& labels)
    {
        os << name;
        for (auto kv = labels.cbegin(); kv != labels.cend(); kv++)
            os << "," << kv->first << "=" << kv->second;
    }

    struct StatsdSerializer
    {
        stringstream os;

        string str() {
            return os.str();
        }

        void serialize(const IRegistry& registry)
        {
            auto names = registry.metricNames();
            for (const auto& name : names)
            {
                auto& group = registry.getGroup(name);
                auto metrics = group.metrics();
                for (const auto& kv : metrics) {
                    const auto& labels = kv.first;
                    const auto& metric = kv.second;
                    switch (metric->type())
                    {
                    case TypeCode::Counter:
                        write(os, name, labels);
                        os << "|" << static_pointer_cast<ICounterValue>(metric)->value() << "|c" << endl;
                        break;
                    case TypeCode::Gauge:
                        write(os, name, labels);
                        os << "|" << static_pointer_cast<IGaugeValue>(metric)->value() << "|g" << endl;
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    };

    string serializeStatsd(const IRegistry& registry)
    {
        StatsdSerializer s;
        s.serialize(registry);
        return s.str();
    }

    class StatsdOnDemandUdpSink : public IOnDemandSink {
    private:
        asio::io_service m_io_service;
        udp::socket m_socket;
        string m_host;
        uint16_t m_port;
        const size_t m_max_packet_size;
    public:
        StatsdOnDemandUdpSink(string host, uint16_t port) :
            m_io_service(),
            m_socket(m_io_service),
            m_host(host),
            m_port(port),
            m_max_packet_size(1000)
        { 
            m_socket.open(udp::v4());
        }

        void send(shared_ptr<IRegistry> registry) override
        {
            // Resolve name
            udp::resolver resolver(m_io_service);
            udp::resolver::query query(m_host, to_string(m_port));
            auto endpoint_iterator = resolver.resolve(query);
            auto endpoint = udp::endpoint(*endpoint_iterator); // Result guaranteed to be valid

            auto send = [&](const char* begin, size_t count) { 
                auto sent = m_socket.send_to(asio::buffer(begin, count), endpoint);
            };

            // Serialize input into string
            auto data = serializeStatsd(*registry);
            
            // Send 
            auto start = data.begin();
            auto end = data.begin();

            while (end < data.end())
            {
                auto new_end = find(end + 1, data.end(), '\n');
                size_t old_dist = end - start;
                size_t new_dist = new_end - start;
                if (new_dist > m_max_packet_size && old_dist > 0)
                {
                    send(&*start, old_dist);
                    start = end;
                }
                else if (new_dist > m_max_packet_size && old_dist <= 0)
                {
                    // Still try to send - better than nothing
                    send(&*start, new_dist);
                    start = new_end;
                    end = new_end;
                }
                else if (new_end == data.end())
                {
                    send(&*start, new_dist);
                    break;
                }
                else
                {
                    end = new_end;
                }
            }
        }
    };

    shared_ptr<IOnDemandSink> Metrics::createStatsdUdpSink(string host, uint16_t port)
    {
        return make_shared<StatsdOnDemandUdpSink>(host, port);
    }

    class StatsdOnDemandTcpSink : public IOnDemandSink {
    private:
        asio::io_service m_io_service;
        string m_host;
        uint16_t m_port;
        const size_t m_max_packet_size;
    public:
        StatsdOnDemandTcpSink(string host, uint16_t port) :
            m_io_service(),
            m_host(host),
            m_port(port),
            m_max_packet_size(1000)
        {
        }

        void send(shared_ptr<IRegistry> registry) override
        {
            // Serialize input into string
            auto data = serializeStatsd(*registry);

            // Prepare network structures
            tcp::socket socket(m_io_service);
            tcp::resolver resolver(m_io_service);

            // Resolve to one or more endpoints
            tcp::resolver::query query(m_host, to_string(m_port));
            auto endpoint_iterator = resolver.resolve(query);
            asio::connect(socket, endpoint_iterator);

            // Send 
            socket.send(asio::buffer(data));
        }
    };

    shared_ptr<IOnDemandSink> Metrics::createStatsdTcpSink(string host, uint16_t port)
    {
        return make_shared<StatsdOnDemandTcpSink>(host, port);
    }
}
