#pragma once

#include <metrics/registry.h>
#include <metrics/sink.h>

#include <memory>

namespace Metrics {
    namespace Prometheus {
        METRICS_EXPORT std::string serialize(std::shared_ptr<IRegistry> registry);
        METRICS_EXPORT std::shared_ptr<IOnDemandSink> createPushGatewaySink(std::string host, uint16_t port, std::string job, std::string instance);
    }
}
