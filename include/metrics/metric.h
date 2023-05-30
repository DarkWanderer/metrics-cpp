#pragma once

#include <metrics/labels.h>
#include <metrics_export.h>

#include <string>
#include <memory>

namespace Metrics
{
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
        operator uint64_t() { return value(); };

    protected:
        METRICS_EXPORT virtual ~ICounterValue() = 0;
    };

    class IGaugeValue : public ITypedMetric<TypeCode::Gauge>
    {
    public:
        virtual IGaugeValue& operator=(double value) = 0;
        virtual IGaugeValue& operator+=(double value) = 0;
        virtual IGaugeValue& operator-=(double value) = 0;
        virtual double value() = 0;
        operator double() { return value(); };

    protected:
        METRICS_EXPORT virtual ~IGaugeValue() = 0;
    };

    class IHistogram : public ITypedMetric<TypeCode::Histogram>
    {
    public:
        virtual void observe(double value) = 0;
        virtual uint64_t count() = 0;
        virtual double sum() = 0;
        virtual std::vector<std::pair<double, uint64_t>> values() = 0;

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

        Counter(std::shared_ptr<ICounterValue> value) : m_value(value) {};
        Counter(const Counter& other) = default;
        Counter(Counter&& other) = default;
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
        double value() override { return m_value->value(); };

        Gauge(std::shared_ptr<IGaugeValue> value) : m_value(value) {};
        Gauge(const Gauge& other) = default;
        Gauge(Gauge&& other) = default;
        ~Gauge() = default;

        std::shared_ptr<IMetric> raw() { return m_value; }
    };
    class Histogram : public ValueProxy<IHistogram>
    {
    private:
        const std::shared_ptr<IHistogram> m_value;

    public:
        void observe(double value) override { m_value->observe(value); };
        uint64_t count() override { return m_value->count(); };
        double sum() override { return m_value->sum(); };
        std::vector<std::pair<double, uint64_t>> values() override { return m_value->values(); };

        Histogram(std::shared_ptr<IHistogram> value) : m_value(value) {};
        Histogram(const Histogram& other) = default;
        Histogram(Histogram&& other) = default;
        ~Histogram() = default;

        std::shared_ptr<IMetric> raw() { return m_value; }
    };
#pragma endregion
}
