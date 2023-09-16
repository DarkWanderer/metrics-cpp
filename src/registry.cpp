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
        RegistryImpl() = default;
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

        Gauge getGauge(const Key& key) override { return get<Gauge>(key, makeGauge); };

        Counter getCounter(const Key& key) override { return get<Counter>(key, makeCounter); };

        Summary getSummary(const Key& key, const std::vector<double>& quantiles, double error) override { return get<Summary>(key, std::bind(makeSummary, quantiles, error)); }

        Histogram getHistogram(const Key& key, const std::vector<double>& bounds) override { return get<Histogram>(key, std::bind(makeHistogram, bounds)); }

        Text getText(const Key& key) override { return get<Text>(key, makeText); }

        std::vector<Key> keys() const
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            std::vector<Key> keys;
            keys.reserve(m_metrics.size());
            for (const auto& kv : m_metrics) {
                keys.push_back(kv.first);
            }
            return keys;
        }

        virtual bool add(const Key& key, std::shared_ptr<IMetric> metric) override
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_metrics.emplace(key, metric).second;
        }

        virtual std::shared_ptr<IMetric> get(const Key& key) const override
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            auto it = m_metrics.find(key);
            if (it != m_metrics.end())
                return it->second;
            else
                return nullptr;
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
}
