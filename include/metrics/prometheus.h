#pragma once
#include <metrics/registry.h>

#include <string>

namespace Metrics
{
    METRICS_EXPORT std::string serializeToPrometheus(const IRegistry& registry);
}
