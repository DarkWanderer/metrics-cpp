#include <metrics/metric.h>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

#include <algorithm>
#include <mutex>
#include <atomic>
#include <vector>
#include <list>
#include <map>

using std::vector;
using std::mutex;
using std::unique_lock;
using std::pair;
using std::sort;

using namespace boost::accumulators;

namespace Metrics {
    // Implements CKMS algorithm for approximate quantile calculation
    // Utilizes locking internally. Using Histogram is recommended instead
    class SummaryImpl : public ISummary {
    private:
        typedef accumulator_set<double, stats<tag::count, tag::sum, tag::tail_quantile<right>>> accumulator_t;

        mutable mutex m_mutex;
        const vector<double> m_quantiles;
        accumulator_t m_accumulator;

    public:
        SummaryImpl(const vector<double>& quantiles, double error) :
            m_quantiles(quantiles),
            m_accumulator(tag::tail<right>::cache_size = 100)
        {
        }

        SummaryImpl(const SummaryImpl&) = delete;

        ISummary& observe(double value) override {
            unique_lock<mutex> lock(m_mutex);
            m_accumulator(value);
            return *this;
        }

        vector<pair<double, uint64_t>> values() const override
        {
            unique_lock<mutex> lock(m_mutex);
            vector<pair<double, uint64_t>> result;
            for (auto q : m_quantiles) {
                result.emplace_back(q, boost::accumulators::quantile(m_accumulator, quantile_probability = q));
            }
            return result;
        };

        uint64_t count() const override {
            unique_lock<mutex> lock(m_mutex);
            return boost::accumulators::count(m_accumulator);
        };

        double sum() const override {
            unique_lock<mutex> lock(m_mutex);
            return boost::accumulators::sum(m_accumulator);;
        };
    };

    std::shared_ptr<ISummary> makeSummary(const vector<double>& quantiles, double error) { 
        // Explicit copy
        auto q = quantiles;
        sort(q.begin(), q.end());
        return std::make_shared<SummaryImpl>(q, error);
    };
}
