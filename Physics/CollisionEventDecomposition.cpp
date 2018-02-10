#include "CollisionEventDecomposition.h"

#include <Conventions\CollisionEvent.h>

#include <Math\FloatTypes.h>
#include <Math\MathFunctions.h>
#include <Utilities\Memory.h>

void Physics::CopyPerPointProperties( Range<Manifold const *> manifolds, Range<Math::Float3 const*> event_relative_positions, Range<Math::Float3*> normals, Range<float *> penetration_depths, Range<std::array<Math::Float3, 2>*> point_relative_positions )
{
    assert(Size(manifolds) == Size(event_relative_positions));
    auto i = 0u;
    for( auto k = 0u; k < Size(manifolds); ++k )
    {
        auto const & manifold = manifolds[k];
        auto relative_position = event_relative_positions[k];

        Copy(manifold.penetration_depths.data(), manifold.contact_point_count, begin(penetration_depths) + i);
        Copy(manifold.separation_axes.data(), manifold.contact_point_count, begin(normals) + i);

        for( uint8_t j = 0u; j < manifold.contact_point_count; ++j, ++i )
        {
            auto contact_position = manifold.positions[j];
            point_relative_positions[i] = { { contact_position, contact_position - relative_position } };
        }
    }
}


void Physics::GetContactPointCounts( Range<Manifold const *> manifolds, Range<uint32_t*> contact_point_counts )
{
    assert(Size(manifolds) == Size(contact_point_counts));
    for( auto i = 0u; i < Size( manifolds ); ++i )
    {
        contact_point_counts[i] = manifolds[i].contact_point_count;
    }
}


void Physics::GetEntityIndices( Range<EntityPair const *> entities, Range<std::array<EntityID::index_t, 2> * > entity_indices )
{
    assert(Size(entities) == Size(entity_indices));
    for( auto i = 0u; i < Size( entities ); ++i )
    {
        entity_indices[i][0] = entities[i].id1.index;
        entity_indices[i][1] = entities[i].id2.index;
    }
}