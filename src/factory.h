#pragma once

#include <metrics/metric.h>

#include <memory>
#include <vector>

namespace Metrics {
    class IMetric;

    std::shared_ptr<IGaugeValue> createGauge();
    std::shared_ptr<ICounterValue> createCounter();
    std::shared_ptr<IHistogram> createHistogram(const std::vector<double>& bounds);
}
