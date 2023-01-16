#include <registry.h>
#include <metric.h>

#include "factory.h"

#include <map>
#include <mutex>

namespace Metrics {
    class RegistryImpl : public IRegistry {
        std::map<std::string, std::shared_ptr<IMetric>> m_metrics;

        Gauge getGauge() override {
            //Key key = { "gauge", {} };
            std::string key = "gauge";
            auto it = m_metrics.find(key);

            if (it == m_metrics.end()) {
                it = m_metrics.emplace(std::make_pair(key, createGauge())).first;
            }
            auto metric = std::dynamic_pointer_cast<IGaugeValue>(it->second);
            return Gauge(metric);
        };

        Counter getCounter() override {
            //Key key = { "gauge", {} };
            std::string key = "counter";
            auto it = m_metrics.find(key);

            if (it == m_metrics.end()) {
                it = m_metrics.emplace(std::make_pair(key, createCounter())).first;
            }
            auto metric = std::dynamic_pointer_cast<ICounterValue>(it->second);
            return Counter(metric);
        };
    };

    IRegistry& defaultRegistry()
    {
        static RegistryImpl s_registry;
        return s_registry;
    };
}
