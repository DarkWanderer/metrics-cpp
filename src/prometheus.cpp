#include <metrics/prometheus.h>

#include <iostream>
#include <sstream>

using std::ostream;
using std::stringstream;
using std::endl;

namespace Metrics
{
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

    ostream& operator<<(ostream& os, const Key& key)
    {
        os << key.name;
        os << key.labels;
        return os;
    }

    void writeHistogram(ostream& os, const Key& key, const IHistogram& histogram)
    {
        for (auto value : histogram.values())
        {
            os << key.name << '{';
            for (auto kv = key.labels.cbegin(); kv != key.labels.cend(); kv++)
            {
                os << kv->first << "=\"" << kv->second << '"' << ',';
            }
            os << "le=\"" << value.first << "\"} " << value.second << endl;
        }
        os << key.name << "_sum" << key.labels << ' ' << histogram.sum() << endl;
        os << key.name << "_count" << key.labels << ' ' << histogram.count() << endl;
    }

    ostream& operator<<(ostream& os, const IRegistry& registry)
    {
        auto keys = registry.keys();
        for (const auto& key : keys)
        {
            auto metric = registry.get(key);
            if (!metric)
                continue;
            switch (metric->type())
            {
            case TypeCode::Counter:
                os << key << ' ' << std::static_pointer_cast<ICounterValue>(metric)->value() << endl;
                break;
            case TypeCode::Gauge:
                os << key << ' ' << std::static_pointer_cast<IGaugeValue>(metric)->value() << endl;
                break;
            case TypeCode::Histogram:
                writeHistogram(os, key, *std::static_pointer_cast<IHistogram>(metric));
                break;
            }
        }
        return os;
    }

    std::string serializeToPrometheus(const IRegistry& registry)
    {
        stringstream ss;
        ss << registry;
        return ss.str();
    }
}
