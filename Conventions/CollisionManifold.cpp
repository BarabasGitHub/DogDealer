#include "CollisionManifold.h"

#include <Math\MathFunctions.h>

#include <Utilities\Memory.h>

Manifold::Manifold()
{
    Zero(this, 1);
}


Math::Float3 GetAveragePosition(Manifold const & m)
{
    Math::Float3 average_position = 0;
    auto count = m.contact_point_count;
    for( auto i = 0u; i < count; ++i )
    {
        average_position += m.positions[i];
    }
    average_position /= count;
    return average_position;
}


Math::Float3 GetAverageSeparationDirection(Manifold const & m)
{
    Math::Float3 average_axis = 0;
    auto count = m.contact_point_count;
    for( auto i = 0u; i < count; ++i )
    {
        average_axis += m.separation_axes[i];
    }
    return Normalize(average_axis);
}