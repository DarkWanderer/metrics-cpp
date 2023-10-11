#pragma once
#include <metrics/registry.h>

#include <string>

namespace Metrics {
    namespace Json {
        METRICS_EXPORT std::string serializeJson(std::shared_ptr<IRegistry> registry);
        METRICS_EXPORT std::string serializeJsonl(std::shared_ptr<IRegistry> registry);
    }
}
