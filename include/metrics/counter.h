#pragma once

#include <metrics_export.h>
#include <metrics/metric.h>

#include <memory>

namespace Metrics
{
    class ICounterValue : public IMetric
    {
    public:
        virtual ICounterValue &operator++(int) = 0;
        virtual ICounterValue &operator+=(uint32_t value) = 0;
        virtual uint64_t value() const = 0;
        virtual void reset() = 0;
        operator uint64_t() { return value(); };

    protected:
        METRICS_EXPORT virtual ~ICounterValue() = 0;
    };

    // Stack-based wrapper for actual metric
    class Counter : public ICounterValue
    {
    private:
        const std::shared_ptr<ICounterValue> m_value;

    public:
        ICounterValue &operator++(int) override { return (*m_value)++; };
        ICounterValue &operator+=(uint32_t value) override { return (*m_value += value); };
        void reset() override { m_value->reset(); };
        uint64_t value() const override { return m_value->value(); };

        Counter(std::shared_ptr<ICounterValue> value) : m_value(value){};
        Counter(const Counter &other) = default;
        Counter(Counter &&other) = default;
        ~Counter() = default;
    };
}
