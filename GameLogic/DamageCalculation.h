#pragma once

#include <Utilities\Range.h>
#include <Math\ForwardDeclarations.h>
#include <Conventions\EntityID.h>

#include <vector>

struct CollisionEvents;
struct IndexedVelocities;

namespace Logic
{

    void CalculateDamage(
        Range<float const *> impact_speeds,
        Range<EntityID const *> damage_inflicting_entities,
        Range<uint32_t const*> entity_to_damage_parameters,
        Range<float const*> damage_parameters,
        Range<float *> damages);

    void CalculateDamage(
        Range<Math::Float3 const *> impact_velocities,
        Range<EntityID const *> damage_inflicting_entities,
        Range<uint32_t const*> entity_to_damage_parameters,
        Range<float const*> damage_parameters,
        Range<float *> damages);

    void DamageTargets(
        Range<float const*> damages,
        Range<EntityID const *> target_entities,
        Range<uint32_t const *> entity_to_hitpoints,
        Range<float *> hitpoints,
        std::vector<EntityID> & killed_entities);

    void CalculateHitsAndDamageTargets(
        CollisionEvents const & collision_events,
        IndexedVelocities const & velocities,
        Range<uint32_t const *> entity_to_damage_parameters,
        Range<float const *> damage_parameters,
        Range<uint32_t const *> entity_to_hitpoints,
        Range<float *> hitpoints,
        std::vector<EntityID> & damage_dealing_entities,
        std::vector<EntityID> & damage_receiving_entities,
        std::vector<EntityID> & killed_entities);
}