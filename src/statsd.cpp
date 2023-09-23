#include <metrics/serialize.h>

#include <iostream>
#include <sstream>

using std::ostream;
using std::stringstream;
using std::string;
using std::endl;

namespace Metrics
{
    struct StatsdSerializer
    {
        stringstream os;

        string str() {
            return os.str();
        }

        void write(const string& name, const Labels& labels)
        {
            os << name;
            for (auto kv = labels.cbegin(); kv != labels.cend(); kv++)
                os << "," << kv->first << "=" << kv->second;
        }

        void visit(const IRegistry& registry)
        {
            auto names = registry.metricNames();
            for (const auto& name : names)
            {
                auto& group = registry.getGroup(name);
                auto metrics = group.metrics();
                for (const auto& kv : metrics) {
                    const auto& labels = kv.first;
                    const auto& metric = kv.second;
                    switch (metric->type())
                    {
                    case TypeCode::Counter:
                        write(name, labels);
                        os << "|" << std::static_pointer_cast<ICounterValue>(metric)->value() << "|c" << endl;
                        break;
                    case TypeCode::Gauge:
                        write(name, labels);
                        os << "|" << std::static_pointer_cast<IGaugeValue>(metric)->value() << "|g" << endl;
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    };

    std::string serializeStatsd(const IRegistry& registry)
    {
        StatsdSerializer s;
        s.visit(registry);
        return s.str();
    }
}
