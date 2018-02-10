#pragma once

#include <Math\FLoatTypes.h>

namespace Physics
{
    struct Inertia;
    struct Movement;

    struct MovementToEffectiveMovement
    {
        Math::Float3 linear, angular;
    };

    MovementToEffectiveMovement operator/(MovementToEffectiveMovement const & a, float b);

    MovementToEffectiveMovement CalculateMovementToEffectiveMovement(Math::Float3 const & direction_in, Math::Float3 const & position_in, Inertia const & inverse_inertia, float inverse_effective_mass);

    float CalculateEffectiveMovement(Movement const & movement, MovementToEffectiveMovement const & movement_to_effective_movement);
}