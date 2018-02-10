#pragma once

#include <Math\FloatTypes.h>

namespace Physics
{
    struct Movement
    {
        Math::Float3 momentum; // [kg m s-¹]
        Math::Float3 angular_momentum; // [kg m² s-¹]
    };

    bool Equal( Movement const& a, Movement const& b, float const absolute_tolerance );
    Movement operator-(Movement const & a);
    Movement operator+(Movement const & a, Movement const & b);
    Movement& operator+=(Movement & a, Movement const & b);
    Movement operator-(Movement const & a, Movement const & b);
    Movement& operator-=(Movement & a, Movement const & b);
    Movement operator*(float a, Movement const & b);
    Movement operator*(Movement const & a, float b);
    Movement& operator*=(Movement & a, float b);
}
