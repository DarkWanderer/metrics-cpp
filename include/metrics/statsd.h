#pragma once

#include <metrics/registry.h>
#include <metrics/sink.h>

#include <memory>

namespace Metrics {
    namespace Statsd {
        METRICS_EXPORT std::string serialize(std::shared_ptr<IRegistry> registry);
        METRICS_EXPORT std::shared_ptr<IOnDemandSink> createUdpSink(const std::string& host, const std::string& port);
        METRICS_EXPORT std::shared_ptr<IOnDemandSink> createTcpSink(const std::string& host, const std::string& port);
    }
}
