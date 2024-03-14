#include <metrics/sink.h>
#include <metrics/prometheus.h>
#include <metrics/statsd.h>

#include <boost/url/parse.hpp>

using boost::urls::parse_absolute_uri;

namespace Metrics {
    IOnDemandSink::~IOnDemandSink() {}
    IRegistrySink::~IRegistrySink() {}

    METRICS_EXPORT std::shared_ptr<IOnDemandSink> createOnDemandSink(const std::string& url_string)
    {
        auto ru = boost::urls::parse_uri_reference(url_string);
        if (!ru)
            return nullptr;
        auto url = *ru;

        try 
        {
            const auto& scheme = url.scheme();
            auto port = std::stoi(url.port());
            if (scheme == "statsd+udp")
                return Statsd::createUdpSink(url.host(), port);
            if (scheme == "statsd+tcp")
                return Statsd::createTcpSink(url.host(), port);
            if (scheme == "pushgateway+http" || scheme == "pushgateway+https")
                return Prometheus::createPushGatewaySink(url.host(), port, "job", "instance");
        }
        catch (std::exception&) 
        {

        }
        return std::shared_ptr<IOnDemandSink>();
    }

    METRICS_EXPORT std::shared_ptr<IRegistrySink> createIRegistrySink(const std::string& url_string)
    {
        auto ru = boost::urls::parse_uri_reference(url_string);
        if (!ru)
            return nullptr;
        auto url = *ru;
        try 
        {
        }
        catch (std::exception&) 
        {
        }

        //if (scheme == "prometheus")
            //return createPrometheusHttpServerSink()

        return std::shared_ptr<IRegistrySink>();
    }
}
