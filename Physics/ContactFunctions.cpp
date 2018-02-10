#include "ContactFunctions.h"

#include <BoundingShapes\AxisAlignedBoxFunctions.h>
#include <BoundingShapes\OrientedBoxFunctions.h>
#include <BoundingShapes\TriangleFunctions.h>
#include <BoundingShapes\PlaneFunctions.h>
#include <BoundingShapes\IntersectionTests.h>

#include <Math\MathFunctions.h>

#include <Utilities\IntegerRange.h>
#include <Utilities\Memory.h>
#include <Utilities\Logger.h>

#include <functional>

// input:
// - plane_normals & plane_origins,
// - vertices, each vertex forms an edge with the subsequent vertex
std::vector<Math::Float3> SutherlandHodgman( Range<BoundingShapes::Plane const *> planes, Range<Math::Float3 const*> vertices )
{
    assert( Size( vertices ) > 0 );

    std::vector<Math::Float3> clipped_vertices( begin( vertices ), end( vertices ) ), temp;

    auto normal_count = Size( planes );
    for( auto i = 0u; i < normal_count && !IsEmpty(clipped_vertices); ++i )
    {
        auto plane = planes[i];
        auto v0 = Last( clipped_vertices );
        auto distance0 = Distance(plane, v0);
        for( auto v1 : clipped_vertices )
        {
            auto distance1 = Distance( plane, v1 );

            if( distance0 < 0 )
            {
                if( distance1 < 0 )
                {
                    temp.push_back( v1 );
                }
                else
                {
                    auto intersect_factor = distance0 / ( distance0 - distance1 );
                    temp.push_back( Lerp( v0, v1, intersect_factor ) );
                }
            }
            else
            {
                if( distance1 < 0 )
                {
                    // Start >= 0, end < 0 so output intersection and end
                    auto intersect_factor = distance0 / ( distance0 - distance1 );
                    temp.push_back( Lerp( v0, v1, intersect_factor ) );
                    temp.push_back( v1 );
                }
            }

            v0 = v1;
            distance0 = distance1;
        }
        swap( clipped_vertices, temp );
        temp.clear();
    }

    return clipped_vertices;
}


void MakePlaneEquations( Math::Float3 extent, uint8_t reference_face_index, std::array<BoundingShapes::Plane, 4> & planes )
{
    auto & aligned_face_normals = BoundingShapes::GetAllAxisAlignedFaceNormals();

    auto skip_index = reference_face_index % 3u;
    auto plane = begin( planes );

    for( uint8_t i = 0; i < 3; i++ )
    {
        if( i != skip_index )
        {
            auto normal = aligned_face_normals[i];
            *plane = BoundingShapes::CreatePlane( normal, extent );
            ++plane;
        }
    }

    // the other two planes are the same, just with inverted normals
    for( auto i = 0u; i < 2; i++ )
    {
        plane[i].normal = -planes[i].normal;
        plane[i].distance = planes[i].distance;
    }
}


void MakePlaneEquations( BoundingShapes::OrientedBox box, uint8_t reference_face_index, std::array<BoundingShapes::Plane, 4> & planes )
{
    auto & aligned_face_normals = BoundingShapes::GetAllAxisAlignedFaceNormals();

    auto skip_index = reference_face_index % 3u;
    auto plane = begin( planes );

    for( uint32_t i = 0; i < 6; i++ )
    {
        if( (i%3u) != skip_index )
        {
            auto normal = Rotate( aligned_face_normals[i], box.rotation );
            auto point = box.extent * aligned_face_normals[i];
            point = Rotate( point, box.rotation );
            point += box.center;
            *plane = BoundingShapes::CreatePlane( normal, point );
            ++plane;
        }
    }
}


void MakePlaneEquations( BoundingShapes::Triangle const & triangle, std::array<BoundingShapes::Plane, 3> & planes )
{
    auto edges = GetEdges( triangle );
    auto triangle_normal = BoundingShapes::GetNormalFromTriangleEdges( edges );
    for( auto i = 0u; i < 3; ++i )
    {
        auto normal = Normalize( Cross( triangle_normal, edges[i] ) );
        planes[i] = BoundingShapes::CreatePlane( normal, triangle.corners[i] );
    }
}


size_t DiscardVertices( BoundingShapes::Plane plane, Range<Math::Float3*> vertices )
{
    auto last = std::remove_if( begin( vertices ), end( vertices ), [&plane]( Math::Float3 vertex )
    {
        return InFront( vertex, plane );
    } );

    return size_t( last - begin( vertices ) );
}


size_t DiscardVertices( BoundingShapes::Plane plane, float const tolerance, Range<Math::Float3*> vertices )
{
    plane.distance -= tolerance;
    return DiscardVertices( plane, vertices );
}


size_t DiscardVertices( BoundingShapes::AxisAlignedBox const & box, uint8_t reference_face_index, Range<Math::Float3*> vertices )
{
    return DiscardVertices( box, reference_face_index, 0, vertices );
}


size_t DiscardVertices( BoundingShapes::AxisAlignedBox const & box, uint8_t reference_face_index, float const tolerance, Range<Math::Float3*> vertices )
{
    using namespace BoundingShapes;
    auto clip_normal = GetAllAxisAlignedFaceNormals()[reference_face_index];
    auto point = box.center + box.extent * clip_normal;
    auto plane = CreatePlane( clip_normal, point );
    return DiscardVertices( plane, tolerance, vertices );
}


size_t DiscardVertices( BoundingShapes::OrientedBox const & box, uint8_t reference_face_index, Range<Math::Float3*> vertices )
{
    return DiscardVertices( box, reference_face_index, 0, vertices );
}


size_t DiscardVertices( BoundingShapes::OrientedBox const & box, uint8_t reference_face_index, float const tolerance, Range<Math::Float3*> vertices )
{
    using namespace BoundingShapes;
    auto clip_normal = GetAllAxisAlignedFaceNormals()[reference_face_index];
    auto extent_position = CopySign(box.extent, clip_normal);
    extent_position = Rotate(extent_position, box.rotation);
    clip_normal = Rotate(clip_normal, box.rotation);
    auto position = box.center + extent_position;
    auto plane = CreatePlane(clip_normal, position);
    return DiscardVertices(plane, tolerance, vertices);
}


// First we determine what edges are involved in teh collision.
// Then we identify the reference edge and incident edge. The reference edge is the edge most perpendicular to the separation normal.
// The reference edge will be used to clip the incident edge vertices to generate the contact manifold.
// Returns the number of contact points found

// sources used: http://www.codezealot.org/archives/394 and http://www.massimpressionsprojects.com/dev/altdevblog/2011/05/13/contact-generation-between-3d-convex-meshes/
uint8_t FindContactPoints( Math::Float3 extent, BoundingShapes::Triangle triangle, Math::Float3 separation_axis, Range<Math::Float3*> output_contact_points )
{
    BoundingShapes::AxisAlignedBox aligned_box = { 0, extent };
    auto & aligned_face_normals = GetAllFaceNormals( aligned_box );
    float max_dot1 = 0;
    uint8_t face_index = 0;
    auto triangle_normal = GetNormal( triangle );
    float max_dot2 = Dot( triangle_normal, separation_axis );
    for( uint8_t i = 0; i < 6; i++ )
    {
        auto dot1 = -Dot( aligned_face_normals[i], separation_axis );
        if( dot1 > max_dot1 )
        {
            max_dot1 = dot1;
            face_index = i;
        }
    }

    auto flip = max_dot1 < max_dot2;
    if( !flip )
    {
        // make the plane equations, with a, b, c as normal and d as distance;
        std::array<BoundingShapes::Plane, 4> planes;
        MakePlaneEquations( extent, face_index, planes );

        std::array<Math::Float3, 4> vertices;
        std::copy( begin( triangle.corners ), end( triangle.corners ), begin( vertices ) );
        // find the longest side of the triangle and add a vertex that sits in the middle of this side
        auto edges = GetEdges( triangle );
        std::array<float, 3> lengths;
        for( auto i : CreateIntegerRange( 3 ) )
        {
            lengths[i] = SquaredNorm( edges[i] );
        }
        auto max_index = uint32_t(std::max_element( begin( lengths ), end( lengths ) ) - begin( lengths ));
        for( auto i = vertices.size() - 2u; i > max_index; --i )
        {
            vertices[i + 1] = vertices[i];
        }
        vertices[max_index + 1] = triangle.corners[max_index] + edges[max_index] * 0.5f;

        auto clipped_vertices = SutherlandHodgman( planes, vertices);
        auto clip_normal = aligned_face_normals[face_index];
        auto plane = BoundingShapes::CreatePlane(clip_normal, CopySign(extent, clip_normal));
        auto size_clipped_vertices = DiscardVertices( plane, clipped_vertices );
        auto contact_point_count = uint8_t( std::min( size_clipped_vertices, Size( output_contact_points ) ) );
        Copy( clipped_vertices.data(), contact_point_count, begin(output_contact_points));
        return contact_point_count;
    }
    else
    {
        std::array<BoundingShapes::Plane, 3> planes;
        MakePlaneEquations( triangle, planes );
        auto vertices = GetFaceCorners( aligned_box, face_index );
        auto clipped_vertices = SutherlandHodgman( planes, vertices );
        auto plane = BoundingShapes::CreatePlane(triangle_normal, GetCenter(triangle));
        auto size_clipped_vertices = DiscardVertices( plane, clipped_vertices );
        auto contact_point_count = uint8_t( std::min( size_clipped_vertices, Size(output_contact_points) ) );
        Copy( clipped_vertices.data(), contact_point_count, begin( output_contact_points ) );
        return contact_point_count;
    }
}