#pragma once

#include "MinMax.h"
#include <Math\MathFunctions.h>

template<typename Type> auto Combine(MinMax<Type> a, MinMax<Type> b);

template<typename Type> auto Update(MinMax<Type> a, Type const & b);



// implementations
template<typename Type> auto Combine(MinMax<Type> a, MinMax<Type> b)
{
    using namespace Math;
    MinMax<Type> c;
    c.min = Min(a.min, b.min);
    c.max = Max(a.max, b.max);
    return c;
}


template<typename Type> auto Update(MinMax<Type> a, Type const & b)
{
    using namespace Math;
    MinMax<Type> c;
    c.min = Min(a.min, b);
    c.max = Max(a.max, b);
    return c;
}
