#include <metrics/sink.h>
#include <metrics/prometheus.h>
#include <metrics/statsd.h>

#include "url.hpp"

namespace Metrics {
    IOnDemandSink::~IOnDemandSink() {}
    IRegistrySink::~IRegistrySink() {}

    METRICS_EXPORT std::shared_ptr<IOnDemandSink> createOnDemandSink(const std::string& url_string)
    {
        try 
        {
            Url url(url_string);
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
        Url url(url_string);
        const auto& scheme = url.scheme();

        //if (scheme == "prometheus")
            //return createPrometheusHttpServerSink()

        return std::shared_ptr<IRegistrySink>();
    }
}
