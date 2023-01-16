#include <metrics/counter.h>

#include <atomic>
#include "factory.h"

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
            m_value.fetch_add(1, std::memory_order_acq_rel);
            return *this;
        };
        ICounterValue &operator+=(uint32_t value) override
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

    std::shared_ptr<ICounterValue> createCounter()
    {
        return std::make_shared<CounterImpl>();
    }
}
