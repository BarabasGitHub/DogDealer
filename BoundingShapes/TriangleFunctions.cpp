#include "TriangleFunctions.h"

#include <Math\MathFunctions.h>
#include <Math\FloatOperators.h>

namespace BoundingShapes
{

    Math::Float3 GetAxis( Triangle const & triangle )
    {
        auto edge01 = triangle.corners[1] - triangle.corners[0];
        auto edge02 = triangle.corners[2] - triangle.corners[0];
        return Cross( edge01, edge02 );
    }


    Math::Float3 GetNormal( Triangle const & triangle )
    {
        return Normalize( GetAxis( triangle ) );
    }


    Math::Float3 GetCenter( Triangle const & triangle )
    {
        auto center = triangle.corners[0] + triangle.corners[1] + triangle.corners[2];
        return center / 3;
    }


    float Area(Triangle const & triangle)
    {
        return Norm( GetAxis( triangle ) );
    }


    float SquaredArea(Triangle const & triangle)
    {
        return SquaredNorm( GetAxis( triangle ) );
    }


    Triangle Transform( Triangle triangle, Math::Float4x4 const & transform )
    {
        for( auto & corner : triangle.corners )
        {
            corner = Math::TransformPosition( corner, transform );
        }
        return triangle;
    }


    Triangle TransformByOrientation( Triangle triangle, Orientation const & orientation )
    {
        for( auto & corner : triangle.corners )
        {
            corner = Rotate( corner, orientation.rotation );
            corner += orientation.position;
        }
        return triangle;
    }


    Triangle Translate( Triangle triangle, Math::Float3 translation )
    {
        for( auto & corner : triangle.corners )
        {
            corner += translation;
        }
        return triangle;
    }


    std::array<Math::Float3, 3> GetEdges( Triangle const & triangle )
    {
        std::array<Math::Float3, 3> edges;
        GetEdges( triangle, edges );
        return edges;
    }


    void GetEdges( Triangle const & triangle, Range<Math::Float3 * __restrict> output )
    {
        assert( Size( output ) == 3 );
        output[0] = triangle.corners[1] - triangle.corners[0];
        output[1] = triangle.corners[2] - triangle.corners[1];
        output[2] = triangle.corners[0] - triangle.corners[2];
    }


    Math::Float3 GetNormalFromTriangleEdges( std::array<Math::Float3, 3> const & edges )
    {
        return Normalize( Cross( edges[1], edges[0] ) );
    }
}