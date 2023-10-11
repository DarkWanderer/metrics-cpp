#pragma once

#include <metrics/registry.h>
#include <metrics/sink.h>

#include <memory>

namespace Metrics {
    namespace Prometheus {
        METRICS_EXPORT std::string serialize(std::shared_ptr<IRegistry> registry);
        std::shared_ptr<IOnDemandSink> createPrometheusPushGatewaySink(std::shared_ptr<IRegistry> registry = nullptr);

    }
}
