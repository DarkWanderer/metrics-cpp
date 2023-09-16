#pragma once
#include <metrics/metric.h>
#include <chrono>

namespace Metrics
{
    template<typename TDuration=std::chrono::seconds> class Timer : IMetricVisitor {
    private:
        std::shared_ptr<IMetric> m_metric;
        std::chrono::time_point<std::chrono::steady_clock> m_start;
    public:
        template<typename TValueProxy> Timer(TValueProxy proxy) : Timer(proxy.raw()) { }

        Timer(std::shared_ptr<IMetric> metric) :
            m_metric(metric),
            m_start(std::chrono::steady_clock::now())
        {}

        ~Timer()
        {
            m_metric->accept(*this);
        }

        TDuration elapsed()
        {
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration_cast<TDuration>(now - m_start);
        }
    private:
        virtual void visit(ICounterValue& v) override
        {
            v += elapsed().count();
        }

        virtual void visit(IGaugeValue& v) override
        {
            v = elapsed().count();
        }

        virtual void visit(ISummary& v) override
        {
            v.observe(elapsed().count());
        }

        virtual void visit(IHistogram& v) override
        {
            v.observe(elapsed().count());
        }

        virtual void visit(IText& v) override
        {
            ;
        }
    };
}
