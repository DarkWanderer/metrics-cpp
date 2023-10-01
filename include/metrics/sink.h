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

    METRICS_EXPORT std::shared_ptr<IOnDemandSink> createStatsdUdpSink(std::string host, uint16_t port);
    METRICS_EXPORT std::shared_ptr<IOnDemandSink> createStatsdTcpSink(std::string host, uint16_t port);

    //std::shared_ptr<IOnDemandSink> createPrometheusPushGatewaySink(std::shared_ptr<IRegistry> registry = nullptr);
    //std::shared_ptr<IRegistrySink> createPrometheusHttpServerSink(std::shared_ptr<IRegistry> registry = nullptr);
}
