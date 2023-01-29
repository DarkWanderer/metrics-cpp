#pragma once

#include <metrics_export.h>
#include <metrics/metric.h>

#include <memory>

namespace Metrics
{
    class IGaugeValue : public IMetric
    {
    public:
        virtual IGaugeValue &operator=(double value) = 0;
        virtual IGaugeValue &operator+=(double value) = 0;
        virtual IGaugeValue &operator-=(double value) = 0;
        virtual double value() = 0;
        operator double() { return value(); };

    protected:
        METRICS_EXPORT virtual ~IGaugeValue() = 0;
        IGaugeValue() = default;
        IGaugeValue(IGaugeValue&&) noexcept = default;
    };

    // Stack-based container referencing actual metric
    class Gauge : public IGaugeValue
    {
    private:
        const std::shared_ptr<IGaugeValue> m_value;

    public:
        IGaugeValue &operator=(double value) override
        {
            *m_value = value;
            return *this;
        };
        IGaugeValue &operator+=(double value) override
        {
            *m_value += value;
            return *this;
        };
        IGaugeValue &operator-=(double value) override
        {
            *m_value -= value;
            return *this;
        };
        double value() override { return m_value->value(); };

        Gauge(std::shared_ptr<IGaugeValue> value) : m_value(value){};
        Gauge(const Gauge &other) = default;
        Gauge(Gauge &&other) = default;
        ~Gauge() = default;
    };
}
