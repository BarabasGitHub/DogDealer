#pragma once

#include "BodyID.h"

#include <Math\ForwardDeclarations.h>
#include <Utilities\MinMax.h>

#include <cstdint>
#include <vector>
#include <array>

namespace Physics
{
    // distance between the centers of two bodies (parent and child, although it doesn't really matter which one is which)
    struct DistanceConstraints
    {
        // parent and child
        std::vector<BodyPair> bodies;
        // parent and child attachment points
        std::vector<std::array<Math::Float3, 2>> attachment_points;
        std::vector<float> distances;
        std::vector<MinMax<float>> minmax_forces;
    };

    // position of one body (child) relative to another (parent) in the coordinate space of the parent
    struct PositionConstraints
    {
        // parent and child
        std::vector<BodyPair> bodies;
        // direction in parent local coordinates
        std::vector<Math::Float3> directions;
        // parent and child attachment points
        std::vector<std::array<Math::Float3, 2>> attachment_points;
        // distance to be kept between the attachment points on the two bodies
        std::vector<float> distances;
        // min and max forces to be used to keep the distance
        std::vector<MinMax<float>> minmax_forces;
    };

    // rotation of one body relative the world
    struct RotationConstraints
    {
        std::vector<BodyPair> bodies;
        std::vector<Math::Float3> rotation_normals;
        std::vector<float> target_angles;
        std::vector<MinMax<float>> minmax_torques;
    };

    // velocity of one body (child) relative to another (parent) in the coordinate space of the parent
    struct VelocityConstraints
    {
        // parent and child
        std::vector<BodyPair> bodies;
        // direction in parent local coordinates
        std::vector<Math::Float3> directions;
        // parent and child attachment points, both local to their own body
        std::vector<std::array<Math::Float3, 2>> attachment_points;
        // target speed to be kept between the attachment points on the two bodies
        std::vector<float> target_speeds;
        // min and max forces to be used to keep the distance
        std::vector<MinMax<float>> minmax_forces;
    };

    // rotation of one body relative the world
    struct WorldRotationConstraints
    {
        std::vector<BodyID> body_ids;
        std::vector<Math::Float3> rotation_normals;
        std::vector<float> target_angles;
        std::vector<MinMax<float>> minmax_torques;
    };

    // velocity of one body relative to the world
    struct WorldVelocityConstraints
    {
        std::vector<BodyID> body_ids;
        std::vector<Math::Float3> directions;
        std::vector<float> target_speeds;
        std::vector<MinMax<float>> minmax_force;
    };


    // angular velocity of one body in world space
    struct WorldAngularVelocityConstraints
    {
        std::vector<BodyID> body_ids;
        std::vector<Math::Float3> angular_directions;
        std::vector<float> angular_target_speeds;
        std::vector<MinMax<float>> minmax_torque;
    };
}
