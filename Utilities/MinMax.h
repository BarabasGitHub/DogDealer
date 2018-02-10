#pragma once

template<typename DataType>
struct MinMax
{
    DataType min, max;

    MinMax() = default;
    MinMax(DataType min, DataType max): min(min), max(max) {};
};