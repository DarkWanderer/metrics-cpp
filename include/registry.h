#pragma once

#include <metrics_export.h>

#include <counter.h>
#include <gauge.h>

namespace Metrics
{
    class IRegistry {
    public:
        virtual Gauge getGauge(const Key& key) = 0;
        virtual Counter getCounter(const Key& key) = 0;
    };

    METRICS_EXPORT IRegistry& defaultRegistry();
}
