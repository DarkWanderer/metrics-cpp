#pragma once

#include <metrics_export.h>
#include <metrics/metric.h>

#include <vector>

namespace Metrics
{
    struct Key
    {
        Key() = default;
        Key(Key&&) = default;
        Key(const Key&) = default;
        ~Key() = default;

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
        /// <summary>
        /// Get untyped IMetric or nullptr
        /// </summary>
        /// <param name="key">metric key</param>
        /// <returns>metric</returns>
        virtual std::shared_ptr<IMetric> get(const Key& key) const = 0;

        /// <summary>
        /// Get or create a gauge with provided key
        /// </summary>
        /// <param name="key">metric key</param>
        /// <returns>new or existing metric object</returns>
        virtual Gauge getGauge(const Key& key) = 0;

        /// <summary>
        /// Get or create a counter with provided key
        /// </summary>
        /// <param name="key">metric key</param>
        /// <returns>new or existing metric object</returns>
        virtual Counter getCounter(const Key& key) = 0;

        /// <summary>
        /// Get or create a summary with provided key
        /// </summary>
        /// <param name="key">metric key</param>
        /// <returns>new or existing metric object</returns>
        virtual Summary getSummary(const Key& key, const std::vector<double>& quantiles = {}, double error = 0.01) = 0;
            
            /// <summary>
        /// Get or create a histogram with provided key
        /// </summary>
        /// <param name="key">metric key</param>
        /// <returns>new or existing metric object</returns>
        virtual Histogram getHistogram(const Key& key, const std::vector<double>& bounds = {}) = 0;

        /// <summary>
        /// Register an existing metric wrapper object with the registry
        /// </summary>
        /// <typeparam name="TMetric"></typeparam>
        /// <param name="key"></param>
        /// <param name="metric"></param>
        /// <returns></returns>
        template<class TMetric> bool add(const Key& key, TMetric metric) { return add(key, metric.raw()); }

        /// <summary>
        /// Register an existing low-level metric object with the registry
        /// </summary>
        /// <typeparam name="TMetric"></typeparam>
        /// <param name="key"></param>
        /// <param name="metric"></param>
        /// <returns></returns>
        virtual bool add(const Key& key, std::shared_ptr<IMetric> metric) = 0;

        virtual std::vector<Key> keys() const = 0;

        virtual ~IRegistry() = 0;
    };

    METRICS_EXPORT IRegistry& defaultRegistry();
    METRICS_EXPORT std::unique_ptr<IRegistry> createRegistry();
    METRICS_EXPORT std::unique_ptr<IRegistry> createLargeRegistry();
}
