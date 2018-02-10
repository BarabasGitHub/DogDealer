#pragma once

#include "Orientation.h"
#include <Utilities\Range.h>

bool Equal( Orientation const& a, Orientation const& b, float const tolerance );

// get the orientation of a relative to b
// this means the resulting orientation has the same effect as 'undoing' orientation b and then applying orientation a
Orientation GetOffset( const Orientation& a, const Orientation& b );

// Somehow adding them up and ufuff!
Orientation ToParentFromLocal( Orientation const & parent, Orientation const & local );

// Applying the result will undo orientation a.
// This means that applying a and then applying the result will have no net result.
Orientation Invert( Orientation a );

// create transforms from orientations
void CreateTransforms( Range<Orientation const *> orientations, Range<Math::Float4x4 *> transforms );

// create transforms with an offset position
void CreateTransforms( Range<Orientation const *> orientations, Math::Float3 position_offset, Range<Math::Float4x4 *> transforms );

// blend the rotations of the orientations indexed by the entities
void BlendRotations( Range<EntityID const *> const entities, IndexedOrientations const & orientations, float const blend_factor, Range<Math::Quaternion *> rotations );

// gets the positions of all entities in 'entities'
// assumes all entities actually have a position
void GetEntityPositions( Range<EntityID const *> entities, IndexedOrientations const & orientations, Range<Math::Float3 *> positions);


// gets the rotations of all entities in 'entities'
// assumes all entities actually have a rotation
void GetEntityRotations(Range<EntityID const *> entities, IndexedOrientations const & orientations, Range<Math::Quaternion *> rotations);

// gets all the angles around the positive z axis for all entities
// assumes all entities have a rotation
void GetEntityZAngles(Range<EntityID const *> entities, IndexedOrientations const & orientations, Range<float *> z_angles);