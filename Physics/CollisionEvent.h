#pragma once
#include "BodyID.h"
#include <Conventions\CollisionManifold.h>

#include <Math\FloatTypes.h>
#include <Utilities\Range.h>

#include <vector>

namespace Physics
{
    struct CollisionEvents
    {
        // collided entities
        // if you flip these you also have to invert the corresponding relative_position and adjust the contact positions in the manifold
        std::vector<BodyPair> bodies;
        // positions of entity 2 relative to entity 1
        std::vector<Math::Float3> relative_positions;
        // collision manifolds
        std::vector<Manifold> manifolds;
    };
}

void Clear(Physics::CollisionEvents & ce);
uint32_t Size(Physics::CollisionEvents const & ce);
void Resize(size_t size, Physics::CollisionEvents & ce);
void Reorder(Range<uint32_t const *> indices, Physics::CollisionEvents & ce);
void Append(Physics::CollisionEvents & a, Physics::CollisionEvents const & b);
void Flip(Physics::BodyPair & bodies, Math::Float3 & relative_position, Manifold & manifold);
void PutLowerOrderBodyFirst(Range<uint32_t const *> body_to_order, Physics::CollisionEvents & events);
void PutHigherOrderBodyFirst(Range<uint32_t const *> body_to_order, Physics::CollisionEvents & events);


template<typename CompareFunction>
void FlipIf(Physics::CollisionEvents & events, CompareFunction const & compare)
{
    auto size = ::Size(events.bodies);
    for(auto i = 0u; i < size; ++i)
    {
        auto bodies = events.bodies[i];
        if(compare(bodies))
        {
            Flip(events.bodies[i], events.relative_positions[i], events.manifolds[i]);
        }
    }
}