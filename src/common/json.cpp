#include <metrics/json.h>

#include <boost/json.hpp>

#include <sstream>

namespace json = boost::json;
using namespace std;

namespace Metrics {
    namespace Json {
        json::object serialize(const string & name, const Labels & labels, const std::shared_ptr<IMetric> metric)
        {
            json::object serialized;
            serialized["name"] = name;
            json::object jlabels;
            for (auto kv = labels.cbegin(); kv != labels.cend(); kv++)
            {
                jlabels[kv->first] = kv->second;
            }
            if (!jlabels.empty())
                serialized["labels"] = jlabels;

            switch (metric->type())
            {
            case TypeCode::Counter:
                serialized["type"] = "counter";
                serialized["value"] = std::static_pointer_cast<ICounterValue>(metric)->value();
                break;
            case TypeCode::Gauge:
                serialized["type"] = "gauge";
                serialized["value"] = std::static_pointer_cast<IGaugeValue>(metric)->value();
                break;
            case TypeCode::Summary:
                {
                    serialized["type"] = "summary";

                    auto s = std::static_pointer_cast<ISummary>(metric);
                    serialized["count"] = s->count();
                    serialized["sum"] = s->sum();

                    json::array quantiles;
                    for (const auto& kv : s->values())
                    {
                        json::object v;
                        v["quantile"] = kv.first;
                        v["count"] = kv.second;
                        quantiles.emplace_back(v);
                    }
                    serialized["quantiles"] = quantiles;
                }
                break;
            case TypeCode::Histogram:
                {
                    serialized["type"] = "histogram";

                    auto s = std::static_pointer_cast<IHistogram>(metric);
                    serialized["count"] = s->count();
                    serialized["sum"] = s->sum();

                    json::array buckets;
                    for (const auto& kv : s->values())
                    {
                        json::object v;
                        v["bound"] = kv.first;
                        v["count"] = kv.second;
                        buckets.emplace_back(v);
                    }
                    serialized["buckets"] = buckets;
                }
                break;
            }

            return serialized;
        }

        METRICS_EXPORT std::string serializeJson(std::shared_ptr<IRegistry> registry)
        {
            json::array result;

            auto names = registry->metricNames();
            for (const auto& name : names)
            {
                auto& group = registry->getGroup(name);
                auto metrics = group.metrics();
                for (const auto& metric : metrics)
                    result.emplace_back(serialize(name, metric.first, metric.second));
            }

            std::stringstream out;
            out << result;
            return out.str();
        }

        METRICS_EXPORT std::string serializeJsonl(std::shared_ptr<IRegistry> registry)
        {
            std::stringstream out;
            auto names = registry->metricNames();
            for (const auto& name : names)
            {
                auto& group = registry->getGroup(name);
                auto metrics = group.metrics();
                for (const auto& metric : metrics)
                    out << serialize(name, metric.first, metric.second) << std::endl;
            }

            return out.str();
        }
    }
}
