#include <gauge.h>

#include <atomic>

namespace Metrics
{
    IGaugeValue::~IGaugeValue()
    {
    }

    class GaugeImpl : public IGaugeValue
    {
    private:
        std::atomic<double> m_value;

    public:
        GaugeImpl() = default;
        GaugeImpl(const GaugeImpl &) = delete;
        ~GaugeImpl() = default;

        IGaugeValue &operator=(double value) override
        {
            m_value.store(value);
            return *this;
        };
        
        IGaugeValue &operator+=(double value) override
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

        IGaugeValue &operator-=(double value) override
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

    std::shared_ptr<IMetric> createGauge()
    {
        return std::make_shared<GaugeImpl>();
    }
}
