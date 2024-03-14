#pragma once

#include <metrics/registry.h>
#include <metrics/sink.h>

#include <memory>

namespace Metrics {
    namespace Prometheus {
        METRICS_EXPORT std::string serialize(std::shared_ptr<IRegistry> registry);
        METRICS_EXPORT std::shared_ptr<IOnDemandSink> createPushGatewaySink(std::string host, std::string port, std::string job, std::string instance);
        METRICS_EXPORT std::shared_ptr<IRegistrySink> createPrometheusHttpServerSink(std::shared_ptr<IRegistry> registry, std::string address, std::string port);
    }
}
