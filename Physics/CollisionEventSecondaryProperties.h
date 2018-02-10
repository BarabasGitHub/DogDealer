#pragma once

#include <Utilities\Range.h>

#include <Conventions\EntityID.h>
#include <Math\ForwardDeclarations.h>

#include <vector>
#include <array>

struct Manifold;

namespace Physics
{
    struct Movement;
    struct Inertia;


    void CalculateEntityDataOffsets( 
        Range<EntityID const * > entity_ids,
        Range<uint32_t const *> entity_to_data,
        Range<uint32_t *> data_offsets 
        );

    void CalculateEntityDataOffsets( 
        Range<EntityID::index_t const * > ids,
        Range<uint32_t const *> entity_to_data,
        Range<uint32_t *> data_offsets 
        );

    void CalculateEntityDataOffsets( 
        Range<std::array<EntityID::index_t, 2> const * > ids,
        Range<uint32_t const *> entity_to_data,
        bool second_entity_is_static,
        Range<std::array<uint32_t, 2>*> data_offsets 
        );

    void CalculateRelativeVelocity( 
        Range<std::array<EntityID::index_t, 2> const *> event_data_offsets,
        Range<Movement const *> movements,
        Range<Inertia const *> inverse_inertias,
        Range<uint32_t const*> contact_point_data_offsets,
        Range<std::array<Math::Float3, 2> const *> contact_point_relative_positions,
        bool second_entity_is_static,
        Range<Math::Float3*> relative_velocities 
        );

    void CalculateContactPointsOffsets( 
        Range<Manifold const*> manifolds, 
        uint32_t elements_per_point, 
        std::vector<uint32_t> & offsets 
        );
}