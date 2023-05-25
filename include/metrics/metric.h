#pragma once

#include <metrics/labels.h>
#include <metrics_export.h>

#include <string>

namespace Metrics
{
    enum class TypeCode {
        Gauge,
        Counter,
        Summary,
        Histogram
    };

    class IMetric
    {
    public:
        IMetric() = default;
        IMetric(const IMetric &) = default;
        IMetric(IMetric &&) = default;
        METRICS_EXPORT virtual ~IMetric() = default;
        virtual TypeCode type() = 0;
    };

    template<TypeCode T> class ITypedMetric : public IMetric
    {
    public:
        static inline TypeCode stype() { return T; }
        TypeCode type() override { return T; }
    };

    template<typename Value> class ValueProxy : public Value
    {
    public:
        typedef Value value_type;
    };

    struct Key
    {
        std::string name;
        Labels labels;

        bool operator==(const Metrics::Key& other) const { return name == other.name && labels == other.labels; }
    };
}
