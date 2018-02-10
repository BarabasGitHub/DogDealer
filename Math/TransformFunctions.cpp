#include "TransformFunctions.h"

#include "FloatMatrixOperators.h"
#include "MathFunctions.h"
#include "SSETransformFunctions.h"
#include "SSEMathConversions.h"
#include <cmath>

#include <Utilities\Range.h>

namespace Math
{
    Quaternion NormalAndAngleToQuaternion( Float3 normal, float angle )
    {
        angle *= 0.5f;
        normal *= std::sin( angle );
        angle = std::cos( angle );
        return Quaternion( normal.x, normal.y, normal.z, angle );
    }


    Quaternion AxisAndAngleToQuaternion( Float3 axis, float angle )
    {
        return NormalAndAngleToQuaternion( Normalize( axis ), angle );
    }


    Quaternion XAngleToQuaternion( const float angle )
    {
        return NormalAndAngleToQuaternion( Float3( 1, 0, 0 ), angle );
    }


    Quaternion YAngleToQuaternion( const float angle )
    {
        return NormalAndAngleToQuaternion( Float3( 0, 1, 0 ), angle );
    }


    Quaternion ZAngleToQuaternion( const float angle )
    {
        return NormalAndAngleToQuaternion( Float3( 0, 0, 1 ), angle );
    }


    Quaternion RotationMatrixToQuaternion( Float3x3 const & m )
    {

        // from here http://www.thetenthplanet.de/archives/1994
        // It also says: 'The max( 0, ... ) is just a safeguard against rounding error.'

        Quaternion q;
        q.w = Sqrt( Max( 0.0f, 1 + m(0,0) + m(1,1) + m(2,2) ) ) / 2;
        q.x = Sqrt( Max( 0.0f, 1 + m(0,0) - m(1,1) - m(2,2) ) ) / 2;
        q.y = Sqrt( Max( 0.0f, 1 - m(0,0) + m(1,1) - m(2,2) ) ) / 2;
        q.z = Sqrt( Max( 0.0f, 1 - m(0,0) - m(1,1) + m(2,2) ) ) / 2;
        auto x = m( 2, 1 ) - m( 1, 2 );
        auto y = m( 0, 2 ) - m( 2, 0 );
        auto z = m( 1, 0 ) - m( 0, 1 );
        q.x = CopySign( q.x, x );
        q.y = CopySign( q.y, y );
        q.z = CopySign( q.z, z );

        return q;
    }


    Float3x3 TranslationToFloat3x3( Float2 translation )
    {
        return Float3x3( 1, 0, translation.x,
                         0, 1, translation.y,
                         0, 0, 1 );
    }


    Float3x3 RotationToFloat3x3( Quaternion q )
    {
        // based on rotating the 1,0,0 0,1,0 0,0,1 vectors with q

        // so we don't have to multiply with 2 later
        q *= c_SQRT2;
        auto const axis = GetAxis( q );
        auto const qw = axis * q.w;
        auto const qq = axis * axis;
        auto const xy = axis.x * axis.y;
        auto const xz = axis.x * axis.z;
        auto const yz = axis.y * axis.z;

        return{
            1 - qq.y - qq.z, xy - qw.z, xz + qw.y,
            xy + qw.z, 1 - qq.z - qq.x, yz - qw.x,
            xz - qw.y, yz + qw.x, 1 - qq.x - qq.y
        };
    }


    Float3x3 Float4x4ToFloat3x3( Float4x4 m )
    {
        return Float3x3(
            m(0,0), m(0,1), m(0,2),
            m(1,0), m(1,1), m(1,2),
            m(2,0), m(2,1), m(2,2));
    }


    Float3x3 ScaleToFloat3x3( Float3 scale )
    {
        return Float3x3(
            scale.x, 0, 0,
            0, scale.y, 0,
            0, 0, scale.z );
    }


    Float4x4 CombineFloat3x3AndTranslation( Float3x3 const & r, Float3 const & t )
    {
        return{
            { Float4(r.row[0], t.x) },
            { Float4(r.row[1], t.y) },
            { Float4(r.row[2], t.z) },
            { 0, 0, 0, 1 } };
    }


    Float4x4 ScaleToFloat4x4( Float3 scale )
    {
        return{ { scale[0], 0, 0, 0 },
                { 0, scale[1], 0, 0 },
                { 0, 0, scale[2], 0 },
                { 0, 0, 0, 1 } };
    }


    Float4x4 RotationToFloat4x4( Quaternion q )
    {
        return SSE::Float4x4FromSSE( SSE::RotationMatrixFromQuaternion( SSE::SSEFromQuaternion( q ) ) );
        //return CombineFloat3x3AndTranslation( RotationToFloat3x3( q ), 0 );
    }


    Float4x4 RotationAroundPositionToFloat4x4( Quaternion rotation, Float3 rotation_position )
    {
        auto const r = RotationToFloat3x3( rotation );
        auto const t = rotation_position - r * rotation_position;
        return CombineFloat3x3AndTranslation( r, t );
    }


    Float4x4 TranslationToFloat4x4( Float3 t )
    {
        return{
            1, 0, 0, t.x,
            0, 1, 0, t.y,
            0, 0, 1, t.z,
            0, 0, 0, 1,
        };
    }


    // translation * rotation
    Float4x4 AffineTransform( Float3 const & translation, Quaternion const & rotation )
    {
        //auto const r = RotationToFloat3x3( rotation );
        //return CombineFloat3x3AndTranslation( r, translation );
        return SSE::Float4x4FromSSE( SSE::AffineTransformTranslationRotation( SSE::SSEFromFloat3( translation ), SSE::SSEFromQuaternion( rotation ) ) );
    }


    // rotation^-1 * -translation
    Float4x4 ReverseAffineTransform( Float3 const & translation, Quaternion const & rotation )
    {
        auto const r = RotationToFloat3x3( Conjugate(rotation) );
        auto const t = r * translation;
        return CombineFloat3x3AndTranslation( r, -t );
    }


    Float4x4 AffineTransform( Float3 translation, Float3 scale )
    {
        return{
            scale.x, 0, 0, translation.x,
            0, scale.y, 0, translation.y,
            0, 0, scale.z, translation.z,
            0, 0, 0, 1 };
    }

    namespace
    {
        inline Float4x4 ScaleTransformationMatrix(Float4x4 m, Float3 scale)
        {
            auto scale_vector = Float4( scale.x, scale.y, scale.z, 1 );
            for( auto i = 0u; i < 3; ++i )
            {
                m.row[i] *= scale_vector;
            }
            return m;
        }
    }


    // translation * rotation * scale
    Float4x4 AffineTransform( Float3 translation, Quaternion rotation, Float3 scale )
    {
        auto transformation = AffineTransform( translation, rotation );
        return ScaleTransformationMatrix(transformation, scale);
    }


    Float4x4 AffineTransform( Float3 translation, Float3 rotation_origin, Quaternion rotation )
    {
        //auto translation_matrix = TranslationToFloat4x4( translation );
        auto rotation_matrix = RotationAroundPositionToFloat4x4( rotation, rotation_origin );
        //return translation_matrix * rotation_matrix;
        rotation_matrix(0,3) += translation[0];
        rotation_matrix(1,3) += translation[1];
        rotation_matrix(2,3) += translation[2];
        return rotation_matrix;
    }


    // translation * rotation_origin * rotation * -rotation_origin * scale
    Float4x4 AffineTransform( Float3 translation, Float3 rotation_origin, Quaternion rotation, Float3 scale )
    {
        auto m = AffineTransform( translation, rotation_origin, rotation );
        return ScaleTransformationMatrix(m, scale);
    }


    Float4x4 OrthographicView( float height, float width, float near_z, float far_z )
    {
        assert( width > 0 );
        assert( height > 0 );
        assert( Abs( far_z - near_z ) > 0 );

        auto range = 1.0f / ( near_z - far_z );
        width = 2.0f / width;
        height = 2.0f / height;

        return Float4x4( width, 0, 0, 0,
                         0, height, 0, 0,
                         0, 0, range, range * near_z,
                         0, 0, 0, 1 );
    }


    Float4x4 OrthographicOffCenterView( Math::Float3 size, Math::Float3 center )
    {
        assert( Abs(size.x) > 0.0f );
        assert( Abs(size.y) > 0.0f );
        assert( Abs(size.z) > 0.0f );

        size.z = -size.z;
        auto const reciprocal_size = 1.0f / size;
        auto near_z = center.z + size.z / 2;
        return Float4x4( reciprocal_size.x * 2, 0, 0, -center.x / size.x,
                         0, reciprocal_size.y * 2, 0, -center.y / size.y,
                         0, 0, reciprocal_size.z, reciprocal_size.z * near_z,
                         0, 0, 0, 1 );
    }


    Float4x4 OrthographicOffCenterView( float const left, float const right, float const bottom, float const top, float const near_z, float const far_z )
    {
        assert( right - left > 0.00001f );
        assert( top - bottom > 0.00001f );
        assert( Abs( far_z - near_z ) > 0.00001f );

        auto const size = Math::Float3( right - left, top - bottom, far_z - near_z );
        auto const center = Math::Float2( right + left, bottom + top );
        auto const reciprocal_size = 1.0f / size;

        return Float4x4( reciprocal_size.x * 2, 0, 0, - center.x / size.x,
                         0, reciprocal_size.y * 2, 0, - center.y / size.y,
                         0, 0, -reciprocal_size.z, reciprocal_size.z * -near_z,
                         0, 0, 0, 1 );
    }


    Float4x4 PerspectiveHeightWidth( float height, float width, float near_z, float far_z )
    {
        assert( Abs( far_z - near_z ) > 0.00001f );

        float range = far_z / ( near_z - far_z );
        return Float4x4( width, 0, 0, 0,
                         0, height, 0, 0,
                         0, 0, range, range * near_z,
                         0, 0, -1, 0 );
    }


    Float4x4 PerspectiveFieldOfViewVertical( float field_of_view, float aspect_ratio, float near_z, float far_z )
    {
        assert( field_of_view > 0.00001f * 2.0f );
        assert( aspect_ratio > 0.00001f );

        field_of_view *= 0.5f;
        //float height = std::cos( field_of_view ) / std::sin( field_of_view );
        float height = cot( field_of_view );
        float width = height / aspect_ratio;
        return PerspectiveHeightWidth(height, width, near_z, far_z);
    }


    Float4x4 PerspectiveFieldOfViewHorizontal( float field_of_view, float aspect_ratio, float near_z, float far_z )
    {
        assert( field_of_view > 0.00001f * 2.0f );
        assert( aspect_ratio > 0.00001f );

        field_of_view *= 0.5f;
        //float height = std::cos( field_of_view ) / std::sin( field_of_view );
        float width = cot( field_of_view );
        float height = width * aspect_ratio;
        return PerspectiveHeightWidth(height, width, near_z, far_z);
    }


    void BlendTransforms( Range<Math::Float4x4 const *> transforms, Range<Math::Float4x4 const *> previous_transforms, float const blend_factor, Range<Math::Float4x4 *> output_buffer )
    {
        assert( Size(transforms) == Size(previous_transforms) );
        assert( Size( output_buffer ) == Size( transforms ) );
        auto size = Size(transforms);

        for( auto i = 0u; i < size; ++i )
        {
           output_buffer[i] = Lerp( previous_transforms[i], transforms[i], blend_factor );
        }
    }
}