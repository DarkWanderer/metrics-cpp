#pragma once

#include <metrics/registry.h>
#include <metrics/sink.h>

#include <memory>

namespace Metrics {
    namespace Statsd {
        METRICS_EXPORT std::string serialize(std::shared_ptr<IRegistry> registry);
        std::shared_ptr<IOnDemandSink> createUdpSink(std::shared_ptr<IRegistry> registry = nullptr);
        std::shared_ptr<IOnDemandSink> createTcpSink(std::shared_ptr<IRegistry> registry = nullptr);
    }
}
