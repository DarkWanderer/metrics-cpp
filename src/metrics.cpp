#include <metrics/gauge.h>
#include <metrics/counter.h>
#include <metrics/histogram.h>

#include <atomic>
#include <vector>
#include <map>
// #include <metrics/vecmap.h>

namespace Metrics
{
	IGaugeValue::~IGaugeValue()
	{
	}
	ICounterValue::~ICounterValue()
	{
	}
	IHistogram::~IHistogram() {
	}

	class GaugeImpl : public IGaugeValue
	{
	private:
		std::atomic<double> m_value;

	public:
		GaugeImpl() = default;
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

		double value() override
		{
			return m_value.load();
		}
	};
	class CounterImpl : public ICounterValue
	{
	private:
		std::atomic<uint64_t> m_value;

	public:
		CounterImpl() = default;
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
		// TODO: vector-based storage
		std::map<double, CounterImpl> m_buckets;
		CounterImpl m_count;
		GaugeImpl m_sum;
	public:
		HistogramImpl(const std::vector<double>& bounds)
		{
			for (auto bound : bounds)
				m_buckets[bound].reset();
			m_buckets[std::numeric_limits<double>::infinity()].reset();
		}

		void observe(double value) override {
			for (auto& bucket : m_buckets)
			{
				if (bucket.first >= value)
					bucket.second++;
			}
			m_count++;
			m_sum += value;
		}

		uint64_t count() override { return m_count; };
		double sum() override { return m_sum; };
	};

	std::shared_ptr<ICounterValue> createCounter() { return std::make_shared<CounterImpl>(); };
	std::shared_ptr<IGaugeValue> createGauge() { return std::make_shared<GaugeImpl>(); };
	std::shared_ptr<IHistogram> createHistogram(const std::vector<double>& bounds) { return std::make_shared<HistogramImpl>(bounds); };
}
