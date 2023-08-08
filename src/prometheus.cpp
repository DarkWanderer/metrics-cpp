#include <metrics/prometheus.h>

#include <iostream>
#include <sstream>

using std::ostream;
using std::stringstream;
using std::endl;

namespace Metrics
{
    ostream& operator<<(ostream& os, const Key& key)
    {
        os << key.name;
        bool opened = false;
        for (auto kv = key.labels.cbegin(); kv != key.labels.cend(); kv++)
        {
            os << (opened ? "," : "{") << kv->first << "=\"" << kv->second << '"';
            opened = true;
        }
        if (opened)
            os << '}';
        return os;
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
                os << key << ' ' << std::static_pointer_cast<ICounterValue>(metric)->value();
                break;
            case TypeCode::Gauge:
                os << key << ' ' << std::static_pointer_cast<IGaugeValue>(metric)->value();
                break;
            }
            os << endl;
        }
        return os;
    }

    std::string Metrics::serializeToPrometheus(const IRegistry& registry)
    {
        stringstream ss;
        ss << registry;
        return ss.str();
    }
}
