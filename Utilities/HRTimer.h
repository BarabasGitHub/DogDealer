#pragma once
#include <chrono>

class HRTimer
{
private:
     std::chrono::high_resolution_clock clock;
     decltype(clock)::time_point start, stop;
public:
    inline void Start()
    {
        start = clock.now();
    }
    inline void Stop()
    {
        stop = clock.now();
    }
    auto GetDuration() const
    {
        return stop - start;
    }

    inline double GetSeconds() const
    {
        return std::chrono::duration_cast<std::chrono::duration<double>>(GetDuration()).count();
    }
    inline double GetMilliSeconds() const
    {
        return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(GetDuration()).count();
    }
};
