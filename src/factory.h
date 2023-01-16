#pragma once

#include <memory>

namespace Metrics {
    class IMetric;

    std::shared_ptr<IMetric> createGauge();
    std::shared_ptr<IMetric> createCounter();
}
