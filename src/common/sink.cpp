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
        const auto& url = *ru;

        try 
        {
            const auto& scheme = url.scheme();
            if (scheme == "statsd+udp")
                return Statsd::createUdpSink(url.host(), url.port());
            if (scheme == "statsd+tcp")
                return Statsd::createTcpSink(url.host(), url.port());
            if (scheme == "pushgateway+http" || scheme == "pushgateway+https")
                return Prometheus::createPushGatewaySink(url.host(), url.port(), "job", "instance");
        }
        catch (std::exception&) 
        {

        }
        return std::shared_ptr<IOnDemandSink>();
    }

    METRICS_EXPORT std::shared_ptr<IRegistrySink> createRegistrySink(std::shared_ptr<IRegistry> registry, const std::string& url_string)
    {
        auto ru = boost::urls::parse_uri_reference(url_string);
        if (!ru)
            return nullptr;
        const auto& url = *ru;
        try 
        {
            const auto& scheme = url.scheme();
        }
        catch (std::exception&) 
        {
        }

        if (scheme == "prometheus+http")
            return Prometheus::createPrometheusHttpServerSink(registry, url.host(), url.port());

        return std::shared_ptr<IRegistrySink>();
    }
}
