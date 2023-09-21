#include <metrics/serialize.h>

#include <fstream>

#define RAPIDJSON_HAS_STDSTRING (1)

#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"

#include <iostream>
#include <sstream>
#include <set>

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

    void write(ostream& os, const Key& key, const ISummary& histogram)
    {
        for (auto value : histogram.values())
        {
            os << key.name << '{';
            for (auto kv = key.labels.cbegin(); kv != key.labels.cend(); kv++)
            {
                os << kv->first << "=\"" << kv->second << '"' << ',';
            }
            os << "quantile=\"" << value.first << "\"} " << value.second << endl;
        }
        os << key.name << "_sum" << key.labels << ' ' << histogram.sum() << endl;
        os << key.name << "_count" << key.labels << ' ' << histogram.count() << endl;
    }

    void write(ostream& os, const Key& key, const IHistogram& histogram)
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
                // os << "# TYPE " << key.name << " counter" << endl;
                os << key << ' ' << std::static_pointer_cast<ICounterValue>(metric)->value() << endl;
                break;
            case TypeCode::Gauge:
                // os << "# TYPE " << key.name << " gauge" << endl;
                os << key << ' ' << std::static_pointer_cast<IGaugeValue>(metric)->value() << endl;
                break;
            case TypeCode::Summary:
                os << "# TYPE " << key.name << " summary" << endl;
                write(os, key, *std::static_pointer_cast<ISummary>(metric));
                break;
            case TypeCode::Histogram:
                os << "# TYPE " << key.name << " histogram" << endl;
                write(os, key, *std::static_pointer_cast<IHistogram>(metric));
                break;
            case TypeCode::Text:
                os << key << ' ' << std::static_pointer_cast<IText>(metric)->value() << endl;
                break;
            }
        }
        return os;
    }

    std::string serializePrometheus(const IRegistry& registry)
    {
        stringstream ss;
        ss << registry;
        return ss.str();
    }

    std::string getMetricLabel(Labels labels) {
        std::ostringstream ss;
        ss << labels;
        std::string label = ss.str();
        std::size_t pos = label.find("kind=\"");
        if (pos == std::string::npos) {
            return "";
        }
        label = label.substr(pos+6);
        pos = label.find("\"");
        return label.substr(0, pos);
    }

    void serializeJSON(const IRegistry& registry, std::string filename) {
        using namespace rapidjson;

        // Create the JSON document
        Document d;
        d.SetObject();
    
        // Add data to the JSON document
        std::set<std::string> usedLabels;
        auto keys = registry.keys();
        for (const auto& key : keys)
        {
            auto metric = registry.get(key);
            if (!metric)
                continue;
            Value mKey(key.name, d.GetAllocator());
            std::string label = getMetricLabel(key.labels);
            switch (metric->type())
            {
            case TypeCode::Counter:
            {
                if (label != "") {
                    // Nested
                    if (usedLabels.count(key.name) == 0) {
                        // Create new nested object
                        Value nestedObject(rapidjson::kObjectType);
                        d.AddMember(mKey, nestedObject, d.GetAllocator());
                        usedLabels.insert(key.name);
                    }
                    Value &nestedObjectValue = d[key.name];
                    Value mLabel(label, d.GetAllocator());
                    nestedObjectValue.AddMember(mLabel, std::static_pointer_cast<ICounterValue>(metric)->value(), d.GetAllocator());
                } 
                else 
                {
                    d.AddMember(mKey, std::static_pointer_cast<ICounterValue>(metric)->value(), d.GetAllocator());
                }
                break;
            }
            case TypeCode::Gauge:
            {
                if (label != "") {
                    // Nested
                    if (usedLabels.count(key.name) == 0) {
                        // Create new nested object
                        Value nestedObject(rapidjson::kObjectType);
                        d.AddMember(mKey, nestedObject, d.GetAllocator());
                        usedLabels.insert(key.name);
                    }
                    Value &nestedObjectValue = d[key.name];
                    Value mLabel(label, d.GetAllocator());
                    nestedObjectValue.AddMember(mLabel, std::static_pointer_cast<IGaugeValue>(metric)->value(), d.GetAllocator());
                } 
                else 
                {
                    d.AddMember(mKey, std::static_pointer_cast<IGaugeValue>(metric)->value(), d.GetAllocator());
                }
                break;
            }
            case TypeCode::Summary:
            {
                const ISummary& summary = *std::static_pointer_cast<ISummary>(metric);

                // Create new nested object
                Value nestedObject(rapidjson::kObjectType);
                d.AddMember(mKey, nestedObject, d.GetAllocator());
                Value &nestedObjectValue = d[key.name];
                
                for (auto& value : summary.values())
                {
                    Value quantile(std::to_string(static_cast<int>(value.first))+"x", d.GetAllocator());
                    nestedObjectValue.AddMember(quantile, value.second, d.GetAllocator());
                }
                nestedObjectValue.AddMember("sum", summary.sum(), d.GetAllocator());
                nestedObjectValue.AddMember("average", summary.sum() / summary.count(), d.GetAllocator());
                nestedObjectValue.AddMember("count", summary.count(), d.GetAllocator());
                break;
            }
            case TypeCode::Histogram:
            {
                const IHistogram& histogram = *std::static_pointer_cast<IHistogram>(metric);

                // Create new nested object
                Value nestedObject(rapidjson::kObjectType);
                d.AddMember(mKey, nestedObject, d.GetAllocator());
                Value &nestedObjectValue = d[key.name];
                
                for (auto& value : histogram.values())
                {
                    Value quantile(std::to_string(static_cast<int>(value.first))+"x", d.GetAllocator());
                    nestedObjectValue.AddMember(quantile, value.second, d.GetAllocator());
                }
                nestedObjectValue.AddMember("sum", histogram.sum(), d.GetAllocator());
                nestedObjectValue.AddMember("average", histogram.sum() / histogram.count(), d.GetAllocator());
                nestedObjectValue.AddMember("count", histogram.count(), d.GetAllocator());
                break;
            }
            case TypeCode::Text:
            {
                if (label != "") {
                    // Nested
                    if (usedLabels.count(key.name) == 0) {
                        // Create new nested object
                        Value nestedObject(rapidjson::kObjectType);
                        d.AddMember(mKey, nestedObject, d.GetAllocator());
                        usedLabels.insert(key.name);
                    }
                    Value &nestedObjectValue = d[key.name];
                    Value mLabel(label, d.GetAllocator());
                    Value mText(std::static_pointer_cast<IText>(metric)->value(), d.GetAllocator());
                    nestedObjectValue.AddMember(mLabel, mText, d.GetAllocator());
                } else {
                    Value mText(std::static_pointer_cast<IText>(metric)->value(), d.GetAllocator());
                    d.AddMember(mKey, mText, d.GetAllocator());
                }
                break;
            }
            }
        }

        // Write JSON file
        FILE* fp2 = NULL;
        errno_t err = 0;
        if ((err = fopen_s(&fp2, filename.c_str(), "w")) == 0)
        {
            char writeBuffer[65536];
            FileWriteStream os(fp2, writeBuffer, sizeof(writeBuffer));
            Writer<FileWriteStream> writer(os);
            d.Accept(writer);
            fclose(fp2);
        }
    }
}
