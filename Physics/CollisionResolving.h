#pragma once

#include "BodyID.h"
#include <Utilities\Range.h>

#include <cstdint>

// forward declarations
struct Manifold;
struct Orientation;

namespace Physics
{
    void ResolveKinematicStatic( 
        Range<BodyPair const *> collision_bodies, 
        Range<Manifold const *> manifolds, 
        Range<uint32_t const *> entity_to_orientation, 
        Range<Orientation *> orientations 
        );

    void ResolveStaticKinematic( 
        Range<BodyPair const *> collision_bodies, 
        Range<Manifold const *> manifolds, 
        Range<uint32_t const *> entity_to_orientation, 
        Range<Orientation *> orientations 
        );

    void ResolveKinematicKinematic( 
        Range<BodyPair const *> collision_bodies, 
        Range<Manifold const *> manifolds, 
        Range<uint32_t const *> entity_to_orientation, 
        Range<Orientation *> orientations 
        );
}
