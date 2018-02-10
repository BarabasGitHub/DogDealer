#pragma once

#include <BoundingShapes\AxisAlignedBox.h>

#include <Utilities\Range.h>

namespace Physics
{
    struct Movement;
    struct Inertia;

    void ApplyDrag( 
        Range<Movement *> movements, 
        Range<Inertia const *> inertias, 
        Range<BoundingShapes::AxisAlignedBox const *> boxes, 
        float const time_step 
        );

    // reduces the movement by a fraction which will amount to a total of fraction_per_second each second
    void ApplyInternalFriction( 
        Range<Movement const *> movements, 
        float const fraction_per_second, 
        float const time_step, 
        Range<Movement *> result_movements 
        );

    float CombineFrictionFactors( 
        float f1, 
        float f2 
        );
}