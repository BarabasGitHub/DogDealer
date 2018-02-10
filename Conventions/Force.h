#pragma once

#include "EntityID.h"
#include <Math\FloatTypes.h>

#include <vector>

typedef Math::Float3 Force; // [kg m s-²]

struct EntityForces
{
    std::vector<Math::Float3> forces;
    std::vector<EntityID> entity_ids;
};

typedef Math::Float3 Torque; // [kg m² s-²]

struct EntityTorques
{
    std::vector<Math::Float3> torques;
    std::vector<EntityID> entity_ids;
};