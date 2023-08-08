#pragma once
#include <metrics/metric.h>
#include <chrono>

namespace Metrics
{
    template<typename TDuration> class Timer {
    private:
        std::shared_ptr<ICounterValue> m_counter;
        std::chrono::time_point<std::chrono::steady_clock> m_start;
    public:
        Timer(ICounterValue counter) 
        {
            m_start = std::chrono::steady_clock::now();
            m_counter = counter;
        }
        Timer(Counter counter)
        {
            m_start = std::chrono::steady_clock::now();
            m_counter = counter.raw();
        }
        ~Timer()
        {
            auto now = std::chrono::steady_clock::now();
            auto diff = now - m_start;
            m_counter += std::chrono::duration_cast<TDuration>(diff).count();
        }
    };
}
