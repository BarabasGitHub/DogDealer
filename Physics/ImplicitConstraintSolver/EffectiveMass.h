#pragma once

namespace Physics
{
    struct Inertia;
    struct Movement;

    float CalculateInverseEffectiveMass(Movement const & movement, Inertia const & inverse_inertia);

    float CalculateInverseEffectiveMass(Movement const & movement0, Movement const & movement1, Inertia const & inverse_inertia0, Inertia const & inverse_inertia1);
}