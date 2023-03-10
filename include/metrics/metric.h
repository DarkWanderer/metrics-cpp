#pragma once

#include <metrics/labels.h>
#include <metrics_export.h>

#include <string>

namespace Metrics
{
    class IMetric
    {
    public:
        IMetric() = default;
        IMetric(const IMetric &) = default;
        IMetric(IMetric &&) = default;
        METRICS_EXPORT virtual ~IMetric() = default;
    };

    struct Key
    {
        std::string name;
        Labels labels;
    };
}
