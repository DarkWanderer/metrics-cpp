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
		CounterImpl() noexcept { m_value.store(0); };
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

	struct Bucket {
		double bound = 0.0;
		CounterImpl counter;
	};

	class HistogramImpl : public IHistogram {
	private:
		std::unique_ptr<Bucket[]> m_buckets;
		size_t m_bucket_count;
		CounterImpl m_count;
		GaugeImpl m_sum;
	public:
		HistogramImpl(const std::vector<double>& bounds)
		{
			m_bucket_count = bounds.size();
            m_buckets = std::unique_ptr<Bucket[]>(new Bucket[m_bucket_count]);
			for (int i = 0; i < m_bucket_count; i++) {
				m_buckets[i].bound = bounds[i];
			}
		}

		void observe(double value) override {
			for (int i = 0; i < m_bucket_count; i++)
			{
				auto& bucket = m_buckets[i];
				if (bucket.bound >= value)
					bucket.counter++;
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
