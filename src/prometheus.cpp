#include <metrics/serialize.h>
#include <metrics/sink.h>

#include <asio.hpp>

#include <iostream>
#include <sstream>

using namespace std;

namespace Metrics
{
    const char* typeString(TypeCode type) {
        switch (type)
        {
        case TypeCode::Gauge:
            return "gauge";
            break;
        case TypeCode::Counter:
            return "counter";
            break;
        case TypeCode::Summary:
            return "summary";
            break;
        case TypeCode::Histogram:
            return "histogram";
            break;
        default:
            break;
        }
        return "unknown";
    }

    ostream& operator<<(ostream& os, const Labels& labels)
    {
        bool opened = false;
        for (auto kv = labels.cbegin(); kv != labels.cend(); kv++)
        {
            os << (opened ? "," : "{") << kv->first << "=\"" << kv->second << '"';
            opened = true;
        }
        if (opened)
            os << '}';
        return os;
    }

    void serialize(ostream& os, const string& name, const Labels& labels, const ISummary& summary)
    {
        for (auto& value : summary.values())
        {
            os << name << '{';
            for (auto kv = labels.cbegin(); kv != labels.cend(); kv++)
            {
                os << kv->first << "=\"" << kv->second << '"' << ',';
            }
            os << "quantile=\"" << value.first << "\"} " << value.second << endl;
        }
        os << name << "_sum" << labels << ' ' << summary.sum() << endl;
        os << name << "_count" << labels << ' ' << summary.count() << endl;
    }

    void serialize(ostream& os, const string& name, const Labels& labels, const IHistogram& histogram)
    {
        for (auto& value : histogram.values())
        {
            os << name << '{';
            for (auto kv = labels.cbegin(); kv != labels.cend(); kv++)
            {
                os << kv->first << "=\"" << kv->second << '"' << ',';
            }
            os << "le=\"" << value.first << "\"} " << value.second << endl;
        }
        os << name << "_sum" << labels << ' ' << histogram.sum() << endl;
        os << name << "_count" << labels << ' ' << histogram.count() << endl;
    }

    void serialize(ostream& os, const string& name, const Labels& labels, const std::shared_ptr<IMetric> metric)
    {
        if (!metric)
            return ;
        switch (metric->type())
        {
        case TypeCode::Counter:
            // os << "# TYPE " << name << " counter" << endl;
            os << name << labels << ' ' << static_pointer_cast<ICounterValue>(metric)->value() << endl;
            break;
        case TypeCode::Gauge:
            // os << "# TYPE " << name << " gauge" << endl;
            os << name << labels << ' ' << static_pointer_cast<IGaugeValue>(metric)->value() << endl;
            break;
        case TypeCode::Summary:
            serialize(os, name, labels, *static_pointer_cast<ISummary>(metric));
            break;
        case TypeCode::Histogram:
            serialize(os, name, labels, *static_pointer_cast<IHistogram>(metric));
            break;
        }
    }

    string serializePrometheus(const IRegistry& registry)
    {
        stringstream out;
        auto names = registry.metricNames();
        for (const auto& name : names)
        {
            auto& group = registry.getGroup(name);
            if (!group.description().empty())
                out << "# HELP " << name << " " << group.description() << endl;
            out << "# TYPE " << name << " " << typeString(group.type()) << endl;
            auto metrics = group.metrics();
            for (const auto& metric : metrics)
                serialize(out, name, metric.first, metric.second);
        }
        return out.str();
    }

    class PrometheusHttpServerSink : public IRegistrySink {
    private:
        shared_ptr<IRegistry> m_registry;

    public:
        PrometheusHttpServerSink(shared_ptr<IRegistry> registry) : m_registry(registry) {};

        virtual shared_ptr<IRegistry> registry() const override {
            return m_registry;
        };
    };

    shared_ptr<IRegistrySink> createPrometheusHttpServerSink( shared_ptr<IRegistry> registry)
    {
        if (!registry)
            registry = createRegistry();

        return make_shared<PrometheusHttpServerSink>(registry);
    }

    class PrometheusOnDemandPushGatewaySink : public IOnDemandSink 
    {
    private:
        string m_host;
        uint16_t m_port;
    public:
        PrometheusOnDemandPushGatewaySink(string host, uint16_t port) :
            m_host(host), m_port(port)
        {};

        void send(shared_ptr<IRegistry> registry) override
        {
            asio::io_service io_service;
            asio::ip::tcp::resolver resolver(io_service);
            asio::ip::tcp::resolver::query query(m_host, std::to_string(m_port));
            auto endpoint_iterator = resolver.resolve(query);

        }
    };
}
