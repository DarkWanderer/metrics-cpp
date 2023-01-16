#include <metrics/histogram.h>

#include "factory.h"

#include <map>
#include <vector>
#include <limits>

namespace Metrics 
{
    IHistogram::~IHistogram() {

    }

    class HistogramImpl : public IHistogram {
    private:
        // TODO: vector-based storage
        std::map<double, Counter> m_buckets;
        Counter m_count;
        Gauge m_sum;
    public:
        HistogramImpl(const std::vector<double>& bounds) :
            m_count(createCounter()), m_sum(createGauge())
        {
            for (auto bound: bounds)
                m_buckets.emplace(bound, createCounter());
            m_buckets.emplace(std::numeric_limits<double>::infinity(), createCounter());
        }

        void observe(double value) override {
            for (auto it = m_buckets.begin(); it != m_buckets.end(); it++)
            {
                if (it->first >= value)
                    it->second++;
            }
            m_count++;
            m_sum += value;
        }

        uint64_t count() override { return m_count; };
        double sum() override { return m_sum; };
    };

    std::shared_ptr<IHistogram> Metrics::createHistogram(const std::vector<double>& bounds)
    {
        return std::make_shared<HistogramImpl>(bounds);
    }
}
