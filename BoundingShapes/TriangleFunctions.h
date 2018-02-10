#pragma once

#include "Triangle.h"

#include <Conventions\Orientation.h>

#include <Math\MathFunctions.h>

#include <Utilities\Range.h>

namespace BoundingShapes
{
    Triangle CreateTriangle( Math::Float3 corner0, Math::Float3 corner1, Math::Float3 corner2 );

    Math::Float3 GetNormal( Triangle const & triangle );
    Math::Float3 GetAxis( Triangle const & triangle );
    Math::Float3 GetCenter( Triangle const & triangle );

    float Area(Triangle const & triangle);
    // is cheaper to compute than plain area
    float SquaredArea(Triangle const & triangle);

    Triangle Transform( Triangle triangle, Math::Float4x4 const & transform );
    Triangle TransformByOrientation( Triangle triangle, Orientation const & orientation );

    Triangle Translate( Triangle triangle, Math::Float3 translation );

    std::array<Math::Float3, 3> const & GetCorners( Triangle const & triangle );
    // the edges are in order: 0->1, 1->2, 2->0
    std::array<Math::Float3, 3> GetEdges( Triangle const & triangle );
    // the edges are in order: 0->1, 1->2, 2->0
    void GetEdges( Triangle const & triangle, Range<Math::Float3 * __restrict> output );
    // get the normal of the triangle from the edges, usually acquired via the GetEdges above
    Math::Float3 GetNormalFromTriangleEdges( std::array<Math::Float3, 3> const & edges );
}

// implementations
namespace BoundingShapes
{
    inline Triangle CreateTriangle( Math::Float3 corner0, Math::Float3 corner1, Math::Float3 corner2 )
    {
        return{ { { corner0, corner1, corner2 } } };
    }

    inline std::array<Math::Float3, 3> const & GetCorners( Triangle const & triangle )
    {
        triangle.corners;
    }
}