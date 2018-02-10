#include "FrustumFunctions.h"

#include "PlaneFunctions.h"

#include <Math\FloatMatrixOperators.h>

#include <array>

namespace
{
    // homogeneous corner coordinates
    std::array<Math::Float4, 8> homogeneous_corner_coordinates = {{
        {-1,  1,  1,  1},
        { 1,  1,  1,  1},
        { 1, -1,  1,  1},
        {-1, -1,  1,  1},
        {-1,  1,  0,  1},
        { 1,  1,  0,  1},
        { 1, -1,  0,  1},
        {-1, -1,  0,  1},
    }};

}

void BoundingShapes::GetFrustumCorners(Math::Float4x4 const & projection_matrix, Range<Math::Float3 *> corners)
{
    assert(Size(corners) == 8);
    auto inverse_projection_matrix = Inverse(projection_matrix);
    for( auto j = 0u; j < 8; ++j )
    {
        auto corner4 = inverse_projection_matrix * homogeneous_corner_coordinates[j];
        corner4 /= corner4.w;
        corners[j] = Math::Float3( corner4.x, corner4.y, corner4.z );
    }
}


BoundingShapes::Plane BoundingShapes::GetNearPlane(Math::Float4x4 const & projection_matrix)
{
    return CreatePlane(projection_matrix[2]);
}


BoundingShapes::Plane BoundingShapes::GetFarPlane(Math::Float4x4 const & projection_matrix)
{
    return CreatePlane(projection_matrix[3] - projection_matrix[2]);
}


BoundingShapes::Plane BoundingShapes::GetRightPlane(Math::Float4x4 const & projection_matrix)
{
    return CreatePlane(projection_matrix[3] - projection_matrix[0]);
}


BoundingShapes::Plane BoundingShapes::GetLeftPlane(Math::Float4x4 const & projection_matrix)
{
    return CreatePlane(projection_matrix[3] + projection_matrix[0]);
}


BoundingShapes::Plane BoundingShapes::GetTopPlane(Math::Float4x4 const & projection_matrix)
{
    return CreatePlane(projection_matrix[3] - projection_matrix[1]);
}


BoundingShapes::Plane BoundingShapes::GetBottomPlane(Math::Float4x4 const & projection_matrix)
{
    return CreatePlane(projection_matrix[3] + projection_matrix[1]);
}

