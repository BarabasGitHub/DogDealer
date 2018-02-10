#pragma once

#include <Math\FloatMatrixTypes.h>

namespace Physics
{
    struct Inertia
    {
        Math::Float3x3 moment; // [kg m²]
        float mass; // [kg]

        Inertia() = default;
        Inertia(Math::Float3x3 const & moment, float mass) : moment(moment), mass(mass) {}
    };
}