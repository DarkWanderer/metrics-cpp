#pragma once

#include <metrics_export.h>
#include <metrics/metric.h>

#include <vector>

namespace Metrics
{
    class IMetricGroup {
    public:
        virtual std::string description() const = 0;
        virtual TypeCode type() const = 0;
        virtual std::vector<std::pair<Labels, std::shared_ptr<IMetric>>> metrics() const = 0;
    };

    class IRegistry
    {
    public:
        virtual std::vector<std::string> metricNames() const = 0;
        virtual const IMetricGroup& getGroup(const std::string& name) const = 0;

        /// <summary>
        /// Get or create a gauge with provided key
        /// </summary>
        /// <param name="key">metric key</param>
        /// <returns>new or existing metric object</returns>
        virtual Gauge getGauge(const std::string name, const Labels& labels = {}) = 0;

        /// <summary>
        /// Get or create a counter with provided key
        /// </summary>
        /// <param name="key">metric key</param>
        /// <returns>new or existing metric object</returns>
        virtual Counter getCounter(const std::string name, const Labels& labels = {}) = 0;

        /// <summary>
        /// Get or create a summary with provided key
        /// </summary>
        /// <param name="key">metric key</param>
        /// <returns>new or existing metric object</returns>
        virtual Summary getSummary(const std::string name, const Labels& labels = {}, const std::vector<double>& quantiles = { 0.50, 0.90, 0.99, 0.999 }, double error = 0.01) = 0;

        /// <summary>
        /// Get or create a histogram with provided key
        /// </summary>
        /// <param name="key">metric key</param>
        /// <returns>new or existing metric object</returns>
        virtual Histogram getHistogram(const std::string name, const Labels& labels = {}, const std::vector<double>& bounds = { 100., 200., 300., 400., 500. }) = 0;

        /// <summary>
        /// Register an existing metric wrapper object with the registry
        /// </summary>
        /// <typeparam name="TMetric"></typeparam>
        /// <param name="key"></param>
        /// <param name="metric"></param>
        /// <returns></returns>
        template<class TMetric> bool add(TMetric metric, const std::string name, const Labels& labels = {}) { return add(metric.raw(), name, labels); }

        /// <summary>
        /// Register an existing low-level metric object with the registry
        /// </summary>
        /// <typeparam name="TMetric"></typeparam>
        /// <param name="key"></param>
        /// <param name="metric"></param>
        /// <returns></returns>
        virtual bool add(std::shared_ptr<IMetric> metric, const std::string name, const Labels& labels = {}) = 0;

        virtual void setDescription(std::string name, std::string description) = 0;

        virtual ~IRegistry() = 0;
    };

    METRICS_EXPORT IRegistry& defaultRegistry();
    METRICS_EXPORT std::unique_ptr<IRegistry> createRegistry();
}
