#include <metrics/registry.h>
#include <metrics/metric.h>

#include <map>
#include <mutex>
#include <cstdint>
#include <stdexcept>
#include <functional>
#include <unordered_map>


namespace Metrics
{
    // defined in metrics.cpp
    std::shared_ptr<IGaugeValue> createGauge();
    std::shared_ptr<ICounterValue> createCounter();
    std::shared_ptr<IHistogram> createHistogram(const std::vector<double>& bounds);

    struct MetricKeyHasher
    {
        // Hash
        std::uint64_t operator()(const Key& key) const
        {
            std::hash<std::string> h;
            // computes the hash of an employee using a variant
            // of the Fowler-Noll-Vo hash function
            constexpr std::uint64_t prime{ 0x100000001B3 };
            std::uint64_t result = h(key.name);
            for (auto it = key.labels.cbegin(); it != key.labels.cend(); it++)
                result = (result * prime) ^ h(it->first) ^ h(it->second);

            return result;
        }
    };

    class RegistryImpl : public IRegistry
    {
    private:
        mutable std::mutex m_mutex;
        // Assumption: expected metrics count per registry is in order of hundreds, not hundred thousands
        std::map<Key, std::shared_ptr<IMetric>> m_metrics;
        // std::unordered_map<Key, std::shared_ptr<IMetric>, MetricKeyHasher> m_metrics;

    public:
        ~RegistryImpl() {}

        template<typename TValueProxy> TValueProxy get(const Key& key, std::function<std::shared_ptr<typename TValueProxy::value_type>(void)> factory)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            auto it = m_metrics.find(key);

            if (it == m_metrics.end())
            {
                auto new_value = factory();
                it = m_metrics.emplace(std::move(Key(key)), std::move(new_value)).first;
            }

            auto metric = it->second;
            if (TValueProxy::value_type::stype() != metric->type())
                throw std::logic_error("Inconsistent type of metric");

            return TValueProxy(std::static_pointer_cast<typename TValueProxy::value_type>(metric));
        }

        Gauge getGauge(const Key& key) override { return get<Gauge>(key, createGauge); };

        Counter getCounter(const Key& key) override { return get<Counter>(key, createCounter); };

        Histogram getHistogram(const Key& key, const std::vector<double>& bounds) override { return get<Histogram>(key, std::bind(createHistogram, bounds)); }

        std::vector<Key> keys() const
        {
            std::vector<Key> keys;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                keys.reserve(m_metrics.size());
                for (const auto& kv : m_metrics) {
                    keys.push_back(kv.first);
                }
            }
            return keys;
        }

        virtual bool add(const Key& key, std::shared_ptr<IMetric> metric) override
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_metrics.emplace(key, metric).second;
        }
    };

    class LargeRegistryImpl : public IRegistry
    {
    private:
        mutable std::mutex m_mutex;
        std::unordered_map<Key, std::shared_ptr<IMetric>, MetricKeyHasher> m_metrics;

    public:
        ~LargeRegistryImpl() {}

        template<typename TValueProxy, typename TValue> TValueProxy get(const Key& key, std::function<std::shared_ptr<TValue>(void)> factory)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            auto it = m_metrics.find(key);

            if (it == m_metrics.end())
            {
                auto new_value = factory();
                it = m_metrics.emplace(std::move(Key(key)), std::move(new_value)).first;
            }

            auto metric = it->second;
            if (TValue::stype() != metric->type())
                throw std::logic_error("Wrong type of metric");

            return TValueProxy(std::static_pointer_cast<TValue>(metric));
        }

        Gauge getGauge(const Key& key) override { return get<Gauge, IGaugeValue>(key, createGauge); };

        Counter getCounter(const Key& key) override { return get<Counter, ICounterValue>(key, createCounter); };

        Histogram getHistogram(const Key& key, const std::vector<double>& bounds) override { return get<Histogram, IHistogram>(key, std::bind(createHistogram, bounds)); }

        std::vector<Key> keys() const
        {
            std::vector<Key> keys;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                keys.reserve(m_metrics.size());
                for (const auto& kv : m_metrics) {
                    keys.push_back(kv.first);
                }
            }
            return keys;
        }

        virtual bool add(const Key& key, std::shared_ptr<IMetric> metric) override
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_metrics.emplace(key, metric).second;
        }
    };

    IRegistry::~IRegistry()
    {
    }

    IRegistry& defaultRegistry()
    {
        static RegistryImpl s_registry;
        return s_registry;
    }

    METRICS_EXPORT std::unique_ptr<IRegistry> createRegistry()
    {
        return std::unique_ptr<IRegistry>(new RegistryImpl());
    };

    METRICS_EXPORT std::unique_ptr<IRegistry> createLargeRegistry()
    {
        return std::unique_ptr<IRegistry>(new LargeRegistryImpl());
    };
}
