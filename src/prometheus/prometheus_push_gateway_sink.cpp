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
            const string m_url;
        public:
            PrometheusOnDemandPushGatewaySink(const string& url) : m_url(url) {};

            void send(shared_ptr<IRegistry> registry) override {

            }
        };

        shared_ptr<IOnDemandSink> createPushGatewaySink(const string& url)
        {
            return make_shared<PrometheusOnDemandPushGatewaySink>(url);
        }
    }
}
