#include <metrics/registry.h>
#include <metrics/metric.h>

#include "factory.h"

#include <map>
#include <mutex>
#include <stdexcept>

namespace Metrics
{
    struct MetricKeyComparer
    {
        bool operator()(const Metrics::Key &k1, const Metrics::Key &k2) const
        {
            return (k1.name < k2.name) || (k1.labels < k2.labels);
        }
    };

    class RegistryImpl : public IRegistry
    {
        std::map<Key, std::shared_ptr<IMetric>, MetricKeyComparer> m_metrics;

        Gauge getGauge(const Key &key) override
        {
            auto it = m_metrics.find(key);

            if (it == m_metrics.end())
            {
                it = m_metrics.emplace(std::make_pair(key, createGauge())).first;
            }
            auto metric = std::dynamic_pointer_cast<IGaugeValue>(it->second);
            if (!metric)
                throw std::logic_error("Metric exists but is of wrong type");
            return Gauge(metric);
        };

        Counter getCounter(const Key &key) override
        {
            auto it = m_metrics.find(key);

            if (it == m_metrics.end())
            {
                it = m_metrics.emplace(std::make_pair(key, createCounter())).first;
            }
            auto metric = std::dynamic_pointer_cast<ICounterValue>(it->second);
            if (!metric)
                throw std::logic_error("Metric exists but is of wrong type");
            return Counter(metric);
        };

        Histogram getHistogram(const Key& key, const std::vector<double>& bounds) override
        {
            auto it = m_metrics.find(key);

            if (it == m_metrics.end())
            {
                it = m_metrics.emplace(std::make_pair(key, createHistogram(bounds))).first;
            }
            auto metric = std::dynamic_pointer_cast<IHistogram>(it->second);
            if (!metric)
                throw std::logic_error("Metric exists but is of wrong type");
            return Histogram(metric);
        };
    };

    IRegistry::~IRegistry()
    {
    }

    IRegistry &defaultRegistry()
    {
        static RegistryImpl s_registry;
        return s_registry;
    }

    METRICS_EXPORT std::unique_ptr<IRegistry> Metrics::createRegistry()
    {
        return std::unique_ptr<IRegistry>(new RegistryImpl());
    };
}
