#pragma once

#include <metrics_export.h>
#include <metrics/metric.h>

#include <memory>

namespace Metrics
{
    class IHistogram : public ITypedMetric<TypeCode::Histogram>
    {
    public:
        virtual void observe(double value) = 0;
        virtual uint64_t count() = 0;
        virtual double sum() = 0;

    protected:
        METRICS_EXPORT virtual ~IHistogram() = 0;
    };

    // Stack-based wrapper for actual metric
    class Histogram : public ValueProxy<IHistogram>
    {
    private:
        const std::shared_ptr<IHistogram> m_value;

    public:
        void observe(double value) override { m_value->observe(value); };
        uint64_t count() override { return m_value->count(); };
        double sum() override { return m_value->sum(); };

        Histogram(std::shared_ptr<IHistogram> value) : m_value(value) {};
        Histogram(const Histogram& other) = default;
        Histogram(Histogram&& other) = default;
        ~Histogram() = default;
    };
}
