#pragma once

#include <metrics_export.h>
#include <metrics/metric.h>

#include <vector>

namespace Metrics
{
    struct Key
    {
        const std::string name;
        const Labels labels;

        bool operator==(const Metrics::Key& other) const { return name == other.name && labels == other.labels; }

        bool operator<(const Metrics::Key& other) const
        {
            auto c1 = name.compare(other.name);
            if (c1 != 0)
                return c1 < 0;
            return labels < other.labels;
        }
    };

    class IRegistry
    {
    public:
        virtual std::shared_ptr<IMetric> get(const Key& key) const = 0;
        virtual Gauge getGauge(const Key& key) = 0;
        virtual Counter getCounter(const Key& key) = 0;
        virtual Histogram getHistogram(const Key& key, const std::vector<double>& bounds = {}) = 0;

        template<class TMetric> bool add(const Key& key, TMetric metric) { return add(key, metric.raw()); }
        virtual bool add(const Key& key, std::shared_ptr<IMetric> metric) = 0;

        virtual std::vector<Key> keys() const = 0;

        virtual ~IRegistry() = 0;
    };

    METRICS_EXPORT IRegistry& defaultRegistry();
    METRICS_EXPORT std::unique_ptr<IRegistry> createRegistry();
    METRICS_EXPORT std::unique_ptr<IRegistry> createLargeRegistry();
}
