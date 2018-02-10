#pragma once
#include "EntityID.h"
#include "CollisionManifold.h"

#include <Math\FloatTypes.h>
#include <Utilities\Range.h>

#include <vector>


struct CollisionEvents
{
    // collided entities
    // if you flip these you also have to invert the corresponding relative_position and adjust the contact positions in the manifold
    std::vector<EntityPair> entities;
    // positions of entity 2 relative to entity 1
    std::vector<Math::Float3> relative_positions;
    // collision manifolds
    std::vector<Manifold> manifolds;
};


void Clear(CollisionEvents & ce);
uint32_t Size(CollisionEvents const & ce);
void Resize(size_t size, CollisionEvents & ce);
void Reorder(Range<uint32_t const *> indices, CollisionEvents & ce);
void Append(CollisionEvents & a, CollisionEvents const & b);
void Flip( EntityPair & entities, Math::Float3 & relative_position, Manifold & manifold );
void PutLowerOrderEntityFirst( Range<uint32_t const *> entity_to_order, CollisionEvents & events );
void PutHigherOrderEntityFirst( Range<uint32_t const *> entity_to_order, CollisionEvents & events );


template<typename CompareFunction>
void FlipIf(CollisionEvents & events, CompareFunction const & compare)
{
    auto size = Size( events.entities );
    for( auto i = 0u; i < size; ++i )
    {
        auto entities = events.entities[i];
        if( compare(entities) )
        {
            Flip( events.entities[i], events.relative_positions[i], events.manifolds[i] );
        }
    }
}