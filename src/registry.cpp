#include <metrics/registry.h>
#include <metrics/metric.h>

#include <map>
#include <mutex>
#include <cstdint>
#include <stdexcept>
#include <functional>
#include <unordered_map>

using namespace std;


namespace Metrics {
    struct MetricKeyHasher
    {
        // Hash
        uint64_t operator()(const std::string name, const Labels& labels) const
        {
            hash<string> h;
            // computes the hash of an employee using a variant
            // of the Fowler-Noll-Vo hash function
            constexpr uint64_t prime{ 0x100000001B3 };
            uint64_t result = h(name);
            for (auto it = labels.cbegin(); it != labels.cend(); it++)
                result = (result * prime) ^ h(it->first) ^ h(it->second);

            return result;
        }
    };

    class MetricGroup : public IMetricGroup {
    private:
        mutable mutex m_mutex;
        TypeCode m_type;
        string m_description;
        map<Labels, shared_ptr<IMetric>> m_metrics;

    public:
        MetricGroup(TypeCode type) : m_type(type) { }
        ~MetricGroup() = default;
        MetricGroup(const MetricGroup&) = delete;
        MetricGroup(MetricGroup&&) = delete;

        typedef decltype(m_metrics)::const_iterator const_iterator;

        TypeCode type() const override { return m_type; }

        string description() const override {
            unique_lock<mutex> lock(m_mutex);
            return m_description;
        }

        void setDescription(string description) {
            unique_lock<mutex> lock(m_mutex);
            m_description = description;
        }

        bool add(const Labels& labels, shared_ptr<IMetric> metric)
        {
            unique_lock<mutex> lock(m_mutex);

            if (m_type != metric->type())
                throw logic_error("Inconsistent type of metric");

            return m_metrics.emplace(labels, metric).second;
        }

        template<typename TValueProxy> TValueProxy get(const Labels& labels, function<shared_ptr<typename TValueProxy::value_type>(void)> factory)
        {
            unique_lock<mutex> lock(m_mutex);
            auto it = m_metrics.find(labels);
            if (it == m_metrics.end()) {
                auto new_value = factory();
                it = m_metrics.emplace(move(Labels(labels)), move(new_value)).first;
            }
            auto metric = it->second;
            if (TValueProxy::value_type::stype() != it->second->type()) {
                throw logic_error("Inconsistent type of metric");
            }
            return TValueProxy(static_pointer_cast<typename TValueProxy::value_type>(metric));
        }

        vector<pair<Labels, shared_ptr<IMetric>>> metrics() const override
        {
            unique_lock<mutex> lock(m_mutex);
            vector<pair<Labels, shared_ptr<IMetric>>> result;
            result.reserve(m_metrics.size());

            for (const auto& kv : m_metrics)
                result.emplace_back(kv.first, kv.second);

            return result;
        }

        size_t size() const {
            unique_lock<mutex> lock(m_mutex);
            return m_metrics.size();
        }
    };

    class RegistryImpl : public IRegistry
    {
    private:
        mutable mutex m_mutex;

        // Group metrics by name to support Prometheus model
        map<string, MetricGroup> m_groups;

    public:
        RegistryImpl() = default;
        ~RegistryImpl() {}

        const IMetricGroup& getGroup(const string& name) const override
        {
            unique_lock<mutex> lock(m_mutex);
            auto it = m_groups.find(name);
            if (it == m_groups.end())
                throw std::logic_error("Group not found");
            return it->second;
        }

        MetricGroup& getOrCreateGroup(const string& name, TypeCode type)
        {
            unique_lock<mutex> lock(m_mutex);
            auto it = m_groups.find(name);

            if (it == m_groups.end()) {
                it = m_groups.emplace(name, type).first;
            }
            else if (type != it->second.type()) {
                throw logic_error("Inconsistent type of metric");
            }

            return it->second;
        }

        Gauge getGauge(const std::string name, const Labels& labels) override {
            auto& group = getOrCreateGroup(name, TypeCode::Gauge);
            return group.get<Gauge>(labels, makeGauge);
        };

        Counter getCounter(const std::string name, const Labels& labels) override {
            auto& group = getOrCreateGroup(name, TypeCode::Counter);
            return group.get<Counter>(labels, makeCounter);
        };

        Summary getSummary(const std::string name, const Labels& labels, const vector<double>& quantiles, double error) override {
            auto& group = getOrCreateGroup(name, TypeCode::Summary);
            return group.get<Summary>(labels, bind(makeSummary, quantiles, error));
        }

        Histogram getHistogram(const std::string name, const Labels& labels, const vector<double>& bounds) override {
            auto& group = getOrCreateGroup(name, TypeCode::Histogram);
            return group.get<Histogram>(labels, bind(makeHistogram, bounds));
        }

        virtual bool add(shared_ptr<IMetric> metric, const std::string name, const Labels& labels) override
        {
            auto& group = getOrCreateGroup(name, metric->type());
            return group.add(labels, metric);
        }

        // Inherited via IRegistry
        vector<string> metricNames() const override
        {
            unique_lock<mutex> lock(m_mutex);
            vector<string> result;
            result.reserve(m_groups.size());
            for (const auto& g : m_groups)
                result.push_back(g.first);
            return result;
        }

        virtual void setDescription(std::string name, std::string description) override
        {
            unique_lock<mutex> lock(m_mutex);
            auto it = m_groups.find(name);
            if (it != m_groups.end())
                it->second.setDescription(description);
        }

        virtual size_t size() const override
        {
            size_t result = 0;
            unique_lock<mutex> lock(m_mutex);
            for (const auto& g : m_groups)
                result += g.second.size();
            return result;
        }
    };

    IRegistry::~IRegistry() { }

    IMetricGroup::~IMetricGroup() { }

    shared_ptr<IRegistry> defaultRegistry()
    {
        static shared_ptr<IRegistry> s_registry = createRegistry();
        return s_registry;
    }

    METRICS_EXPORT shared_ptr<IRegistry> createRegistry()
    {
        return make_shared<RegistryImpl>();
    };
}
