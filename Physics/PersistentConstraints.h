#pragma once

#include "Constraints.h"

#include <Utilities\Range.h>

#include <vector>

namespace Physics
{
    struct PerstistentConstraints
    {
        DistanceConstraints distance_constraints;
        PositionConstraints position_constraints;
        RotationConstraints rotation_constraints;

        VelocityConstraints velocity_constraints;
    };


    void Remove(BodyID id, PerstistentConstraints & self);
    void Remove(Range<BodyID const*> ids, PerstistentConstraints & self);
}
