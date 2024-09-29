#pragma once

#include <metrics/registry.h>
#include <metrics/sink.h>

#include <memory>

namespace Metrics {
    namespace Prometheus {
        METRICS_EXPORT std::string serialize(std::shared_ptr<IRegistry> registry);
        METRICS_EXPORT std::shared_ptr<IOnDemandSink> createPushGatewaySink(const std::string& url);
        METRICS_EXPORT std::shared_ptr<IRegistrySink> createPrometheusHttpServerSink(std::shared_ptr<IRegistry> registry, const std::string& address, const std::string& port);
    }
}
