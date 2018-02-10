#include "Orientation.h"

#include <Math\SSE.h>
#include <Math\SSEMathConversions.h>
#include <Math\MathFunctions.h>
#include <Math\TransformFunctions.h>

#include <Utilities\Range.h>

#include <algorithm>

bool Equal( Orientation const& a, Orientation const& b, float const tolerance )
{
    return Equal( a.position, b.position, tolerance ) && Equal( Math::Float4( a.rotation ), Math::Float4( b.rotation ), tolerance );
}


// get the orientation of a relative to b
Orientation GetOffset( const Orientation& a, const Orientation& b )
{
    using namespace Math::SSE;
    Orientation result;

    // make the right rotation by 'subtracting' the rotation of b from the rotation of a
    auto b_rotation = SSEFromQuaternion( b.rotation );
    auto a_rotation = SSEFromQuaternion( a.rotation );
    auto result_rotation = QuaternionMultiply( a_rotation, QuaternionConjugate( b_rotation ) );
    result.rotation = QuaternionFromSSE( result_rotation );

    // make the position by first calculating the difference and then rotating it into the local reference frame of b
    auto a_position = SSEFromFloat3( a.position );
    auto b_position = SSEFromFloat3( b.position );
    result.position = Float3FromSSE( Subtract( a_position, QuaternionRotate3DVector( b_position, result_rotation ) ) );
    return result;
}

// Somehow adding them up and ufuff!
Orientation ToParentFromLocal( Orientation const & parent, Orientation const & local )
{
    using namespace Math::SSE;
    Orientation result;
    //result.rotation = parent.rotation * local.rotation;
    auto parent_rotation = SSEFromQuaternion( parent.rotation );
    auto local_rotation = SSEFromQuaternion( local.rotation );
    auto result_rotation = QuaternionMultiply( parent_rotation, local_rotation );
    result.rotation = QuaternionFromSSE( result_rotation );

    //result.position = parent.position + Rotate( local.position, parent.rotation );
    auto local_position = SSEFromFloat3( local.position );
    auto parent_position = SSEFromFloat3( parent.position );
    auto result_position = Add( parent_position, QuaternionRotate3DVector( local_position, parent_rotation ) );
    result.position = Float3FromSSE( result_position );
    return result;
}


Orientation Invert( Orientation a )
{
    a.rotation = Conjugate( a.rotation );
    a.position = Rotate(-a.position, a.rotation);
    return a;
}




void CreateTransforms( Range<Orientation const *> orientations, Range<Math::Float4x4 *> transforms )
{
    assert(Size(orientations) == Size(transforms) );
    auto size = Size( orientations );
    for( auto i = 0u; i < size; ++i )
    {
        auto orientation = orientations[i];
        transforms[i] = AffineTransform( orientation.position, orientation.rotation );
    }
}


void CreateTransforms( Range<Orientation const *> orientations, Math::Float3 position_offset, Range<Math::Float4x4 *> transforms )
{
    assert(Size(orientations) == Size(transforms) );
    auto size = Size( orientations );
    for( auto i = 0u; i < size; ++i )
    {
        auto orientation = orientations[i];
        transforms[i] = AffineTransform( orientation.position + position_offset, orientation.rotation );
    }
}


void BlendRotations( Range<EntityID const *> const entities, IndexedOrientations const & orientations, float const float_blend_factor, Range<Math::Quaternion *> rotations )
{
    assert( Size(rotations) == Size( entities ) );
    if( IsEmpty( entities ) ) return;
    auto blend_factor = Math::SSE::SetAll(float_blend_factor);
    std::transform( begin( entities ), end( entities ), begin( rotations ),
                    [blend_factor, &orientations]( EntityID entity_id )
    {
        using namespace Math::SSE;
        auto const orientation_index = orientations.indices[entity_id.index];
        auto previous_rotation = SSEFromQuaternion(orientations.previous_orientations[orientation_index].rotation);
        auto current_rotation = SSEFromQuaternion(orientations.orientations[orientation_index].rotation);
        auto const rotation = Nlerp4( current_rotation, previous_rotation, blend_factor );
        return QuaternionFromSSE(rotation);
    } );
}


void GetEntityPositions( Range<EntityID const *> entities, IndexedOrientations const & orientations, Range<Math::Float3 *> positions)
{
    assert(Size(entities) == Size(positions));
    auto count = Size(entities);
    for (auto i = 0u; i < count; ++i)
    {
        auto& entity = entities[i];
        auto index = orientations.indices[entity.index];
        auto& orientation = orientations.orientations[index];
        auto position = orientation.position;
        positions[i] = position;
    }
}


void GetEntityRotations(Range<EntityID const *> entities, IndexedOrientations const & orientations, Range<Math::Quaternion *> rotations)
{
    assert(Size(entities) == Size(rotations));
    auto count = Size(entities);
    for (auto i = 0u; i < count; ++i)
    {
        auto& entity = entities[i];
        auto index = orientations.indices[entity.index];
        auto& orientation = orientations.orientations[index];
        auto rotation = orientation.rotation;
        rotations[i] = rotation;
    }
}


void GetEntityZAngles(Range<EntityID const *> entities, IndexedOrientations const & orientations, Range<float *> z_angles)
{
    assert(Size(entities) == Size(z_angles));
    auto count = Size(entities);
    for (auto i = 0u; i < count; ++i)
    {
        auto& entity = entities[i];
        auto index = orientations.indices[entity.index];
        auto& orientation = orientations.orientations[index];
        auto rotation = orientation.rotation;
        Math::Quaternion z_rotation, unused;
        DecomposeInSimpleAndTwistRotation(unused, z_rotation, rotation, Math::Float3(0,0,1));
        auto z_angle = GetAngle(z_rotation);
        z_angles[i] = z_angle;
    }
}