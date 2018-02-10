#pragma once

#include <Utilities\Range.h>

#include <Math\ForwardDeclarations.h>

#include <Conventions\EntityID.h>

#include <array>
struct Manifold;

namespace Physics
{
    void CopyPerPointProperties( 
        Range<Manifold const *> manifolds, 
        Range<Math::Float3 const*> event_relative_positions, 
        Range<Math::Float3*> normals, 
        Range<float *> penetration_depths, 
        Range<std::array<Math::Float3, 2>*> point_relative_positions 
        );

    void GetContactPointCounts( 
        Range<Manifold const *> manifolds, 
        Range<uint32_t*> contact_point_counts 
        );

    void GetEntityIndices( 
        Range<EntityPair const *> entities, 
        Range<std::array<EntityID::index_t, 2> *> entity_indices 
        );
}