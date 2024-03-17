#include <metrics/metric.h>

#include <algorithm>
#include <mutex>
#include <atomic>
#include <vector>
#include <list>
#include <map>

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
        vector<double> m_bounds;
        vector<CounterImpl> m_counts;
		CounterImpl m_count;
		GaugeImpl m_sum;

	public:
		HistogramImpl(const vector<double>& bounds)
		{
            m_bounds = bounds;
            sort(m_bounds.begin(), m_bounds.end());
            auto last = unique(m_bounds.begin(), m_bounds.end());
            m_bounds.erase(last, m_bounds.end());
            m_counts = vector<CounterImpl>(m_bounds.size());
		}

		HistogramImpl(const HistogramImpl&) = delete;

		IHistogram& observe(double value) override {
            m_count++;
            m_sum += value;
            auto bound = lower_bound(m_bounds.begin(), m_bounds.end(), value);
            if (bound != m_bounds.end())
            {
                auto index = distance(m_bounds.begin(), bound);
                m_counts[index]++;
            }
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

		uint64_t count() const override { return m_count; };
		double sum() const override { return m_sum; };
	};

	// Definitions for functions referenced in registry.cpp
	std::shared_ptr<ICounterValue> makeCounter() { return std::make_shared<CounterImpl>(); };
	std::shared_ptr<IGaugeValue> makeGauge() { return std::make_shared<GaugeImpl>(); };
	std::shared_ptr<IHistogram> makeHistogram(const vector<double>& bounds) { return std::make_shared<HistogramImpl>(bounds); };
}
