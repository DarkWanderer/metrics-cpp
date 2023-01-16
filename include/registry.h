#pragma once

#include <metrics_export.h>

#include <counter.h>
#include <gauge.h>

namespace Metrics
{
    class IRegistry {
    public:
        virtual Gauge getGauge() = 0;
        virtual Counter getCounter() = 0;
    };

    METRICS_EXPORT IRegistry& defaultRegistry();
}
