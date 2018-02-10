#include "CollisionEventSecondaryProperties.h"

#include <Utilities\IntegerRange.h>
#include <Utilities\VectorHelper.h>
#include <Conventions\CollisionEvent.h>
#include <Math\FloatTypes.h>

#include "Algorithms.h"
#include "Movement.h"
#include "Inertia.h"


void Physics::CalculateEntityDataOffsets( 
    Range<EntityID const * > entity_ids,
    Range<uint32_t const *> entity_to_data,
    Range<uint32_t *> data_offsets 
    )
{
    assert( Size( entity_ids ) == Size( data_offsets ) );
    auto current_data_offsets = begin( data_offsets );
    for( auto & entity_id : entity_ids )
    {
        *current_data_offsets = GetOptional( entity_to_data, entity_id.index );
        ++current_data_offsets;
    }
}


void Physics::CalculateEntityDataOffsets( 
    Range<EntityID::index_t const * > entity_indices,
    Range<uint32_t const *> entity_to_data,
    Range<uint32_t *> data_offsets 
    )
{
    assert( Size( entity_indices ) == Size( data_offsets ) );
    auto current_data_offsets = begin( data_offsets );
    for( auto entity_index : entity_indices )
    {
        *current_data_offsets = GetOptional( entity_to_data, entity_index );
        ++current_data_offsets;
    }
}


void Physics::CalculateEntityDataOffsets( 
    Range<std::array<EntityID::index_t, 2> const * > entity_indices,
    Range<uint32_t const *> entity_to_data,
    bool /*second_entity_is_static*/,
    Range<std::array<uint32_t, 2>*> data_offsets 
    )
{
    CalculateEntityDataOffsets( ReinterpretRange<EntityID::index_t const>( entity_indices ), entity_to_data, ReinterpretRange<EntityID::index_t>( data_offsets ) );
}


void Physics::CalculateRelativeVelocity(
    Range<std::array<EntityID::index_t, 2> const *> event_data_offsets,
    Range<Movement const *> movements,
    Range<Inertia const *> inverse_inertias,
    Range<uint32_t const*> contact_point_data_offsets,
    Range<std::array<Math::Float3, 2> const *> contact_point_relative_positions,
    bool second_entity_is_static,
    Range<Math::Float3*> relative_velocities 
    )
{
    for( auto i : CreateIntegerRange( Size( event_data_offsets ) ) )
    {
        auto indices = event_data_offsets[i];
        auto const movement1 = movements[indices[0]];
        auto const movement2 = second_entity_is_static ? Movement{ 0, 0 } : movements[indices[1]];
        std::array<Inertia, 2> entity_inverse_inertias = { inverse_inertias[indices[0]],
            second_entity_is_static ? Inertia{ Math::Float3x3{ 0 }, 0 } : inverse_inertias[indices[1]] };

        for( auto k : CreateIntegerRange(contact_point_data_offsets[i], contact_point_data_offsets[i + 1]) )
        {
            auto relative_positions = contact_point_relative_positions[k];

            auto velocity1 = CalculateVelocity( movement1, entity_inverse_inertias[0], relative_positions[0] );
            auto velocity2 = second_entity_is_static ? 0.f : CalculateVelocity( movement2, entity_inverse_inertias[1], relative_positions[1] );

            auto relative_velocity = velocity2 - velocity1;
            relative_velocities[k] = relative_velocity;
        }
    }
}



void Physics::CalculateContactPointsOffsets( 
    Range<Manifold const*> manifolds, 
    uint32_t elements_per_point, 
    std::vector<uint32_t> & offsets 
    )
{
    auto size = std::max( offsets.size(), size_t( 1 ) );
    offsets.reserve( size + Size( manifolds ) );
    if( offsets.empty() )
    {
        offsets.push_back( 0 );
    }

    for( auto const & manifold : manifolds )
    {
        for( size_t i = 0; i < manifold.contact_point_count; i++ )
        {
            offsets.emplace_back( offsets.back() + elements_per_point );
        }
    }
}