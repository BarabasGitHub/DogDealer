#pragma once
#include "ForwardDeclarations.h"
#include "FloatTypes.h"
#include "FloatMatrixTypes.h"
#include <Utilities\Range.h>

namespace Math
{

    Quaternion NormalAndAngleToQuaternion( Float3, float angle );
    Quaternion AxisAndAngleToQuaternion( Float3, float angle );
    Quaternion XAngleToQuaternion( const float angle );
    Quaternion YAngleToQuaternion( const float angle );
    Quaternion ZAngleToQuaternion( const float angle );
    Quaternion RotationMatrixToQuaternion( Float3x3 const & rotation_matrix );

    Float3x3 TranslationToFloat3x3( Float2 translation );

    Float3x3 RotationToFloat3x3( Quaternion rotation );
    Float3x3 ScaleToFloat3x3( Float3 scale );

    Float3x3 Float4x4ToFloat3x3( Float4x4 other );

    Float4x4 CombineFloat3x3AndTranslation( Float3x3 const &, Float3 const & translation );

    Float4x4 TranslationToFloat4x4( Float3 translation );
    Float4x4 RotationToFloat4x4( Quaternion rotation );
    Float4x4 ScaleToFloat4x4( Float3 scale );

    // inverse translation by rotation_origin followed by a rotation 'rotation' and finally a translation by rotation_origin
    Float4x4 RotationAroundPositionToFloat4x4( Quaternion rotation, Float3 rotation_position );

    // rotation followed by a translation
    Float4x4 AffineTransform( Float3 const & translation, Quaternion const & rotation );
    // inverse translation followed by a inverse rotation
    Float4x4 ReverseAffineTransform( Float3 const & translation, Quaternion const & rotation );
    // scale followed by a translation
    Float4x4 AffineTransform( Float3 translation, Float3 scale );
    // inverse translation by rotation_origin followed by a rotation 'rotation', a translation by rotation_origin and another translation by 'translation'
    Float4x4 AffineTransform( Float3 translation, Float3 rotation_origin, Quaternion rotation );
    // scale followed by a rotation and a translation
    Float4x4 AffineTransform( Float3 translation, Quaternion rotation, Float3 scale );
    // scale * -rotation_origin * rotation * rotation_origin * translation
    Float4x4 AffineTransform( Float3 translation, Float3 rotation_origin, Quaternion rotation, Float3 scale );
    Float4x4 OrthographicView( float height, float width, float near_z, float far_z );
    Float4x4 PerspectiveHeightWidth( float height, float width, float near_z, float far_z );
    Float4x4 PerspectiveFieldOfViewVertical( float field_of_view, float aspect_ratio, float near_z, float far_z );
    Float4x4 PerspectiveFieldOfViewHorizontal( float field_of_view, float aspect_ratio, float near_z, float far_z );

    void BlendTransforms( Range<Math::Float4x4 const *> transforms, Range<Math::Float4x4 const *> previous_transforms, float const blend_factor, Range<Math::Float4x4 *> output_buffer );
}