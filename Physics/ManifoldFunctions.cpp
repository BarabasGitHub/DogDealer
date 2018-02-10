#include "ManifoldFunctions.h"

#include "ContactFunctions.h"

#include <BoundingShapes\BoundingShapeHierarchyMeshFunctions.h>
#include <BoundingShapes\SATFunctions.h>
#include <BoundingShapes\AxisAlignedBoxFunctions.h>
#include <BoundingShapes\OrientedBoxFunctions.h>
#include <BoundingShapes\TriangleFunctions.h>
#include <BoundingShapes\IntersectionTests.h>
#include <BoundingShapes\PlaneFunctions.h>


#include <Math\FloatOperators.h>
#include <Math\MathFunctions.h>
#include <Math\PointFunctions.h>
#include <Math\TransformFunctions.h>

#include <Utilities\Range.h>
#include <Utilities\Memory.h>
#include <Utilities\MinMax.h>

#include <array>

namespace Physics
{

    Manifold CreateManifold( BoundingShapes::Sphere sphere1, BoundingShapes::Sphere sphere2 )
    {
        auto combined_radius = sphere1.radius + sphere2.radius;
        auto relative_center = sphere1.center - sphere2.center;
        auto squared_center_distance = SquaredNorm(relative_center);
        Manifold manifold;
        if(squared_center_distance < combined_radius * combined_radius)
        {
            auto center_distance = Math::Sqrt(squared_center_distance);
            auto separation_axis = relative_center / center_distance;

            manifold.contact_point_count = 1;
            manifold.separation_axes[0] = separation_axis;
            manifold.positions[0] = sphere1.center - separation_axis * sphere1.radius;
            manifold.penetration_depths[0] = combined_radius - center_distance;
        }
        return manifold;
    }


    namespace
    {
        Manifold CreateManifoldFromExtent( BoundingShapes::Sphere sphere, Math::Float3 extent )
        {
            auto closest_point = Clamp(-extent, extent, sphere.center);
            Manifold manifold;
            auto distance_vector = sphere.center - closest_point;
            auto squared_distance = SquaredNorm(distance_vector);
            if(squared_distance < sphere.radius * sphere.radius)
            {
                auto absolute_position = Abs(sphere.center);
                Math::Float3 separation_axis;
                // if centre is inside the box
                if( (extent.x >= absolute_position.x) &&
                    (extent.y >= absolute_position.y) &&
                    (extent.z >= absolute_position.z) )
                {
                    // get the axis that points to the closest point on the surface of the box from the centre of the sphere
                    auto smallest_axis = GetMinElementIndex(extent - absolute_position);
                    separation_axis = 0;
                    separation_axis[smallest_axis] = 1;
                    separation_axis = CopySign(separation_axis, closest_point);
                }
                else
                {
                    // separation_axis = Normalize(distance_vector);
                    auto distance = Math::Sqrt(squared_distance);
                    separation_axis = distance_vector / distance;
                }
                manifold.contact_point_count = 1;
                manifold.separation_axes[0] = separation_axis;
                manifold.positions[0] = closest_point;
                manifold.penetration_depths[0] = sphere.radius - Dot(separation_axis, distance_vector);
            }
            return manifold;
        }
    }


    Manifold CreateManifold( BoundingShapes::Sphere sphere, BoundingShapes::AxisAlignedBox const & box )
    {
        sphere.center -= box.center;
        auto manifold = CreateManifoldFromExtent( sphere, box.extent );
        for( auto i = 0u; i < manifold.contact_point_count; ++i )
        {
            manifold.positions[i] += box.center;
        }
        return manifold;
    }


    Manifold CreateManifold( BoundingShapes::Sphere sphere, BoundingShapes::OrientedBox const & box )
    {
        sphere.center -= box.center;
        sphere.center = InverseRotate(sphere.center, box.rotation);
        auto manifold = CreateManifoldFromExtent( sphere, box.extent );
        for( auto i = 0u; i < manifold.contact_point_count; ++i )
        {
            manifold.separation_axes[i] = Rotate( manifold.separation_axes[i], box.rotation );
        }
        for( auto i = 0u; i < manifold.contact_point_count; ++i )
        {
            manifold.positions[i] = Rotate( manifold.positions[i], box.rotation ) + box.center;
        }
        return manifold;
    }


    Manifold CreateManifold( BoundingShapes::Sphere sphere, DensityFunctionType const & sample_function )
    {
        // Sample density and gradient for the center
        float center_density;
        Math::Float3 center_gradient;
        sample_function( sphere.center, center_density, center_gradient );
        // sample at the edge of the sphere in the opposite direction of the center gradient
        float edge_density;
        Math::Float3 edge_gradient;
        Math::Float3 edge_position = sphere.center + Normalize(center_gradient) * sphere.radius;
        sample_function( edge_position, edge_density, edge_gradient );
        Manifold manifold;
        if(edge_density > 0)
        {
            manifold.contact_point_count = 1;
            manifold.separation_axes[0] = -Normalize(edge_gradient);
            manifold.positions[0] = edge_position;
            manifold.penetration_depths[0] = edge_density;
        }
        return manifold;
    }


    Manifold CreateManifold( BoundingShapes::AxisAlignedBox box1, BoundingShapes::AxisAlignedBox box2 )
    {
        assert(!"This function probably doesn't work right, please check before using it.");
        auto const center_diff = box1.center - box2.center;

        auto const combined_extents = box1.extent + box2.extent;

        auto const overlap = combined_extents - Abs( center_diff );

        auto const min_index = Math::GetMinElementIndex( overlap );

        Manifold manifold;
        manifold.separation_axes[0] = 0;
        manifold.separation_axes[0][min_index] = 1;
        manifold.penetration_depths[0] = overlap[min_index];
        manifold.positions[0] = box1.center - Math::CopySign( box1.extent - overlap / 2, center_diff );
        manifold.contact_point_count = 1;
        return manifold;
    }


    namespace
    {

        struct Line
        {
            Math::Float3 start, direction;
        };

        // source http://geomalgorithms.com/a07-_distance.html
        // assuming the lines aren't degenerate or parallel
        void ClosestPoints( Line line1, Line line2, Math::Float3 & point1, Math::Float3 & point2 )
        {
            auto u = line1.direction;
            auto v = line2.direction;
            auto w = line1.start - line2.start;
            auto a = SquaredNorm(u); // always >= 0
            assert(a > 0 && "Line1 is degenerate.");
            auto b = Dot(u,v);
            auto c = SquaredNorm(v); // always >= 0
            assert(c > 0 && "Line2 is degenerate.");
            auto d = Dot(u,w);
            auto e = Dot(v,w);
            auto f = a*c - b*b; // always >= 0
            assert(f > 0 && "Line1 and line2 are parallel.");

            // compute the line parameters of the two closest points
            auto s = ( b * e - c * d ) / f;
            auto t = ( a * e - b * d ) / f;

            point1 = line1.start + line1.direction * s;
            point2 = line2.start + line2.direction * t;
        }


        uint8_t GetReferenceFaceIndex( Math::Float3 axis )
        {
            auto reference_face_index = GetMaxElementIndex( Abs( axis ) );
            if( axis[reference_face_index] < 0 )
            {
                reference_face_index += 3;
            }
            return reference_face_index;
        };


        Manifold CreateManifoldFromExtent( Math::Float3 const extent, BoundingShapes::OrientedBox box )
        {
            float const zero_axis_tolerance = 1e-3f;
            float const angle_tolerance = 1e-3f;
            // float const vertex_discard_tolerance = 1e-3f;
            // make face axes
            std::array< Math::Float3, 3 > axis_aligned_axes;
            SAT::GetAxisAlignedBoxAxes( axis_aligned_axes );

            BoundingShapes::AxisAlignedBox aligned_box = { 0, extent };
            auto const corners1 = GetCorners( aligned_box );
            auto const corners2 = GetCorners( box );

            Manifold manifold;
            float min_depth_extent;
            uint32_t min_depth_axis_extent;

            // test faces of extent
            if( !SAT::CalculateOverlap( axis_aligned_axes, corners1, corners2, min_depth_extent, min_depth_axis_extent ) )
            {
                manifold.contact_point_count = 0;
                return manifold;
            }

            std::array< Math::Float3, 3 > box_axes;
            SAT::GetAxes( box, box_axes );
            float min_depth_box;
            uint32_t min_depth_axis_box;
            // test faces of box
            if( !SAT::CalculateOverlap( box_axes, corners1, corners2, min_depth_box, min_depth_axis_box ) )
            {
                manifold.contact_point_count = 0;
                return manifold;
            }

            float min_depth_edges;
            uint32_t min_depth_edge1, min_depth_edge2;
            Math::Float3 separation_axis_edges;
            // only correct for small angles, but that's ok, because we're checking if it is small
            auto rotation_axis_norm_squared = SquaredNorm( GetAxis( box.rotation ) );
            auto scaled_zero_axis_tolerance = (zero_axis_tolerance * zero_axis_tolerance) * rotation_axis_norm_squared;
            // scale the tolerance instead of dividing the norm
            // angle is approximated by: Norm(GetAxis(box.rotation))/box.rotation.w
            // then squared to avoid the square root and the w is then brought to the other side of the inequality test: approx_angle > tolerance
            auto scaled_angle_tolerance = (angle_tolerance * angle_tolerance) * (box.rotation.w * box.rotation.w);
            if( ( Math::Abs( box.rotation.x * box.rotation.x ) > scaled_zero_axis_tolerance ) &&
                ( Math::Abs( box.rotation.y * box.rotation.y ) > scaled_zero_axis_tolerance ) &&
                ( Math::Abs( box.rotation.z * box.rotation.z ) > scaled_zero_axis_tolerance ) &&
                ( rotation_axis_norm_squared > scaled_angle_tolerance ) )
            {
                // test edge-edge
                if( !SAT::CalculateOverlap( axis_aligned_axes, box_axes, corners1, corners2, zero_axis_tolerance, min_depth_edges, min_depth_edge1, min_depth_edge2, separation_axis_edges ) )
                {
                    manifold.contact_point_count = 0;
                    return manifold;
                }
            }
            else
            {
                min_depth_edges = std::numeric_limits<float>::max();
                separation_axis_edges = 0;
                min_depth_edge1 = min_depth_edge2 = 0;
            }


            float min_depth;
            Math::Float3 separation_axis;
            if( min_depth_extent <= min_depth_box && min_depth_extent <= min_depth_edges )
            {
                min_depth = min_depth_extent;
                separation_axis = axis_aligned_axes[min_depth_axis_extent];
                auto box_is_in_direction_of_separation_axis = Dot( separation_axis, box.center ) > 0;
                if( box_is_in_direction_of_separation_axis )
                {
                    separation_axis = -separation_axis;
                }
                // axis belongs to extent, and thus the extent is incident
                auto incident_face_index = uint8_t(box_is_in_direction_of_separation_axis ? min_depth_axis_extent : min_depth_axis_extent + 3);
                // get the reference face of the box
                auto reference_face_index = GetReferenceFaceIndex(InverseRotate(separation_axis, box.rotation));
                auto vertices = BoundingShapes::GetFaceCorners( BoundingShapes::AxisAlignedBox{ 0, extent }, incident_face_index );
                std::array<BoundingShapes::Plane, 4> planes;
                MakePlaneEquations( box, reference_face_index, planes );
                auto clipped_vertices = SutherlandHodgman( planes, vertices );
                clipped_vertices.resize( DiscardVertices( box, reference_face_index, clipped_vertices ) );
                std::vector<float> penetration_depths(clipped_vertices.size(), min_depth);
                std::vector<uint8_t> ages(clipped_vertices.size(), 0);
                std::vector<Math::Float3> separation_axes(clipped_vertices.size(), separation_axis);
                manifold = CreateManifold(penetration_depths, separation_axes, clipped_vertices, ages);
            }
            else if( min_depth_box <= min_depth_edges )
            {
                min_depth = min_depth_box;
                separation_axis = box_axes[min_depth_axis_box];
                auto box_is_in_direction_of_separation_axis = Dot( separation_axis, box.center ) > 0;
                if( box_is_in_direction_of_separation_axis )
                {
                    separation_axis = -separation_axis;
                }
                // axis belongs to box, and thus the box is incident
                auto incident_face_index = uint8_t(box_is_in_direction_of_separation_axis ? min_depth_axis_box + 3: min_depth_axis_box);
                // get the reference face of the extent
                auto reference_face_index = GetReferenceFaceIndex(-separation_axis);
                auto vertices = GetFaceCorners( box, incident_face_index );
                std::array<BoundingShapes::Plane, 4> planes;
                MakePlaneEquations( extent, reference_face_index, planes );
                auto clipped_vertices = SutherlandHodgman( planes, vertices );
                clipped_vertices.resize(DiscardVertices( BoundingShapes::AxisAlignedBox{ 0, extent }, reference_face_index, clipped_vertices ));
                std::vector<float> penetration_depths(clipped_vertices.size(), min_depth);
                std::vector<uint8_t> ages(clipped_vertices.size(), 0);
                std::vector<Math::Float3> separation_axes(clipped_vertices.size(), separation_axis);
                manifold = CreateManifold(penetration_depths, separation_axes, clipped_vertices, ages);
            }
            else
            {
                min_depth = min_depth_edges;
                separation_axis = Normalize(separation_axis_edges);
                auto box_is_in_direction_of_separation_axis = Dot( separation_axis, box.center ) > 0;
                if( box_is_in_direction_of_separation_axis )
                {
                    separation_axis = -separation_axis;
                }

                // axis belongs to two edges
                auto extent_edge_index = min_depth_edge1;
                auto extent_vertex = CopySign( extent, -separation_axis );
                auto extent_direction= CopySign(axis_aligned_axes[extent_edge_index], separation_axis);

                auto box_edge_index = min_depth_edge2;
                auto box_local_separation_axis = InverseRotate( separation_axis, box.rotation );
                auto signed_box_extent = CopySign( box.extent, box_local_separation_axis );
                auto box_vertex = Rotate(signed_box_extent, box.rotation) + box.center;
                auto signed_box_local_axis = CopySign( axis_aligned_axes[box_edge_index], box_local_separation_axis );
                auto box_direction = Rotate(signed_box_local_axis, box.rotation);

                ClosestPoints( {extent_vertex, extent_direction}, {box_vertex, box_direction}, manifold.positions[0], manifold.positions[1]);
                manifold.contact_point_count = 2;
                for( auto i = 0u; i < manifold.contact_point_count; i++ )
                {
                    manifold.penetration_depths[i] = min_depth;
                }
                for( auto i = 0u; i < manifold.contact_point_count; i++ )
                {
                    manifold.separation_axes[i] = separation_axis;
                }
            }

            return manifold;
        }
    }


    Manifold CreateManifold( BoundingShapes::OrientedBox box1, BoundingShapes::OrientedBox box2 )
    {
        box2.center -= box1.center;
        box2 = Rotate( box2, Conjugate(box1.rotation) );
        auto manifold = CreateManifoldFromExtent( box1.extent, box2 );
        for( auto i = 0u; i < manifold.contact_point_count; ++i )
        {
            manifold.separation_axes[i] = Rotate( manifold.separation_axes[i], box1.rotation );
        }
        for( auto i = 0u; i < manifold.contact_point_count; ++i )
        {
            manifold.positions[i] = Rotate( manifold.positions[i], box1.rotation ) + box1.center;
        }
        return manifold;
    }


    namespace
    {
        // intesection between triangle and a axis aligned box with the center at 0
        Manifold CreateManifoldFromExtent( Math::Float3 extent, BoundingShapes::Triangle const & triangle )
        {
            // make face axes
            std::array< Math::Float3, 3 > axis_aligned_axes;
            SAT::GetAxisAlignedBoxAxes( axis_aligned_axes );

            BoundingShapes::AxisAlignedBox aligned_box = { 0, extent };
            auto const corners1 = GetCorners( aligned_box );

            Manifold manifold;
            float min_depth_extent;
            uint32_t min_depth_axis_extent;

            // test faces of extent
            if( !SAT::CalculateOverlap( axis_aligned_axes, corners1, triangle.corners, min_depth_extent, min_depth_axis_extent ) )
            {
                manifold.contact_point_count = 0;
                return manifold;
            }

            float min_depth_triangle;
            uint32_t min_depth_axis_triangle;
            std::array<Math::Float3, 1> triangle_axes;
            triangle_axes[0] = GetNormal( triangle );

            // test faces of triangle
            if( !SAT::CalculateOverlap( triangle_axes, corners1, triangle.corners, min_depth_triangle, min_depth_axis_triangle ) )
            {
                manifold.contact_point_count = 0;
                return manifold;
            }

            float min_depth_edges;
            uint32_t min_depth_edge1, min_depth_edge2;
            Math::Float3 separation_axis_edges;
            auto triangle_edges = GetEdges( triangle );
            // test edge-edge
            if( !SAT::CalculateOverlap( axis_aligned_axes, triangle_edges, corners1, triangle.corners, 1e-3f, min_depth_edges, min_depth_edge1, min_depth_edge2, separation_axis_edges ) )
            {
                manifold.contact_point_count = 0;
                return manifold;
            }

            auto const triangle_center = GetCenter( triangle );

            float min_depth;
            Math::Float3 separation_axis;

            // use old approach
            if( min_depth_extent <= min_depth_triangle && min_depth_extent <= min_depth_edges )
            {
                min_depth = min_depth_extent;
                separation_axis = axis_aligned_axes[min_depth_axis_extent];
                auto triangle_is_in_direction_of_separation_axis = Dot( separation_axis, triangle_center ) > 0;
                if( triangle_is_in_direction_of_separation_axis )
                {
                    separation_axis = -separation_axis;
                }
            }
            else if( min_depth_triangle <= min_depth_edges )
            {
                min_depth = min_depth_triangle;
                separation_axis = triangle_axes[0];
                auto triangle_is_in_direction_of_separation_axis = Dot( separation_axis, triangle_center ) > 0;
                if( triangle_is_in_direction_of_separation_axis )
                {
                    separation_axis = -separation_axis;
                }
            }
            else
            {
                min_depth = min_depth_edges;
                separation_axis = separation_axis_edges;
                auto triangle_is_in_direction_of_separation_axis = Dot( separation_axis, triangle_center ) > 0;
                if( triangle_is_in_direction_of_separation_axis )
                {
                    separation_axis = -separation_axis;
                }
            }

            manifold.contact_point_count = FindContactPoints( extent, triangle, separation_axis, manifold.positions );

            //if( min_depth_extent <= min_depth_triangle && min_depth_extent <= min_depth_edges )
            //{
            //    min_depth = min_depth_extent;
            //    separation_axis = axis_aligned_axes[min_depth_axis_extent];
            //    auto triangle_is_in_direction_of_separation_axis = Dot( separation_axis, triangle_center ) > 0;
            //    if( triangle_is_in_direction_of_separation_axis )
            //    {
            //        separation_axis = -separation_axis;
            //    }

            //    // axis belongs to extent, and thus the extent is incident
            //    auto incident_face_index = uint8_t( triangle_is_in_direction_of_separation_axis ? min_depth_axis_extent : min_depth_axis_extent + 3 );
            //    // get the reference face of the triangle
            //    auto vertices = BoundingShapes::GetFaceCorners( BoundingShapes::AxisAlignedBox{ 0, extent }, incident_face_index );
            //    std::array<BoundingShapes::Plane, 3> planes;
            //    MakePlaneEquations( triangle, planes );
            //    auto clipped_vertices = SutherlandHodgman( planes, vertices );
            //    auto clip_normal = triangle_axes[0];
            //    auto clip_distance = Dot( clip_normal, triangle_center );
            //    manifold.contact_point_count = uint8_t( DiscardVertices( clip_normal, clip_distance, clipped_vertices ) );
            //    manifold.contact_point_count = std::min( manifold.contact_point_count, Manifold::c_max_contact_points );
            //    Copy( clipped_vertices.data(), manifold.contact_point_count, manifold.positions.data() );
            //}
            //else if( min_depth_triangle <= min_depth_edges )
            //{
            //    min_depth = min_depth_triangle;
            //    separation_axis = triangle_axes[0];
            //    auto triangle_is_in_direction_of_separation_axis = Dot( separation_axis, triangle_center ) > 0;
            //    if( triangle_is_in_direction_of_separation_axis )
            //    {
            //        separation_axis = -separation_axis;
            //    }

            //    // axis belongs to the triangle, and thus the triangle is incident
            //    // get the reference face of the extent
            //    auto reference_face_index = GetReferenceFaceIndex( separation_axis );
            //    std::array<BoundingShapes::Plane, 4> planes;
            //    MakePlaneEquations( extent, reference_face_index, planes );
            //    auto clipped_vertices = SutherlandHodgman( planes, triangle.corners );
            //    manifold.contact_point_count = uint8_t( DiscardVertices( BoundingShapes::AxisAlignedBox{ 0, extent }, reference_face_index, clipped_vertices ) );
            //    manifold.contact_point_count = std::min( manifold.contact_point_count, Manifold::c_max_contact_points );
            //    Copy( clipped_vertices.data(), manifold.contact_point_count, manifold.positions.data() );
            //}
            //else
            //{
            //    min_depth = min_depth_edges;
            //    separation_axis = separation_axis_edges;
            //    auto triangle_is_in_direction_of_separation_axis = Dot( separation_axis, triangle_center ) > 0;
            //    if( triangle_is_in_direction_of_separation_axis )
            //    {
            //        separation_axis = -separation_axis;
            //    }
            //    // axis belongs to two edges

            //    // which edges?
            //    auto extent_edge_index = min_depth_edge1;
            //    auto extent_vertex = CopySign( extent, -separation_axis );
            //    auto extent_direction = CopySign( axis_aligned_axes[extent_edge_index], separation_axis );

            //    auto triangle_edge_index = min_depth_edge2;
            //    auto triangle_direction = triangle_edges[triangle_edge_index];
            //    auto triangle_vertex = triangle.corners[triangle_edge_index];

            //    ClosestPoints( { extent_vertex, extent_direction }, { triangle_vertex, triangle_direction }, manifold.positions[0], manifold.positions[1] );
            //    manifold.contact_point_count = 2;
            //}

            manifold.penetration_depths.fill(min_depth);
            manifold.separation_axes.fill(separation_axis);

            return manifold;
        }
    }


    Manifold CreateManifold( BoundingShapes::AxisAlignedBox const & box, BoundingShapes::Triangle const & triangle )
    {
        auto const translated_triangle = Translate( triangle, -box.center );
        auto manifold = CreateManifoldFromExtent( box.extent, translated_triangle );
        for( size_t i = 0; i < manifold.contact_point_count; i++ )
        {
            manifold.positions[i] += box.center;
        }
        return manifold;
    }


    Manifold CreateManifold( BoundingShapes::OrientedBox const & box, BoundingShapes::Triangle const & triangle )
    {
        auto const transformed_triangle = BoundingShapes::Transform( triangle, Math::ReverseAffineTransform( box.center, box.rotation ) );
        auto manifold = CreateManifoldFromExtent( box.extent, transformed_triangle );
        for( size_t i = 0; i < manifold.contact_point_count; i++ )
        {
            manifold.positions[i] = Rotate( manifold.positions[i], box.rotation ) + box.center;
            manifold.separation_axes[i] = Rotate( manifold.separation_axes[i], box.rotation );
        }
        return manifold;
    }


	// OBB vs terrain function
	// Use density of most dense box corner as approximation of penetration depth
	// Use average of dense-corner gradients as separation axis
    Manifold CreateManifold( BoundingShapes::OrientedBox const & box, DensityFunctionType const & get_sample )
    {
        // Get corners of box
        auto const corners = GetCorners( box );

        // Reserve vectors for corner densities and gradients
        std::array<float, 8> densities;
        std::array<Math::Float3, 8> gradients;

        // Sample density and gradient for each corner position
        for( auto i = 0u; i < 8; i++ )
        {
            get_sample( corners[i], densities[i], gradients[i] );
        }

        // Create manifold, using max density as rough
        // approximation for penetration depth
        std::array<float, 8> penetration_depths;
        std::array<Math::Float3, 8> separation_axes;
        std::array<Math::Float3, 8> positions;
        std::array<uint8_t, 8> ages; ages.fill( 0 );

        uint8_t contact_point_count = 0;
        for( auto i = 0u; i < densities.size(); i++ )
        {
            auto density = densities[i];
            // Evaluate corner
            if( density > 0.0f )
            {
                // Add contact point
                positions[contact_point_count] = corners[i];
                penetration_depths[contact_point_count] = density;
                separation_axes[contact_point_count] = -Normalize(gradients[i]);
                contact_point_count += 1;
            }
        }

        return CreateManifold(
            CreateRange( penetration_depths.data(), contact_point_count ),
            CreateRange( separation_axes.data(), contact_point_count ),
            CreateRange( positions.data(), contact_point_count ),
            CreateRange( ages.data(), contact_point_count )
            );
	}


    namespace
    {
        template<typename ShapeType>
        Manifold CreateManifoldImpl( BoundingShapes::OrientedBox const box, BoundingShapes::BoundingShapeHierarchyMesh<ShapeType> const & mesh )
        {
            Manifold manifold;
            manifold.contact_point_count = 0;

            Traverse(mesh,
                [&box](ShapeType const & shape)
                {
                    return Intersect(box, shape);
                },
                [&](std::array<uint32_t,3> const & indices)
                {
                    auto const & positions = mesh.vertex_positions;
                    auto const triangle = BoundingShapes::CreateTriangle( positions[indices[0]], positions[indices[1]], positions[indices[2]] );
                    auto const new_manifold = CreateManifold( box, triangle );
                    if( new_manifold.contact_point_count > 0 )
                    {
                        manifold = MergeManifolds( manifold, new_manifold );
                    }
                    return true;
                });

            return manifold;
        }
    }


    Manifold CreateManifold( BoundingShapes::OrientedBox const & box, BoundingShapes::SphereHierarchyMesh const & mesh )
    {
        return CreateManifoldImpl( box, mesh );
    }


    Manifold CreateManifold( BoundingShapes::OrientedBox const & box, BoundingShapes::AxisAlignedBoxHierarchyMesh const & mesh )
    {
        return CreateManifoldImpl( box, mesh );
    }


    namespace
    {
        template<typename DataType>
        void AddDataPoint(uint8_t read_index, DataType& storage_destination, Range<DataType *> & all_data_range)
        {
            storage_destination = all_data_range[read_index];
            all_data_range[read_index] = First(all_data_range);
            PopFirst(all_data_range);
        }


        void AddDataPoints(uint8_t read_index, uint8_t store_index, Manifold & manifold,
            Range<float *> & all_penetration_depths_range,
            Range<Math::Float3 *> & all_separation_axes_range,
            Range<Math::Float3 *> & all_positions_range,
            Range<uint8_t *> & all_ages_range)
        {
            AddDataPoint(read_index, manifold.penetration_depths[store_index], all_penetration_depths_range);
            AddDataPoint(read_index, manifold.separation_axes[store_index], all_separation_axes_range);
            AddDataPoint(read_index, manifold.positions[store_index], all_positions_range);
            AddDataPoint(read_index, manifold.ages[store_index], all_ages_range);
        }
    }


    Manifold CreateManifold(
        Range<float *> penetration_depths,
        Range<Math::Float3 *> separation_axes,
        Range<Math::Float3 *> positions,
        Range<uint8_t *> ages)
    {
        assert(Size(penetration_depths) == Size(separation_axes));
        assert(Size(penetration_depths) == Size(positions));
        assert(Size(penetration_depths) == Size(ages));

        Manifold new_manifold;
        auto size = Size( penetration_depths );
        if( size <= Manifold::c_max_contact_points )
        {
            Copy( begin( penetration_depths ), size, new_manifold.penetration_depths.data() );
            Copy( begin( separation_axes ), size, new_manifold.separation_axes.data() );
            Copy( begin( positions ), size, new_manifold.positions.data() );
            Copy( begin( ages ), size, new_manifold.ages.data() );
            new_manifold.contact_point_count = uint8_t(size);
        }
        else
        {
            // get the deepest point
            {
                auto deepest = std::max_element( begin( penetration_depths ), end( penetration_depths ) );
                auto i = uint8_t( deepest - begin( penetration_depths ) );
                AddDataPoints( i, 0, new_manifold, penetration_depths, separation_axes, positions, ages );
            }
            // get the points that is furthest away from the others
            for( uint8_t i = 1u; i < Manifold::c_max_contact_points; ++i )
            {
                auto furthest_index = uint8_t( FindFurthestPointIndex( CreateRange(new_manifold.positions, 0u, i), positions ) );
                AddDataPoints( furthest_index, i, new_manifold, penetration_depths, separation_axes, positions, ages );
            }
            new_manifold.contact_point_count = Manifold::c_max_contact_points;
        }
        return new_manifold;
    }


    Manifold MergeManifolds( Manifold const & a, Manifold const & b )
    {
        auto count_a = a.contact_point_count;
        auto count_b = b.contact_point_count;
        auto penetration_depths_range_a = CreateRange( a.penetration_depths.data(), count_a );
        auto separation_axes_range_a = CreateRange( a.separation_axes.data(), count_a );
        auto position_range_a = CreateRange( a.positions.data(), count_a );
        auto age_range_a = CreateRange( a.ages.data(), count_a );
        auto penetration_depths_range_b = CreateRange( b.penetration_depths.data(), count_b );
        auto separation_axes_range_b = CreateRange( b.separation_axes.data(), count_b );
        auto position_range_b = CreateRange( b.positions.data(), count_b );
        auto age_range_b = CreateRange( b.ages.data(), count_b );
        auto total_count = count_a + count_b;

        Manifold new_manifold;

        if( total_count > Manifold::c_max_contact_points )
        {
            // find the deepest point and the rest that makes up the largest area
            std::array< float, Manifold::c_max_contact_points * 2 > all_penetration_depths;
            std::array< Math::Float3, Manifold::c_max_contact_points * 2 > all_separation_axes;
            std::array< Math::Float3 , Manifold::c_max_contact_points * 2 > all_positions;
            std::array< uint8_t , Manifold::c_max_contact_points * 2 > all_ages;
            Copy(begin(penetration_depths_range_a), count_a, all_penetration_depths.data());
            Copy(begin(penetration_depths_range_b), count_b, all_penetration_depths.data() + count_a);
            Copy(begin(separation_axes_range_a), count_a, all_separation_axes.data());
            Copy(begin(separation_axes_range_b), count_b, all_separation_axes.data() + count_a);
            Copy(begin(position_range_a), count_a, all_positions.data() );
            Copy(begin(position_range_b), count_b, all_positions.data() + count_a);
            Copy(begin(age_range_a), count_a, all_ages.data() );
            Copy(begin(age_range_b), count_b, all_ages.data() + count_a);
            auto all_penetration_depths_range = CreateRange(all_penetration_depths, 0, total_count);
            auto all_separation_axes_range = CreateRange(all_separation_axes, 0, total_count);
            auto all_positions_range = CreateRange(all_positions, 0, total_count);
            auto all_ages_range = CreateRange(all_ages, 0, total_count);
            new_manifold = CreateManifold(all_penetration_depths_range, all_separation_axes_range, all_positions_range, all_ages_range);
        }
        else
        {
            // copy the points directly
            new_manifold = a;
            // Copy(begin(range_a), count_a, new_manifold.contact_points.data());
            Copy(begin(penetration_depths_range_b), count_b, new_manifold.penetration_depths.data() + count_a);
            // Copy(begin(range_a), count_a, new_manifold.contact_points.data());
            Copy(begin(separation_axes_range_b), count_b, new_manifold.separation_axes.data() + count_a);
            // Copy(begin(position_range_a), Size(position_range_a), new_manifold.positions.data());
            Copy(begin(position_range_b), count_b, new_manifold.positions.data() + count_a);
            // Copy(begin(age_range_a), Size(age_range_a), new_manifold.ages.data());
            Copy(begin(age_range_b), count_b, new_manifold.ages.data() + count_a);
            new_manifold.contact_point_count = uint8_t( total_count );
        }

        return new_manifold;
    }


    void AgeManifolds(Range<Manifold *> manifolds, uint8_t expire_age_threshold)
    {
        for( auto i = 0u; i < Size(manifolds); ++i )
        {
            auto & manifold = manifolds[i];
            auto contact_point_count = manifold.contact_point_count;
            for( auto j = 0u; j < contact_point_count; )
            {
                auto age = manifold.ages[j];
                if( age >= expire_age_threshold)
                {
                    contact_point_count -= 1;
                    manifold.penetration_depths[j] = manifold.penetration_depths[contact_point_count];
                    manifold.separation_axes[j] = manifold.separation_axes[contact_point_count];
                    manifold.positions[j] = manifold.positions[contact_point_count];
                    manifold.ages[j] = manifold.ages[contact_point_count];
                }
                else
                {
                    age += 1;
                    manifold.ages[j] = age;
                    j += 1;
                }
            }
            manifold.contact_point_count = contact_point_count;
        }
    }


    void UpdatePenetrationDepth(Range<Manifold *> manifolds, float resolved_fraction)
    {
        auto factor = 1 - resolved_fraction;
        for( auto & manifold : manifolds)
        {
            for( auto i = 0u; i < manifold.contact_point_count; ++i )
            {
                manifold.penetration_depths[i] *= factor;
            }
        }
    }
}