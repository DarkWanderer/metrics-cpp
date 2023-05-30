#pragma once

#include <metrics_export.h>
#include <metrics/metric.h>

#include <memory>
#include <vector>

namespace Metrics
{
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

    // Stack-based wrapper for actual metric
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
}
