#pragma once

#include <metrics_export.h>

#include <memory>
#include <string>

namespace Metrics {
    class IMetric;
    class IRegistry;

    struct IOnDemandSink {
        virtual void send(std::shared_ptr<IRegistry> registry) = 0;
        virtual ~IOnDemandSink() = 0;
    };

    struct IRegistrySink {
        virtual std::shared_ptr<IRegistry> registry() const = 0;
        virtual ~IRegistrySink() = 0;
    };

    /// Creates sink based on provided url. Specific implementation called depends on URL schema
    /// Examples:
    ///  statsd+udp://localhost:1234 
    ///  statsd+tcp://localhost:1234 
    ///  pushgateway+https://prometheus.push.gateway/job/123/instance/123
    METRICS_EXPORT std::shared_ptr<IOnDemandSink> createOnDemandSink(const std::string& url);

    /// Creates sink based on provided url. Specific implementation called depends on URL schema
    /// Examples:
    ///  statsd+udp://localhost:1234 
    ///  statsd+tcp://localhost:1234 
    ///  pushgateway+https://prometheus.push.gateway/job/123/instance/123
    METRICS_EXPORT std::shared_ptr<IRegistrySink> createIRegistrySink(const std::string& url);
}
