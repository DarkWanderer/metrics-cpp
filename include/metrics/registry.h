#pragma once

#include <metrics_export.h>
#include <metrics/counter.h>
#include <metrics/gauge.h>
#include <metrics/histogram.h>
#include <metrics/summary.h>

#include <vector>

namespace Metrics
{
    class IRegistry
    {
    public:
        virtual Gauge getGauge(const Key &key) = 0;
        virtual Counter getCounter(const Key &key) = 0;
        virtual Histogram getHistogram(const Key& key, const std::vector<double>& bounds = {}) = 0;
    };

    METRICS_EXPORT IRegistry &defaultRegistry();
    METRICS_EXPORT std::unique_ptr<IRegistry> createRegistry();
}
