#pragma once

#include <metrics/labels.h>
#include <metrics_export.h>

#include <string>
#include <memory>

namespace Metrics
{
#pragma region Forward declarations
    class ICounterValue;
    class IGaugeValue;
    class ISummary;
    class IHistogram;
    class IText;

    class IMetricVisitor
    {
    public:
        virtual void visit(ICounterValue&) = 0;
        virtual void visit(IGaugeValue&) = 0;
        virtual void visit(ISummary&) = 0;
        virtual void visit(IHistogram&) = 0;
        virtual void visit(IText&) = 0;
        METRICS_EXPORT virtual ~IMetricVisitor() = default;
    };

    METRICS_EXPORT std::shared_ptr<IGaugeValue> makeGauge();
    METRICS_EXPORT std::shared_ptr<ICounterValue> makeCounter();
    METRICS_EXPORT std::shared_ptr<ISummary> makeSummary(const std::vector<double>& quantiles, double error);
    METRICS_EXPORT std::shared_ptr<IHistogram> makeHistogram(const std::vector<double>& bounds);
    METRICS_EXPORT std::shared_ptr<IText> makeText();
#pragma endregion

#pragma region Common definitions
    enum class TypeCode {
        Gauge,
        Counter,
        Summary,
        Histogram,
        Text
    };

    class IMetric
    {
    public:
        METRICS_EXPORT virtual ~IMetric() = default;
        virtual TypeCode type() = 0;
        virtual void accept(IMetricVisitor&) = 0;
    };

    template<TypeCode T> class ITypedMetric : public IMetric
    {
    public:
        static inline TypeCode stype() { return T; }
        TypeCode type() override { return T; }
    };

    template<typename Value> class ValueProxy : public Value
    {
    protected:
        const std::shared_ptr<Value> m_value;
        ValueProxy(std::shared_ptr<Value> value) : m_value(value) {};
        ValueProxy(const ValueProxy&) = default;
        ValueProxy(ValueProxy&&) = default;
    public:
        typedef Value value_type;
        std::shared_ptr<IMetric> raw() { return m_value; }
    };
#pragma endregion

#pragma region Interfaces
    class ICounterValue : public ITypedMetric<TypeCode::Counter>
    {
    public:
        virtual ICounterValue& operator++(int) = 0;
        virtual ICounterValue& operator+=(uint64_t value) = 0;
        virtual uint64_t value() const = 0;
        virtual void reset() = 0;
        operator uint64_t() const { return value(); };
        METRICS_EXPORT virtual void accept(IMetricVisitor&) override;

    protected:
        METRICS_EXPORT virtual ~ICounterValue() = 0;
    };

    class IGaugeValue : public ITypedMetric<TypeCode::Gauge>
    {
    public:
        virtual IGaugeValue& operator=(double value) = 0;
        virtual IGaugeValue& operator+=(double value) = 0;
        virtual IGaugeValue& operator-=(double value) = 0;
        virtual double value() const = 0;
        operator double() const { return value(); };
        METRICS_EXPORT virtual void accept(IMetricVisitor&) override;

    protected:
        METRICS_EXPORT virtual ~IGaugeValue() = 0;
    };

    class ISummary : public ITypedMetric<TypeCode::Summary>
    {
    public:
        virtual ISummary& observe(double value) = 0;
        virtual uint64_t count() const = 0;
        virtual double sum() const = 0;
        virtual std::vector<std::pair<double, uint64_t>> values() const = 0;
        METRICS_EXPORT virtual void accept(IMetricVisitor&) override;

    protected:
        METRICS_EXPORT virtual ~ISummary() = 0;
    };

    class IHistogram : public ITypedMetric<TypeCode::Histogram>
    {
    public:
        virtual IHistogram& observe(double value) = 0;
        virtual uint64_t count() const = 0;
        virtual double sum() const = 0;
        virtual std::vector<std::pair<double, uint64_t>> values() const = 0;
        METRICS_EXPORT virtual void accept(IMetricVisitor&) override;

    protected:
        METRICS_EXPORT virtual ~IHistogram() = 0;
    };

    class IText : public ITypedMetric<TypeCode::Text>
    {
    public:
        virtual IText& operator=(std::string value) = 0;
        virtual std::string value() const = 0;
        METRICS_EXPORT virtual void accept(IMetricVisitor&) override;

    protected:
        METRICS_EXPORT virtual ~IText() = 0;
    };
#pragma endregion

#pragma region Stack containers
    class Counter : public ValueProxy<ICounterValue>
    {
    public:
        ICounterValue& operator++(int) override { return (*m_value)++; };
        ICounterValue& operator+=(uint64_t value) override { return (*m_value += value); };
        void reset() override { m_value->reset(); };
        uint64_t value() const override { return m_value->value(); };

        Counter() : ValueProxy(makeCounter()) {};
        Counter(std::shared_ptr<ICounterValue> value) : ValueProxy(value) {};
        Counter(const Counter&) = default;
        Counter(Counter&&) = default;
        ~Counter() = default;
    };

    class Gauge : public ValueProxy<IGaugeValue>
    {
    public:
        IGaugeValue& operator=(double value) override { return (*m_value = value); };
        IGaugeValue& operator+=(double value) override { return (*m_value += value); };
        IGaugeValue& operator-=(double value) override { return (*m_value -= value); };
        double value() const override { return m_value->value(); };

        Gauge() : ValueProxy(makeGauge()) {};
        Gauge(std::shared_ptr<IGaugeValue> value) : ValueProxy(value) {};
        Gauge(const Gauge&) = default;
        Gauge(Gauge&&) = default;
        ~Gauge() = default;
    };

    class Summary : public ValueProxy<ISummary>
    {
    public:
        ISummary& observe(double value) override { return m_value->observe(value); };
        uint64_t count() const override { return m_value->count(); };
        double sum() const override { return m_value->sum(); };
        std::vector<std::pair<double, uint64_t>> values() const override { return m_value->values(); };

        Summary(const std::vector<double>& quantiles, double error = 0.01) : ValueProxy(makeSummary(quantiles, error)) {};
        Summary(std::shared_ptr<ISummary> value) : ValueProxy(value) {};
        Summary(const Summary&) = default;
        Summary(Summary&&) = default;
        ~Summary() = default;
    };

    class Histogram : public ValueProxy<IHistogram>
    {
    public:
        IHistogram& observe(double value) override { return m_value->observe(value); };
        uint64_t count() const override { return m_value->count(); };
        double sum() const override { return m_value->sum(); };
        std::vector<std::pair<double, uint64_t>> values() const override { return m_value->values(); };

        Histogram(const std::vector<double>& bounds) : ValueProxy(makeHistogram(bounds)) {};
        Histogram(std::shared_ptr<IHistogram> value) : ValueProxy(value) {};
        Histogram(const Histogram&) = default;
        Histogram(Histogram&&) = default;
        ~Histogram() = default;
    };

    class Text : public ValueProxy<IText>
    {
    public:
        IText& operator=(std::string value) { return (*m_value = value); };
        std::string value() const override { return m_value->value(); };

        Text() : ValueProxy(makeText()) {};
        Text(std::shared_ptr<IText> value) : ValueProxy(value) {};
        Text(const Text&) = default;
        Text(Text&&) = default;
        ~Text() = default;
    };
#pragma endregion
}
