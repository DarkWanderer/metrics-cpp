#include <metrics/statsd.h>
#include <metrics/sink.h>

#pragma warning(push, 1)
#include <boost/asio.hpp>
#pragma warning(pop)

#include <iostream>
#include <sstream>
#include <functional>

using namespace std;
namespace asio = boost::asio;
using asio::ip::udp;
using asio::ip::tcp;

namespace Metrics {
    namespace Statsd {
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

            void serialize(std::shared_ptr<IRegistry> registry)
            {
                auto names = registry->metricNames();
                for (const auto& name : names)
                {
                    auto& group = registry->getGroup(name);
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

        string serialize(std::shared_ptr<IRegistry> registry)
        {
            StatsdSerializer s;
            s.serialize(registry);
            return s.str();
        }

        class StatsdOnDemandUdpSink : public IOnDemandSink {
        private:
            asio::io_service m_io_service;
            string m_host;
            string m_port;
            const size_t m_max_packet_size;
        public:
            StatsdOnDemandUdpSink(const string& host, const string& port) :
                m_io_service(),
                m_host(host),
                m_port(port),
                m_max_packet_size(1000)
            {
            }

            void send(shared_ptr<IRegistry> registry) override
            {
                // Resolve name
                udp::resolver resolver(m_io_service);
                udp::resolver::query query(m_host, m_port);
                auto endpoint_iterator = resolver.resolve(query);
                auto endpoint = udp::endpoint(*endpoint_iterator); // Result guaranteed to be valid
                udp::socket socket(m_io_service);
                socket.open(endpoint.protocol());

                auto send = [&](const char* begin, size_t count) {
                    socket.send_to(asio::buffer(begin, count), endpoint);
                };

                // Serialize input into string
                auto data = serialize(registry);

                // Send 
                auto start = data.begin();
                auto end = data.begin();

                while (end < data.end())
                {
                    auto new_end = find(end + 1, data.end(), '\n');
                    int64_t old_dist = end - start;
                    int64_t new_dist = new_end - start;
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

        shared_ptr<IOnDemandSink> createUdpSink(const string& host, const string& port)
        {
            return make_shared<StatsdOnDemandUdpSink>(host, port);
        }

        class StatsdOnDemandTcpSink : public IOnDemandSink {
        private:
            asio::io_service m_io_service;
            string m_host;
            string m_port;
        public:
            StatsdOnDemandTcpSink(const string& host, const string& port) :
                m_io_service(),
                m_host(host),
                m_port(port)
            {
            }

            void send(shared_ptr<IRegistry> registry) override
            {
                // Serialize input into string
                auto data = serialize(registry);

                // Prepare network structures
                tcp::socket socket(m_io_service);
                tcp::resolver resolver(m_io_service);

                // Resolve to one or more endpoints
                tcp::resolver::query query(m_host, m_port);
                auto endpoint_iterator = resolver.resolve(query);
                asio::connect(socket, endpoint_iterator);

                // Send
                socket.send(asio::buffer(data));
            }
        };

        shared_ptr<IOnDemandSink> createTcpSink(const string& host, const string& port)
        {
            return make_shared<StatsdOnDemandTcpSink>(host, port);
        }
    }
}
