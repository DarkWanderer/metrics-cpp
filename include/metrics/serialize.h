#pragma once
#include <metrics/registry.h>

#include <string>

namespace Metrics
{
    METRICS_EXPORT std::string serializePrometheus(const IRegistry& registry);
    METRICS_EXPORT std::string serializeJson(const IRegistry& registry);
    METRICS_EXPORT std::string serializeJsonl(const IRegistry& registry);
    METRICS_EXPORT std::string serializeStatsd(const IRegistry& registry);
}
