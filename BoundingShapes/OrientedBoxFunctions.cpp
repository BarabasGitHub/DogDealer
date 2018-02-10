#include "OrientedBoxFunctions.h"

#include "Implementation.h"

#include <Math\FloatOperators.h>
#include <Math\MathFunctions.h>
#include <Math\PointFunctions.h>
#include <Math\SSEFloatFunctions.h>
#include <Math\SSEMathConversions.h>
#include <Math\SSEQuaternionFunctions.h>
#include <Math\TransformFunctions.h>

#include <Utilities\IntegerRange.h>

#include <DirectXCollision.h>

using namespace Math;

namespace BoundingShapes
{


    OrientedBox CreateOrientedBox( Range<Math::Float3 const *> points )
    {
        DirectX::BoundingOrientedBox dx_box;
        static_assert( sizeof( Math::Float3 ) == sizeof( DirectX::XMFLOAT3 ), "Can't reinterpret cast Math::Float3 to DirectX::XMFLOAT3" );
        DirectX::BoundingOrientedBox::CreateFromPoints( dx_box, Size(points), reinterpret_cast<DirectX::XMFLOAT3 const *>( begin(points)), sizeof( Math::Float3 ) );
        OrientedBox box;
        box.center = { dx_box.Center.x, dx_box.Center.y, dx_box.Center.z };
        box.extent = { dx_box.Extents.x, dx_box.Extents.y, dx_box.Extents.z };
        box.rotation = { dx_box.Orientation.x, dx_box.Orientation.y, dx_box.Orientation.z, dx_box.Orientation.w };
        return box;
    }


    OrientedBox ReconstructOrientedBoxFromCorners( Range<Math::Float3 const *> points )
    {
        assert(Size(points) == 8);
        OrientedBox box;
        box.center = points[0];
        for( auto i = 1u; i < Size(points); ++i )
        {
            box.center += points[i];
        }
        box.center /= float(Size(points));
        auto reference_corner = FindClosestPointIndex(0, points);
        auto second = FindClosestPointIndex(reference_corner - 1, CreateRange(points, 1)) + 1;
        // thanks to http://math.stackexchange.com/a/410893/240844 for finding the right axes
        auto axis1 = Normalize(points[0] - points[reference_corner]);
        assert(Dot(axis1, box.center - points[reference_corner]) > 0);
        auto axis3 = Normalize(Cross(axis1, points[second] - points[reference_corner]));
        auto axis2 = Cross(axis3, axis1);
        Float3x3 rotation_matrix = {axis1, axis2, axis3};
        box.rotation = Conjugate(RotationMatrixToQuaternion(rotation_matrix));
        box.extent = Abs(rotation_matrix * (First(points) - box.center));
        return box;
    }


    OrientedBox Rotate( OrientedBox box, Math::Quaternion const & rotation_in )
    {
        //box.center = Rotate( box.center, rotation );
        //return RotateAroundCenter( box, rotation );
        auto rotation = SSE::SSEFromQuaternion( rotation_in );
        box.center = SSE::Float3FromSSE( SSE::QuaternionRotate3DVector( SSE::SSEFromFloat3( box.center ), rotation ) );
        box.rotation = SSE::QuaternionFromSSE( SSE::QuaternionMultiply( rotation, SSE::SSEFromQuaternion( box.rotation ) ));
        return box;
    }


    // rotates the box around its center.
    OrientedBox RotateAroundCenter( OrientedBox box, Math::Quaternion const & rotation )
    {
        box.rotation = rotation * box.rotation;
        return box;
    }


    OrientedBox RotateAroundCenter( OrientedBox box, Math::Float3x3 const & rotation )
    {
        return RotateAroundCenter( box, RotationMatrixToQuaternion( rotation ) );
    }


    OrientedBox Transform( OrientedBox box, Math::Float4x4 const & transform )
    {
        box.center = TransformPosition( box.center, transform );
        return RotateAroundCenter( box, Float4x4ToFloat3x3( transform ) );
    }


    OrientedBox TransformByOrientation( OrientedBox box, Orientation const & orientation )
    {
        box.center = Rotate( box.center, orientation.rotation );
        box.center += orientation.position;
        return RotateAroundCenter( box, orientation.rotation );
    }


    std::vector<OrientedBox> TransformByOrientation( Range<OrientedBox const*> const boxes, Range<Orientation const *> const orientations )
    {
        auto size = Size(boxes);
        std::vector<OrientedBox> new_boxes;
        new_boxes.reserve( size );
        for( auto i : CreateIntegerRange( size ) )
        {
            new_boxes.emplace_back( TransformByOrientation( boxes[i], orientations[i] ) );
        }
        return new_boxes;
    }


    std::array<Math::Float3, 8> GetCorners( OrientedBox const & box )
    {
        std::array<Float3, 8> corners;
        auto center = SSE::SSEFromFloat3( box.center );
        auto extent = SSE::SSEFromFloat3( box.extent );
        auto rotation = SSE::SSEFromQuaternion( box.rotation );
        for( auto i = 0; i < 8; ++i )
        {
            corners[i] = SSE::Float3FromSSE( SSE::Add( center, SSE::QuaternionRotate3DVector( SSE::Multiply( Details::box_corner_factors_sse[i], extent ), rotation ) ) );
        }
        return corners;
    }


    std::array<Math::Float3, 6> GetAllFaceNormals( BoundingShapes::OrientedBox const & box )
    {
        std::array<Math::Float3, 6> rotated_normals{};
        for( uint8_t i = 0; i < 6; i++ )
        {
            rotated_normals[i] = GetFaceNormal( box, i );
        }
        return rotated_normals;
    }


    Math::Float3 GetFaceNormal( BoundingShapes::OrientedBox const & box, uint8_t face_index )
    {
        auto normal = Details::axis_aligned_face_normals[face_index];
        normal = Rotate( normal, box.rotation );
        return normal;
    }


    std::array<Math::Float3, 4> GetFaceCorners( OrientedBox const & box, uint8_t face_index )
    {
        std::array<Float3, 4> corners;
        for( auto i = 0; i < 4; ++i )
        {
            corners[i] = box.center + Rotate( Details::axis_aligned_face_corner_factors[face_index][i] * box.extent, box.rotation );
        }
        return corners;
    }


    float Volume(OrientedBox const & box)
    {
        auto size = box.extent + box.extent;
        return size.x * size.y * size.z;
    }


    Math::Float3 CalculateWeightedCenter(Range<OrientedBox const *> boxes)
    {
        float volume;
        return CalculateWeightedCenter(boxes, volume);
    }


    Math::Float3 CalculateWeightedCenter(Range<OrientedBox const *> boxes, float & output_total_volume)
    {
        Math::Float3 center = 0;
        float total_volume = 0; // actually 1/8 volume
        for( auto & box : boxes )
        {
            auto extent = box.extent;
            // actualy 1/8 volume
            auto volume = extent.x * extent.y * extent.z;
            total_volume += volume;
            center += box.center * volume;
        }
        center /= total_volume;
        output_total_volume = total_volume * 8;
        return center;
    }
}