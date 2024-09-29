#include <metrics/sink.h>
#include <metrics/prometheus.h>
#include <metrics/statsd.h>

#include <boost/url/parse.hpp>

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
            if (scheme.find("pushgateway+") == 0) {
                std::string url_truncated = url_string.substr(strlen("pushgateway+"));
                return Prometheus::createPushGatewaySink(url_truncated);
            }
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
            if (scheme == "prometheus+http")
                return Prometheus::createPrometheusHttpServerSink(registry, url.host(), url.port());
        }
        catch (std::exception&) 
        {
        }

        return std::shared_ptr<IRegistrySink>();
    }
}
