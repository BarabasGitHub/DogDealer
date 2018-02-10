#pragma once

#include "EntityID.h"

#include <Math\ForwardDeclarations.h>

#include <Utilities\MinMax.h>

#include <vector>

struct VelocityConstraints
{
    std::vector<EntityID> entity_ids;
    std::vector<Math::Float3> directions;
    std::vector<float> target_speeds;
    std::vector<MinMax<float>> minmax_force;
};

struct AngularVelocityConstraints
{
    std::vector<EntityID> entity_ids;
    std::vector<Math::Float3> angular_directions;
    std::vector<float> angular_target_speeds;
    std::vector<MinMax<float>> minmax_torque;
};