#include <metrics/metric.h>

#include <algorithm>
#include <atomic>
#include <list>
#include <map>
#include <mutex>
#include <numeric>
#include <vector>
#include <iterator>

using namespace std;

namespace Metrics
{
    void ICounterValue::accept(IMetricVisitor& visitor) { visitor.visit(*this); }
    void IGaugeValue::accept(IMetricVisitor& visitor) { visitor.visit(*this); }
    void ISummary::accept(IMetricVisitor& visitor) { visitor.visit(*this); }
    void IHistogram::accept(IMetricVisitor& visitor) { visitor.visit(*this); }
    
    ICounterValue::~ICounterValue() { }
	IGaugeValue::~IGaugeValue() { }
	ISummary::~ISummary() { }
	IHistogram::~IHistogram() { }

	class GaugeImpl : public IGaugeValue
	{
	private:
		atomic<double> m_value;

	public:
		GaugeImpl() { m_value.store(0.); };
		GaugeImpl(GaugeImpl&&) = delete;
		GaugeImpl(const GaugeImpl&) = delete;
		~GaugeImpl() = default;

		IGaugeValue& operator=(double value) override
		{
			m_value.store(value);
			return *this;
		};

		IGaugeValue& operator+=(double value) override
		{
			// TODO: revisit memory ordering
			double oldv, newv;
			do
			{
				oldv = m_value.load();
				newv = oldv + value;
			} while (!m_value.compare_exchange_weak(oldv, newv));
			return *this;
		};

		IGaugeValue& operator-=(double value) override
		{
			// TODO: revisit memory ordering
			double oldv, newv;
			do
			{
				oldv = m_value.load();
				newv = oldv - value;
			} while (!m_value.compare_exchange_weak(oldv, newv));
			return *this;
		};

		double value() const override
		{
			return m_value.load();
		}
	};

	class CounterImpl : public ICounterValue
	{
	private:
		atomic<uint64_t> m_value;

	public:
        CounterImpl() noexcept { m_value.store(0); };
        CounterImpl(uint64_t value) noexcept { m_value.store(value); };
		CounterImpl(CounterImpl&& other) = delete;
		CounterImpl(const CounterImpl&) = delete;
		~CounterImpl() = default;
		ICounterValue& operator++(int) override
		{
			m_value.fetch_add(1, std::memory_order_acq_rel);
			return *this;
		};
		ICounterValue& operator+=(uint32_t value) override
		{
			m_value.fetch_add(value, std::memory_order_acq_rel);
			return *this;
		};
		uint64_t value() const override
		{
			return m_value.load(std::memory_order_acquire);
		};
		void reset() override
		{
			m_value.store(0, std::memory_order_release);
		};
	};


	class HistogramImpl : public IHistogram {
	private:
        const vector<double> m_bounds;
        vector<CounterImpl> m_counts;
		GaugeImpl m_sum;

        static vector<double> preprocessBounds(const vector<double>& input) {
            vector<double> bounds;

            // Reserve extra for +Inf
            bounds.reserve(bounds.size() + 1); 
            copy(input.begin(), input.end(), back_inserter(bounds));

            // Add mandatory +Inf bound
            bounds.push_back(numeric_limits<double>::infinity());
            sort(bounds.begin(), bounds.end());
            auto last = unique(bounds.begin(), bounds.end());
            bounds.erase(last, bounds.end());
            return bounds;
        }
	public:
		HistogramImpl(const vector<double>& bounds) :
            m_bounds(preprocessBounds(bounds)), m_counts(m_bounds.size()), m_sum()
		{
		}

		HistogramImpl(const HistogramImpl&) = delete;

		IHistogram& observe(double value) override {
            m_sum += value;
            auto bound = lower_bound(m_bounds.begin(), m_bounds.end(), value); // Guaranteed to find because of the infinity bound
            auto index = distance(m_bounds.begin(), bound);
            m_counts[index]++;
			return *this;
		}

		vector<pair<double, uint64_t>> values() const override
		{
			vector<pair<double, uint64_t>> result;
            const auto size = m_bounds.size();
			result.reserve(size);
            uint64_t running_total = 0;
            for (size_t i = 0; i < size; i++)
			{
                running_total += m_counts[i].value();
                result.emplace_back(m_bounds[i], running_total);
			}
			return result;
		};

        uint64_t count() const override {
            uint64_t result = 0;
            for (auto& c : m_counts) {
                result += c.value();
            }
            return result;
        };

		double sum() const override { return m_sum; };
	};

	// Definitions for functions referenced in registry.cpp
	std::shared_ptr<ICounterValue> makeCounter() { return std::make_shared<CounterImpl>(); };
	std::shared_ptr<IGaugeValue> makeGauge() { return std::make_shared<GaugeImpl>(); };
	std::shared_ptr<IHistogram> makeHistogram(const vector<double>& bounds) { return std::make_shared<HistogramImpl>(bounds); };
}
