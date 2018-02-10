#include "PointFunctions.h"
#include "MathFunctions.h"

using namespace Math;

uint32_t Math::FindFurthestPointIndex(Float3 const reference_point, Range< Float3 const *> positions )
{
    auto furthest_index = 0u;
    float max_square_distance = 0;
    for( auto i = 0u; i < Size(positions); ++i )
    {
        auto squared_distance = SquaredNorm(positions[i] - reference_point);
        if( squared_distance > max_square_distance )
        {
            furthest_index = i;
            max_square_distance = squared_distance;
        }
    }
    return furthest_index;
}


uint32_t Math::FindFurthestPointIndex(Range<Float3 const * __restrict> reference_points, Range< Float3 const  * __restrict> positions )
{
    auto furthest_index = 0u;
    float max_distance = 0;
    auto point_count = Size(reference_points);
    for( auto i = 0u; i < Size(positions); ++i )
    {
        float total_distance = 0;
        for( auto j = 0u; j < point_count; ++j )
        {
            total_distance += Norm(positions[i] - reference_points[j]);
        }

        if( total_distance > max_distance )
        {
            furthest_index = i;
            max_distance = total_distance;
        }
    }
    return furthest_index;
}


uint32_t Math::FindClosestPointIndex(uint32_t reference_index, Range<Float3 const *> points)
{
    assert(reference_index < Size(points));
    auto reference_point = points[reference_index];
    float min_distance = std::numeric_limits<float>::max();
    uint32_t index = reference_index;
    for( auto i = 0u; i < Size(points); ++i )
    {
        auto distance = SquaredNorm(reference_point - points[i]);
        if( distance < min_distance && i != reference_index )
        {
            min_distance = distance;
            index = i;
        }
    }
    return index;
}
