#include <counter.h>

#include <atomic>

namespace Metrics
{
    ICounterValue::~ICounterValue()
    {
    }

    class CounterImpl : public ICounterValue
    {
    private:
        std::atomic<uint64_t> m_value;

    public:
        CounterImpl() = default;
        CounterImpl(const CounterImpl &) = delete;
        ~CounterImpl() = default;
        ICounterValue &operator++(int) override
        {
            m_value++;
            return *this;
        };
        ICounterValue &operator+=(uint32_t value) override
        {
            m_value += value;
            return *this;
        };
        uint64_t value() const override
        {
            return m_value.load();
        }
        void reset() override
        {
            m_value.store(0);
        }
    };

    std::shared_ptr<IMetric> createCounter()
    {
        return std::make_shared<CounterImpl>();
    }
}
