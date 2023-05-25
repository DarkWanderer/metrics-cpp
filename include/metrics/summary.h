#pragma once
#include <metrics_export.h>
#include <metrics/metric.h>

namespace Metrics
{
    class ISummary : public IMetric
    {
    public:
        virtual void observe(double value) = 0;
        virtual uint64_t count() = 0;
        virtual double sum() = 0;

    protected:
        METRICS_EXPORT virtual ~ISummary() = 0;
    };

    class Summary : public ValueProxy<ISummary>
    {
    private:
        const std::shared_ptr<ISummary> m_value;

    public:
        void observe(double value) override { m_value->observe(value); };
        uint64_t count() override { return m_value->count(); };
        double sum() override { return m_value->sum(); };

        Summary(std::shared_ptr<ISummary> value) : m_value(value) {};
        Summary(const Summary& other) = default;
        Summary(Summary&& other) = default;
        ~Summary() = default;
    };
}
