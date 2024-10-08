#include <metrics/prometheus.h>
#include <metrics/sink.h>

#include <iostream>
#include <sstream>

using namespace std;

namespace Metrics {
    namespace Prometheus {
        const char* typeString(TypeCode type) {
            switch (type) {
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
            for (auto kv = labels.cbegin(); kv != labels.cend(); kv++) {
                os << (opened ? "," : "{") << kv->first << "=\"" << kv->second << '"';
                opened = true;
            }
            if (opened)
                os << '}';
            return os;
        }

        void serialize(ostream& os, const string& name, const Labels& labels, const ISummary& summary)
        {
            for (auto& value : summary.values()) {
                os << name << '{';
                for (auto kv = labels.cbegin(); kv != labels.cend(); kv++) {
                    os << kv->first << "=\"" << kv->second << '"' << ',';
                }
                os << "quantile=\"" << value.first << "\"} " << value.second << endl;
            }
            os << name << "_sum" << labels << ' ' << summary.sum() << endl;
            os << name << "_count" << labels << ' ' << summary.count() << endl;
        }

        void serialize(ostream& os, const string& name, const Labels& labels, const IHistogram& histogram)
        {
            uint64_t count = 0;
            for (auto& value : histogram.values()) {
                os << name << '{';
                for (auto kv = labels.cbegin(); kv != labels.cend(); kv++) {
                    os << kv->first << "=\"" << kv->second << '"' << ',';
                }
                os << "le=\"" << value.first << "\"} " << value.second << endl;

                if (value.first == numeric_limits<double>::infinity())
                    count = value.second;
            }
            os << name << "_sum" << labels << ' ' << histogram.sum() << endl;
            os << name << "_count" << labels << ' ' << count << endl;
        }

        void serialize(ostream& os, const string& name, const Labels& labels, const std::shared_ptr<IMetric> metric)
        {
            if (!metric)
                return;
            switch (metric->type()) {
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

        string serialize(std::shared_ptr<IRegistry> registry)
        {
            stringstream out;
            auto names = registry->metricNames();
            for (const auto& name : names) {
                auto& group = registry->getGroup(name);
                if (!group.description().empty())
                    out << "# HELP " << name << " " << group.description() << endl;
                out << "# TYPE " << name << " " << typeString(group.type()) << endl;
                auto metrics = group.metrics();
                for (const auto& metric : metrics)
                    serialize(out, name, metric.first, metric.second);
            }
            return out.str();
        }



    }
}
