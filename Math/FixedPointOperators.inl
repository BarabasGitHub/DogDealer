#pragma once

namespace Math
{   template<typename UnderlyingType, size_t NumberOfFractionBits>
    FixedPointTemplate<UnderlyingType, NumberOfFractionBits> Math::operator-(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a)
    {
        return{ { -a.full } };
    }

    template<typename UnderlyingType, size_t NumberOfFractionBits>
    FixedPointTemplate<UnderlyingType, NumberOfFractionBits> Math::operator-(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b)
    {
        return{ { a.full - b.full } };
    }

    template<typename UnderlyingType, size_t NumberOfFractionBits>
    FixedPointTemplate<UnderlyingType, NumberOfFractionBits>& Math::operator-=(FixedPointTemplate<UnderlyingType, NumberOfFractionBits>& a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b)
    {
        a.full -= b.full;
        return a;
    }

    template<typename UnderlyingType, size_t NumberOfFractionBits>
    FixedPointTemplate<UnderlyingType, NumberOfFractionBits> Math::operator+(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a)
    {
        return a;
    }

    template<typename UnderlyingType, size_t NumberOfFractionBits>
    FixedPointTemplate<UnderlyingType, NumberOfFractionBits> Math::operator+(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b)
    {
        return{ { a.full + b.full } };
    }

    template<typename UnderlyingType, size_t NumberOfFractionBits>
    FixedPointTemplate<UnderlyingType, NumberOfFractionBits>& Math::operator+=(FixedPointTemplate<UnderlyingType, NumberOfFractionBits>& a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b)
    {
        a.full += b.full;
        return a;
    }

    template<typename UnderlyingType, size_t NumberOfFractionBits>
    bool Math::operator==(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b)
    {
        return a.full == b.full;
    }

    template<typename UnderlyingType, size_t NumberOfFractionBits>
    bool Math::operator!=(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b)
    {
        return a.full != b.full;
    }

    template<typename UnderlyingType, size_t NumberOfFractionBits>
    bool Math::operator>(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b)
    {
        return a.full > b.full;
    }

    template<typename UnderlyingType, size_t NumberOfFractionBits>
    bool Math::operator>=(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b)
    {
        return a.full >= b.full;
    }

    template<typename UnderlyingType, size_t NumberOfFractionBits>
    bool Math::operator<(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b)
    {
        return a.full < b.full;
    }

    template<typename UnderlyingType, size_t NumberOfFractionBits>
    bool Math::operator<=(FixedPointTemplate<UnderlyingType, NumberOfFractionBits> a, FixedPointTemplate<UnderlyingType, NumberOfFractionBits> b)
    {
        return a.full <= b.full;
    }

}