#pragma once

#include "MovementToEffectiveMovement.h"
#include "..\Movement.h"
#include <Utilities\MinMax.h>
#include <array>
#include <cstdint>

namespace Physics
{
    struct Constraint
    {
        std::array<uint32_t, 2> body_indices;
        float target_impulse;
        float total_impulse;
        float feedback_coefficient;
        MinMax<float> impulse_limits;
        float _padding;
        // the movement that will result from an impulse of magnitude 1
        alignas(16) std::array<Movement, 2> unit_impulses;
        // a dot product with the movement and this movement_to_effective_movement will result in the relevant speed component for this constraint
        alignas(16) std::array<MovementToEffectiveMovement, 2> movement_to_effective_movement;
    };


    struct SingleBodyConstraint
    {
        uint32_t body_index;
        float target_impulse;
        float total_impulse;
        float feedback_coefficient;
        MinMax<float> impulse_limits;
        // the movement that will result from an impulse of magnitude 1
        Movement unit_impulse;
        // a dot product with the movement and this movement_to_effective_movement will result in the relevant speed component for this constraint
        MovementToEffectiveMovement movement_to_effective_movement;
    };

}