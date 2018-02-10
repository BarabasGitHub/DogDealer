#pragma once
#include <Math\FloatTypes.h>

#include <array>
#include <cstdint>

struct Manifold
{
    static const uint8_t c_max_contact_points = 4;

    std::array<float, c_max_contact_points> penetration_depths;
    // has to point from entity 2 to entity 1, that is to the entity on which the contact points are defined, in the direction it has to move to get separated
    std::array<Math::Float3, c_max_contact_points> separation_axes;
    // the contact positions are in the world coordinate frame, but relative to the position of the first entity
    std::array<Math::Float3, c_max_contact_points> positions;
    // age in game ticks of the contact points
    std::array<uint8_t, c_max_contact_points> ages;
    uint8_t contact_point_count;

    // sets everything to zero
    Manifold();
};


Math::Float3 GetAveragePosition(Manifold const & m);
Math::Float3 GetAverageSeparationDirection(Manifold const & m);
