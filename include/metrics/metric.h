#pragma once

#include <metrics/labels.h>
#include <metrics_export.h>

#include <string>
#include <memory>

namespace Metrics
{
#pragma region Forward declarations
    class IGaugeValue;
    class ICounterValue;
    class ISummary;
    class IHistogram;

    METRICS_EXPORT std::shared_ptr<IGaugeValue> makeGauge();
    METRICS_EXPORT std::shared_ptr<ICounterValue> makeCounter();
    METRICS_EXPORT std::shared_ptr<ISummary> makeSummary(const std::vector<double>& quantiles, double error);
    METRICS_EXPORT std::shared_ptr<IHistogram> makeHistogram(const std::vector<double>& bounds);
#pragma endregion

#pragma region Common definitions
    enum class TypeCode {
        Gauge,
        Counter,
        Summary,
        Histogram
    };

    class IMetric
    {
    public:
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
#pragma endregion

#pragma region Interfaces
    class ICounterValue : public ITypedMetric<TypeCode::Counter>
    {
    public:
        virtual ICounterValue& operator++(int) = 0;
        virtual ICounterValue& operator+=(uint32_t value) = 0;
        virtual uint64_t value() const = 0;
        virtual void reset() = 0;
        operator uint64_t() const { return value(); };

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

    protected:
        METRICS_EXPORT virtual ~IGaugeValue() = 0;
    };

    class ISummary : public ITypedMetric<TypeCode::Summary>
    {
    public:
        virtual void observe(double value) = 0;
        virtual uint64_t count() const = 0;
        virtual double sum() const = 0;
        virtual std::vector<std::pair<double, uint64_t>> values() const = 0;

    protected:
        METRICS_EXPORT virtual ~ISummary() = 0;
    };

    class IHistogram : public ITypedMetric<TypeCode::Histogram>
    {
    public:
        virtual void observe(double value) = 0;
        virtual uint64_t count() const = 0;
        virtual double sum() const = 0;
        virtual std::vector<std::pair<double, uint64_t>> values() const = 0;

    protected:
        METRICS_EXPORT virtual ~IHistogram() = 0;
    };
#pragma endregion

#pragma region Stack containers
    class Counter : public ValueProxy<ICounterValue>
    {
    private:
        const std::shared_ptr<ICounterValue> m_value;

    public:
        ICounterValue& operator++(int) override { return (*m_value)++; };
        ICounterValue& operator+=(uint32_t value) override { return (*m_value += value); };
        void reset() override { m_value->reset(); };
        uint64_t value() const override { return m_value->value(); };

        Counter() : m_value(makeCounter()) {};
        Counter(std::shared_ptr<ICounterValue> value) : m_value(value) {};
        Counter(const Counter&) = default;
        Counter(Counter&&) = default;
        ~Counter() = default;

        std::shared_ptr<IMetric> raw() { return m_value; }
    };

    class Gauge : public ValueProxy<IGaugeValue>
    {
    private:
        const std::shared_ptr<IGaugeValue> m_value;

    public:
        IGaugeValue& operator=(double value) override
        {
            *m_value = value;
            return *this;
        };
        IGaugeValue& operator+=(double value) override
        {
            *m_value += value;
            return *this;
        };
        IGaugeValue& operator-=(double value) override
        {
            *m_value -= value;
            return *this;
        };
        double value() const override { return m_value->value(); };

        Gauge() : m_value(makeGauge()) {};
        Gauge(std::shared_ptr<IGaugeValue> value) : m_value(value) {};
        Gauge(const Gauge&) = default;
        Gauge(Gauge&&) = default;
        ~Gauge() = default;

        std::shared_ptr<IMetric> raw() { return m_value; }
    };

    class Summary : public ValueProxy<ISummary>
    {
    private:
        const std::shared_ptr<ISummary> m_value;

    public:
        void observe(double value) override { m_value->observe(value); };
        uint64_t count() const override { return m_value->count(); };
        double sum() const override { return m_value->sum(); };
        std::vector<std::pair<double, uint64_t>> values() const override { return m_value->values(); };

        Summary(const std::vector<double>& quantiles, double error = 0.01) : m_value(makeSummary(quantiles, error)) {};
        Summary(std::shared_ptr<ISummary> value) : m_value(value) {};
        Summary(const Summary&) = default;
        Summary(Summary&&) = default;
        ~Summary() = default;
    };

    class Histogram : public ValueProxy<IHistogram>
    {
    private:
        const std::shared_ptr<IHistogram> m_value;

    public:
        void observe(double value) override { m_value->observe(value); };
        uint64_t count() const override { return m_value->count(); };
        double sum() const override { return m_value->sum(); };
        std::vector<std::pair<double, uint64_t>> values() const override { return m_value->values(); };

        Histogram(const std::vector<double>& bounds) : m_value(makeHistogram(bounds)) {};
        Histogram(std::shared_ptr<IHistogram> value) : m_value(value) {};
        Histogram(const Histogram&) = default;
        Histogram(Histogram&&) = default;
        ~Histogram() = default;

        std::shared_ptr<IMetric> raw() { return m_value; }
    };
#pragma endregion
}
