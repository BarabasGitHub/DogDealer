#include "IntersectionTests.h"

#include "BoundingShapeHierarchyMeshFunctions.h"
#include "AxisAlignedBoxFunctions.h"
#include "OrientedBoxFunctions.h"
#include "TriangleFunctions.h"
#include "AxisAlignedBoxSSEFunctions.h"
#include "PlaneFunctions.h"

#include "SATFunctions.h"

#include <Math\FloatOperators.h>
#include <Math\MathFunctions.h>
#include <Math\TransformFunctions.h>

#include <Math\SSE.h>
#include <Math\SSEMathConversions.h>

#include <Utilities\Range.h>
#include <Utilities\IntegerRange.h>

#include <array>

namespace BoundingShapes
{

    // two 2d line segments, AB and CD
    bool Intersect( Math::Float2 a, Math::Float2 b, Math::Float2 c, Math::Float2 d )
    {
        if( Dot( a - b, c - d ) >= 0 )
        {
            std::swap( c, d );
        }

        return IntersectSameOrientation( a, b, d, c );
    }

    // two 2d line segments, AB and CD, assuming that Dot(AB, CD) >= 0
    bool IntersectSameOrientation( Math::Float2 a, Math::Float2 b, Math::Float2 c, Math::Float2 d )
    {
        return Dot( a - c, b - d ) <= 0;
    }


    bool Contains( Sphere const & sphere, Math::Float3 point )
    {
        auto distance = point - sphere.center;
        return SquaredNorm( distance ) < sphere.radius * sphere.radius;
    }


    bool Intersect( Sphere sphere1, Sphere const & sphere2 )
    {
        sphere1.radius += sphere2.radius;
        return Contains( sphere1, sphere2.center );
    }

    namespace
    {
        using namespace Math::SSE;

        // assumes non-zero extent in first three components, last component should be zero.
        int VECTOR_CALL AxisAlignedBoxContainsPointSSE( Float32Vector center, Float32Vector extent, Float32Vector point )
        {
            auto distance = AbsoluteDifference(center, point);
            auto outside = LessThan( extent, distance );
            // assuming the last element is always false
            return TestAllZero( CastToIntegerFromFloat( outside ) );
        }


        // assumes non-zero extent in first three components, last component should be zero.
        Float32Vector VECTOR_CALL SquaredDistanceAxisAlignedBoxPointSSE(Float32Vector center, Float32Vector extent, Float32Vector point)
        {
            auto relative = AbsoluteDifference(point, center);
            auto a = Max(Subtract(relative, extent), ZeroFloat32Vector());
            // or
            // auto relative = Subtract(point, center);
            // auto signed_extent = CopySign(extent, relative);
            // auto a = Max(Subtract(relative, signed_extent), ZeroFloat32Vector()));
            auto squared_distance = Dot3(a, a);
            return squared_distance;
        }
    }

    bool Contains( AxisAlignedBox const & box, Math::Float3 point )
    {
        auto center = SSEFromFloat3( box.center );
        auto extent = SSEFromFloat3( box.extent );
        auto sse_point = SSEFromFloat3( point );
        return !!AxisAlignedBoxContainsPointSSE( center, extent, sse_point );
        //auto distance = Abs( box.center - point );

        //for( auto i : CreateIntegerRange( 3 ) )
        //{
        //    if( distance[i] > box.extent[i] ) return false;
        //}
        //return true;
    }


    bool Intersect( AxisAlignedBox const & box1, AxisAlignedBox const & box2 )
    {
        auto center1 = SSEFromFloat3( box1.center );
        auto center2 = SSEFromFloat3( box2.center );
        auto extent = Add( SSEFromFloat3( box1.extent ), SSEFromFloat3( box2.extent ) );
        return !!AxisAlignedBoxContainsPointSSE( center1, extent, center2 );
    }


    bool Contains( OrientedBox const & box, Math::Float3 point )
    {
        auto sse_point = SSEFromFloat3( point );
        sse_point = QuaternionRotate3DVector( sse_point, QuaternionConjugate( SSEFromQuaternion( box.rotation ) ) );
        auto center = SSEFromFloat3( box.center );
        auto extent = SSEFromFloat3( box.extent );
        return !!AxisAlignedBoxContainsPointSSE( center, extent, sse_point );
    }


    bool Intersect( AxisAlignedBox box, Sphere const & sphere )
    {
        auto box_center = SSEFromFloat3(box.center);
        auto box_extent = SSEFromFloat3(box.extent);
        auto sphere_center = SSEFromFloat3(sphere.center);
        auto distance = SquaredDistanceAxisAlignedBoxPointSSE(box_center, box_extent, sphere_center);
        return sphere.radius * sphere.radius > GetSingle(distance);
    }


    bool Intersect( OrientedBox box, Sphere const & sphere )
    {
        auto sphere_center = SSEFromFloat3(sphere.center);
        sphere_center = QuaternionRotate3DVector(sphere_center, QuaternionConjugate(SSEFromQuaternion(box.rotation)));
        auto box_center = SSEFromFloat3(box.center);
        auto box_extent = SSEFromFloat3(box.extent);
        auto distance = SquaredDistanceAxisAlignedBoxPointSSE(box_center, box_extent, sphere_center);
        return sphere.radius * sphere.radius > GetSingle(distance);
    }


    bool Intersect( OrientedBox const & oriented_box, AxisAlignedBox const & axis_aligned_box )
    {
        // make face axes
        std::array< Math::Float3, 15 > axes = { { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } } };
        auto axis_aligned_axes = CreateRange( axes.data(), 3 );
        uint8_t axes_filled = 3;
        auto oriented_axes = CreateRange( axes.data() + axes_filled, 3 );
        SAT::GetAxes( oriented_box, oriented_axes );
        axes_filled += 3;

        // make edge axes
        {
            // for a box the face axis are the same as the directions of the edges
            for( auto const & oriented_edge : oriented_axes )
            {
                for( auto const & axis_aligned_edge : axis_aligned_axes )
                {
                    axes[axes_filled] = Cross( oriented_edge, axis_aligned_edge );
                    ++axes_filled;
                }
            }
        }

        auto const axis_aligned_corners = GetCorners( axis_aligned_box );
        auto const oriented_corners = GetCorners( oriented_box );

        for( auto const & axis : CreateRange(axes.data(), axes_filled) )
        {
            auto minmax1 = SAT::ProjectToAxis( oriented_corners, axis );
            auto minmax2 = SAT::ProjectToAxis( axis_aligned_corners, axis );

            auto overlap = SAT::CalculateOverlap( minmax1, minmax2 );
            if( overlap < 0 ) return false;
        }
        return true;
    }


    bool Intersect( OrientedBox const & box1, OrientedBox const & box2 )
    {
        return Intersect( Rotate( box1, Conjugate( box2.rotation ) ), AxisAlignedBox{ box2.center, box2.extent } );
    }


    bool Intersect( Triangle triangle, Sphere const & sphere )
    {
        // first move the origin to the center of the sphere
        triangle = Translate( triangle, -sphere.center );

        // check distance to corners for early out (not sure if it helps)
        for( auto const & corner : triangle.corners )
        {
            if( Dot( corner, corner) < sphere.radius * sphere.radius ) return true;
        }

        auto const edges = GetEdges( triangle );

        // check the distance perpendicular to the triangle face
        auto const face_normal = Normalize( Cross( edges[0], edges[1] ) );
        if( Math::Abs( Dot( face_normal, triangle.corners[0] ) ) > sphere.radius ) return false;

        // check overlap for each edge
        MinMax<float> const sphere_minmax = { -sphere.radius, sphere.radius };
        for( auto const & edge : edges )
        {
            auto const minmax = SAT::ProjectToAxis( triangle.corners, Normalize(edge) );

            auto const overlap = SAT::CalculateOverlap( sphere_minmax, minmax );
            if( overlap < 0 ) return false;
        }

        return true;
    }


    namespace
    {
        /// intesection between triangle and a axis aligned box with the center at 0
        bool IntersectExtent( Triangle const & triangle, Math::Float3 extent )
        {
            // make face axes
            std::array< Math::Float3, 15 > axes = { { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } } };
            auto axis_aligned_axes = CreateRange( axes.data( ), 3 );
            uint8_t axes_filled = 3;
            auto const triangle_edges = GetEdges( triangle );
            axes[axes_filled] = BoundingShapes::GetNormalFromTriangleEdges( triangle_edges );
            ++axes_filled;

            // make edge axes
            for( auto const & triangle_axis : triangle_edges )
            {
                // for a box the face axis are the same as the directions of the edges
                for( auto const & axis_aligned_edge : axis_aligned_axes )
                {
                    axes[axes_filled] = Cross( triangle_axis, axis_aligned_edge );
                    ++axes_filled;
                }
            }

            auto const box_corners = GetCorners( AxisAlignedBox{ 0, extent } );

            for( auto const & axis : CreateRange( axes.data( ), axes_filled ) )
            {
                auto minmax1 = SAT::ProjectToAxis( box_corners, axis );
                auto minmax2 = SAT::ProjectToAxis( triangle.corners, axis );

                auto overlap = SAT::CalculateOverlap( minmax1, minmax2 );
                if( overlap < 0 ) return false;
            }
            return true;
        }
    }


    bool Intersect( Triangle const & triangle, AxisAlignedBox const & box )
    {
        auto translated_triangle = Translate( triangle, -box.center );
        return IntersectExtent( translated_triangle, box.extent );
    }


    bool Intersect( Triangle const & triangle, OrientedBox const & box )
    {
        auto const transformed_triangle = TransformByOrientation( triangle, { -box.center, Conjugate(box.rotation) } );
        return IntersectExtent( transformed_triangle, box.extent );
    }


    namespace
    {
        template<typename ShapeType>
        bool IntersectImpl( BoundingShapeHierarchyMesh<ShapeType> const & mesh, OrientedBox const & box )
        {
            bool intesect = false;
            Traverse(mesh,
                [&](ShapeType const & shape)
                {
                    return Intersect(box, shape);
                },
                [&](auto const & indices)
                {
                    auto const & positions = mesh.vertex_positions;
                    auto const triangle = CreateTriangle( positions[indices[0]], positions[indices[1]], positions[indices[2]] );
                    intesect = Intersect( triangle, box );
                    return !intesect;
                });
            return intesect;
        }
    }


    bool Intersect( SphereHierarchyMesh const & mesh, OrientedBox const & box )
    {
        return IntersectImpl( mesh, box );
    }


    bool Intersect( AxisAlignedBoxHierarchyMesh const & mesh, OrientedBox const & box )
    {
        return IntersectImpl( mesh, box );
    }


    bool InFront( AxisAlignedBox const & box, Plane plane )
    {
        auto signed_extent = CopySign( box.extent, plane.normal );
        auto point = signed_extent + box.center;
        return InFront(point, plane);
    }


    bool InFront( Math::Float3 point, Plane plane )
    {
        auto distance = Distance( plane, point );
        return distance >= 0;
    }


    namespace
    {
        bool VECTOR_CALL TestPlanePointSSE( Math::SSE::Float32Vector point, Math::SSE::Float32Vector plane_equation )
        {
            auto dot = Dot3( point, plane_equation );
            return GetSingle(Swizzle<3>(plane_equation)) + GetSingle(dot) >= 0;
        }


        bool VECTOR_CALL TestPlaneAxisAlignedBoxSSE( Math::SSE::Float32Vector center, Math::SSE::Float32Vector extent, Math::SSE::Float32Vector plane_equation )
        {
            using namespace Math::SSE;
            auto signed_extent = CopySign( extent, plane_equation );
            auto point = Add( signed_extent, center );
            return TestPlanePointSSE(point, plane_equation);
        }

        bool VECTOR_CALL InstersectFrustumAxisAlignedBoxSSE( Math::SSE::Float32Vector center, Math::SSE::Float32Vector extent, Math::SSE::FloatMatrix const & projection_matrix )
        {
            using namespace Math::SSE;

            // near plane
            auto plane_equation = projection_matrix.row[2];
            if( !TestPlaneAxisAlignedBoxSSE( center, extent, plane_equation ) )
            {
                return false;
            }

            // left and bottom plane
            for( auto i = 0u; i < 2; i++ )
            {
                plane_equation = Add( projection_matrix.row[3], projection_matrix.row[i] );
                if( !TestPlaneAxisAlignedBoxSSE( center, extent, plane_equation ) )
                {
                    return false;
                }
            }

            // right, top and far plane
            for( auto i = 0u; i < 3; i++ )
            {
                plane_equation = Subtract( projection_matrix.row[3], projection_matrix.row[i] );
                if( !TestPlaneAxisAlignedBoxSSE( center, extent, plane_equation ) )
                {
                    return false;
                }
            }

            return true;
        }
    }


    bool Intersect( AxisAlignedBox const & box, Ray const & ray)
    {
    //     auto offset = box.center - ray.start;
    //     // scale with extent to pretend the box is cubical
    //     auto scaled_offset = offset / box.extent;
    //     auto scaled_dir = ray.direction / box.extent;
    //     auto scaled_closest_point = ComponentParallelToAxis(scaled_offset, scaled_dir);
    //     auto closest_point = scaled_closest_point * box.extent + ray.start;
    //     return Contains(box, closest_point);

        using namespace Math::SSE;
        auto box_center = SSEFromFloat3( box.center );
        auto box_extent = SSEFromFloat3( box.extent );
        auto box_minmax = GetMinMax(box_center, box_extent);
        auto ray_start = SSEFromFloat3(ray.start);
        auto ray_direction = SSEFromFloat3(ray.direction);
        auto rcp_ray_direction = Blend<0,0,0,1>(Reciproce(ray_direction), ZeroFloat32Vector());
        auto t0 = Multiply(Subtract(box_minmax.min, ray_start), rcp_ray_direction);
        auto t1 = Multiply(Subtract(box_minmax.max, ray_start), rcp_ray_direction);
        auto tmin = Min(t0, t1);
        auto tmax = Max(t0, t1);

        // reduce the three min & max-es to one common min & max
        // tmin = MaxSingle(Swizzle<2>(tmin), tmin);
        // tmin = MaxSingle(Swizzle<1>(tmin), tmin);

        // tmax = MinSingle(Swizzle<2>(tmax), tmax);
        // tmax = MinSingle(Swizzle<1>(tmax), tmax);

        // return GetSingle(tmax) >= GetSingle(tmin);
        auto is_negative0 = ~MaskSignBits(tmax);
        auto is_negative1 = MaskSignBits(Subtract(tmin, tmax));
        auto is_negative2 = MaskSignBits(Subtract(tmin, Swizzle<1,2,0>(tmax)));
        auto is_negative3 = MaskSignBits(Subtract(tmin, Swizzle<2,0,1>(tmax)));
        auto r = is_negative0 & is_negative1 & is_negative2 & is_negative3;
        return r == 0x7;
    }


    // does an intersection test for a box and a frustum defined by a projection matrix
    // http://fgiesen.wordpress.com/2012/08/31/frustum-planes-from-the-projection-matrix/
    bool IntersectFrustum( AxisAlignedBox const & box, Math::Float4x4 const projection_matrix )
    {
        // near plane
        auto plane_equation = projection_matrix[2];
        if( !InFront( box, CreatePlane( plane_equation ) )  )
        {
            return false;
        }

        // left and bottom plane
        for( auto i = 0u; i < 2; i++ )
        {
            plane_equation = projection_matrix[3] + projection_matrix[i];
            if( !InFront( box, CreatePlane( plane_equation ) ) )
            {
                return false;
            }
        }

        // right, top and far plane
        for( auto i = 0u; i < 3; i++ )
        {
            plane_equation = projection_matrix[3] - projection_matrix[i];
            if( !InFront( box, CreatePlane( plane_equation ) ) )
            {
                return false;
            }
        }

        return true;
    }

    // returns the indices of the intersecting boxes
    void IntersectFrustum( Range<AxisAlignedBox const *> boxes, Math::Float4x4 const & projection_matrix, std::vector<uint32_t> & intersecting_indices)
    {
        using namespace Math::SSE;
        auto sse_projection_matrix = SSEFromFloat4x4( projection_matrix );
        auto size = Size( boxes );
        for( auto i = 0u; i < size; ++i )
        {
            auto box_center = SSEFromFloat3( boxes[i].center );
            auto box_extent = SSEFromFloat3( boxes[i].extent );
            if( InstersectFrustumAxisAlignedBoxSSE( box_center, box_extent, sse_projection_matrix ) )
            {
                intersecting_indices.push_back( i );
            }
        }
    }


    void IntersectFrustum( Range<AxisAlignedBox const *> boxes, Range<Math::Float4x4 const*> transforms, Math::Float4x4 const & projection_matrix, std::vector<uint32_t> & intersecting_indices)
    {
        using namespace Math::SSE;
        assert( Size( boxes ) == Size( transforms ) );
        auto sse_projection_matrix = SSEFromFloat4x4(projection_matrix);
        auto size = Size( boxes );
        for( auto i = 0u; i < size; ++i )
        {
            auto transformed_box_center = SSEFromFloat3( boxes[i].center );
            auto transformed_box_extent = SSEFromFloat3( boxes[i].extent );
            auto transformation_matrix = SSEFromFloat4x4( transforms[i] );
            TransformAxisAlignedBoxSSE( transformed_box_center, transformed_box_extent, transformation_matrix );
            if( InstersectFrustumAxisAlignedBoxSSE( transformed_box_center, transformed_box_extent, sse_projection_matrix ) )
            {
                intersecting_indices.push_back( i );
            }
        }
    }


    bool Intersect(AxisAlignedBox const & box, Range<Plane const*> planes)
    {
        using namespace Math;
        using namespace SSE;
        auto center = SSEFromFloat3(box.center);
        auto extent = SSEFromFloat3(box.extent);
        auto plane_count = Size(planes);
        auto plane_data = begin(planes);
        auto p = 0;
        //while( p < plane_count && TestPlaneAxisAlignedBoxSSE(center, extent, SSEFromFloat4(Float4(plane_data[p].normal, plane_data[p].distance))))
        while( p < plane_count && TestPlaneAxisAlignedBoxSSE( center, extent, SSEFromFloat4( reinterpret_cast<Float4 const &>( plane_data[p] ) ) ) )
        {
            ++p;
        }
        return p == plane_count;
    }


    void Intersect( Range<AxisAlignedBox const *> boxes, Range<Plane const*> planes, std::vector<uint32_t> & contained_indices)
    {
        auto box_count = Size(boxes);
        auto box_data = begin( boxes );
        for( auto i = 0u; i < box_count; ++i )
        {
            if( Intersect(box_data[i], planes) )
            {
                contained_indices.push_back(i);
            }
        }
    }
}
