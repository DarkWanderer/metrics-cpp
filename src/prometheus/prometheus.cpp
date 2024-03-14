#include <metrics/prometheus.h>
#include <metrics/sink.h>

#pragma warning(push, 1)
#include <boost/asio.hpp>
#pragma warning(pop)

#include <iostream>
#include <sstream>

using namespace std;
namespace asio = boost::asio;
using asio::ip::tcp;

namespace Metrics {
    namespace Prometheus {
        const char* typeString(TypeCode type) {
            switch (type)
            {
            case TypeCode::Gauge:
                return "gauge";
                break;
            case TypeCode::Counter:
                return "counter";
                break;
            case TypeCode::Summary:
                return "summary";
                break;
            case TypeCode::Histogram:
                return "histogram";
                break;
            default:
                break;
            }
            return "unknown";
        }

        ostream& operator<<(ostream& os, const Labels& labels)
        {
            bool opened = false;
            for (auto kv = labels.cbegin(); kv != labels.cend(); kv++)
            {
                os << (opened ? "," : "{") << kv->first << "=\"" << kv->second << '"';
                opened = true;
            }
            if (opened)
                os << '}';
            return os;
        }

        void serialize(ostream& os, const string& name, const Labels& labels, const ISummary& summary)
        {
            for (auto& value : summary.values())
            {
                os << name << '{';
                for (auto kv = labels.cbegin(); kv != labels.cend(); kv++)
                {
                    os << kv->first << "=\"" << kv->second << '"' << ',';
                }
                os << "quantile=\"" << value.first << "\"} " << value.second << endl;
            }
            os << name << "_sum" << labels << ' ' << summary.sum() << endl;
            os << name << "_count" << labels << ' ' << summary.count() << endl;
        }

        void serialize(ostream& os, const string& name, const Labels& labels, const IHistogram& histogram)
        {
            for (auto& value : histogram.values())
            {
                os << name << '{';
                for (auto kv = labels.cbegin(); kv != labels.cend(); kv++)
                {
                    os << kv->first << "=\"" << kv->second << '"' << ',';
                }
                os << "le=\"" << value.first << "\"} " << value.second << endl;
            }
            os << name << "_sum" << labels << ' ' << histogram.sum() << endl;
            os << name << "_count" << labels << ' ' << histogram.count() << endl;
        }

        void serialize(ostream& os, const string& name, const Labels& labels, const std::shared_ptr<IMetric> metric)
        {
            if (!metric)
                return;
            switch (metric->type())
            {
            case TypeCode::Counter:
                // os << "# TYPE " << name << " counter" << endl;
                os << name << labels << ' ' << static_pointer_cast<ICounterValue>(metric)->value() << endl;
                break;
            case TypeCode::Gauge:
                // os << "# TYPE " << name << " gauge" << endl;
                os << name << labels << ' ' << static_pointer_cast<IGaugeValue>(metric)->value() << endl;
                break;
            case TypeCode::Summary:
                serialize(os, name, labels, *static_pointer_cast<ISummary>(metric));
                break;
            case TypeCode::Histogram:
                serialize(os, name, labels, *static_pointer_cast<IHistogram>(metric));
                break;
            }
        }

        string serialize(std::shared_ptr<IRegistry> registry)
        {
            stringstream out;
            auto names = registry->metricNames();
            for (const auto& name : names)
            {
                auto& group = registry->getGroup(name);
                if (!group.description().empty())
                    out << "# HELP " << name << " " << group.description() << endl;
                out << "# TYPE " << name << " " << typeString(group.type()) << endl;
                auto metrics = group.metrics();
                for (const auto& metric : metrics)
                    serialize(out, name, metric.first, metric.second);
            }
            return out.str();
        }

        class PrometheusHttpServerSink : public IRegistrySink {
        private:
            shared_ptr<IRegistry> m_registry;
            shared_ptr<tcp::acceptor> m_acceptor;
            thread m_thread;
            asio::io_service m_io_service;

            void run()
            {
                // adds some work to the io_service
                accept();
                m_io_service.run();
            }

            void accept()
            {
                auto socket = make_shared<tcp::socket>(m_io_service);
                m_acceptor->async_accept(*socket, bind(&PrometheusHttpServerSink::handle, this, socket));
            }

            void handle(shared_ptr<tcp::socket> socket)
            {
                asio::streambuf response;
                std::ostream res_stream(&response);


                res_stream << "HTTP/1.0 200 OK" << endl
                    << "Content-Type: text/html; charset=UTF-8" << endl
                    << "Content-Length: 5" << endl << endl
                    << "Hello";
                asio::write(*socket, response);
                socket->close();
                accept();
            }
        public:
            PrometheusHttpServerSink(shared_ptr<IRegistry> registry, const string& address, const string& port)
            {
                tcp::resolver resolver(m_io_service);
                tcp::resolver::query query(address, port);
                auto endpoint_iterator = resolver.resolve(query);
                m_registry = registry;
                m_acceptor = make_shared<tcp::acceptor>(m_io_service, *endpoint_iterator.begin());
                m_thread = thread(&PrometheusHttpServerSink::run, this);
            };

            virtual ~PrometheusHttpServerSink() {
                m_io_service.stop();
                m_thread.join();
            }

            virtual shared_ptr<IRegistry> registry() const override {
                return m_registry;
            };

        };

        shared_ptr<IRegistrySink> createPrometheusHttpServerSink(shared_ptr<IRegistry> registry, const string& address, const string& port)
        {
            if (!registry)
                registry = createRegistry();

            return make_shared<PrometheusHttpServerSink>(registry, address, port);
        }

        class PrometheusOnDemandPushGatewaySink : public IOnDemandSink
        {
        private:
            const string m_host;
            const string m_port;
            const string m_job;
            const string m_instance;
        public:
            PrometheusOnDemandPushGatewaySink(const string& host, const string& port, string job, string instance) :
                m_host(host), m_port(port), m_job(job), m_instance(instance)
            {};

            void send(shared_ptr<IRegistry> registry) override
            {
                auto data = serialize(registry);

                stringstream request;
                request << "POST /metrics/job/" << m_job << "/instance/" << m_instance << " HTTP/1.1" << endl;
                request << "Host:" << m_host << endl;
                request << "User-Agent: metrics-cpp/1.0\r\n";
                request << "Accept: */*\r\n";
                request << "Content-Length: " << data.size() << endl;
                request << "Connection: close" << endl << endl;
                request << data;

                auto dbg = request.str();

                asio::io_service io_service;
                tcp::socket socket(io_service);
                tcp::resolver resolver(io_service);
                tcp::resolver::query query(m_host, m_port);
                auto endpoint_iterator = resolver.resolve(query);
                asio::connect(socket, endpoint_iterator);
                socket.send(asio::buffer(request.str()));

                // Receive response
                asio::streambuf response;
                asio::read_until(socket, response, "\n");

                // Parse response
                std::istream response_stream(&response);
                std::string http_version;
                unsigned int status_code;
                response_stream >> http_version;
                response_stream >> status_code;

                std::string status_message;
                std::getline(response_stream, status_message);

                if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
                    throw std::logic_error("HTTP response expected");
                }
                if (status_code != 200) {
                    stringstream ss;
                    ss << "Response code: " << status_code << " reason: " << status_message;
                    throw std::logic_error(ss.str());
                }
            }
        };

        shared_ptr<IOnDemandSink> createPushGatewaySink(const string& host, const string& port, string job, std::string instance)
        {
            return make_shared<PrometheusOnDemandPushGatewaySink>(host, port, job, instance);
        }
    }
}
