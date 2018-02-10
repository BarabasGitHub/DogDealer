#pragma once

#include <cstdint>

struct Time
{
    // ticks since some specified time point
    uint64_t ticks;
};


inline bool operator<(Time a, Time b)
{
    return a.ticks < b.ticks;
}


inline bool operator>(Time a, Time b)
{
    return a.ticks > b.ticks;
}


inline bool operator>=(Time a, Time b)
{
    return a.ticks >= b.ticks;
}


inline bool operator<=(Time a, Time b)
{
    return a.ticks <= b.ticks;
}