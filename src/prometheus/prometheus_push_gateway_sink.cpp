#include <metrics/prometheus.h>
#include <metrics/sink.h>

//#pragma warning(push, 1)
#include <boost/asio.hpp>
//#pragma warning(pop)

using namespace std;
namespace asio = boost::asio;
using asio::ip::tcp;

namespace Metrics {
    namespace Prometheus {
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
