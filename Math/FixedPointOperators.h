#pragma once

#include "FixedPoint.h"
namespace Math
{
    template<typename UnderlyingType, size_t NumberOfFractionBits>
    FixedPointTemplate<UnderlyingType, NumberOfFractionBits> operator-(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a);
    template<typename UnderlyingType, size_t NumberOfFractionBits>
    FixedPointTemplate<UnderlyingType, NumberOfFractionBits> operator-(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b);
    template<typename UnderlyingType, size_t NumberOfFractionBits>
    FixedPointTemplate<UnderlyingType, NumberOfFractionBits>& operator-=(FixedPointTemplate<UnderlyingType, NumberOfFractionBits>& a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b);
    template<typename UnderlyingType, size_t NumberOfFractionBits>
    FixedPointTemplate<UnderlyingType, NumberOfFractionBits> operator+(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a);
    template<typename UnderlyingType, size_t NumberOfFractionBits>
    FixedPointTemplate<UnderlyingType, NumberOfFractionBits> operator+(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b);
    template<typename UnderlyingType, size_t NumberOfFractionBits>
    FixedPointTemplate<UnderlyingType, NumberOfFractionBits>& operator+=(FixedPointTemplate<UnderlyingType, NumberOfFractionBits>& a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b);

    template<typename UnderlyingType, size_t NumberOfFractionBits>
    bool operator==(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b);
    template<typename UnderlyingType, size_t NumberOfFractionBits>
    bool operator!=(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b);
    template<typename UnderlyingType, size_t NumberOfFractionBits>
    bool operator>(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b);
    template<typename UnderlyingType, size_t NumberOfFractionBits>
    bool operator>=(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b);
    template<typename UnderlyingType, size_t NumberOfFractionBits>
    bool operator<(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b);
    template<typename UnderlyingType, size_t NumberOfFractionBits>
    bool operator<=(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b);
}

#include "FixedPointOperators.inl"