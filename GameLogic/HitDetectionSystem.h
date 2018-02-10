#pragma once

#include <Conventions\Orientation.h>
#include <Conventions\Velocity.h>
#include <Conventions\Force.h>

#include <Conventions\CollisionEvent.h>
#include <Conventions\AnimatingInstructions.h>

#include <Conventions\EntityIDGenerator.h>
#include <Conventions\EntitySpawnDescription.h> // Currently for hit visualization

#include <Utilities\Range.h>

#include <vector>

namespace Logic{

    struct NumericPropertyContainer;

    struct WeaponHitParameters;

    struct PlayAnimationTimers;
    struct EntityAnimations;

    // Return true if the given collision involves exactly one entity
    // with a valid entry in the inflictor_data_indices and assign roles.
    void DetermineInflictorAndTarget(EntityPair const collision_entities,
        std::vector<unsigned> const & inflictor_data_indices,
        EntityID & inflicting_entity,
        EntityID & target_entity,
        bool & valid_hit,
        bool & inflictor_vs_inflictor);

    // For all input collisions and their parallel weapon entities, calculate
    // the hit direction, relative position and velocity based on the weapon physics
    void DetermineHitProperties(
        CollisionEvents const & collisions,
        Range<EntityID const *> weapon_entities,
        IndexedVelocities const & indexed_velocities,
        Range<Math::Float3 *> hit_directions,
        Range<Math::Float3 *> relative_hit_positions,
        Range<Velocity *> hit_velocities);

    void DebugSpawnHitVisualization(IndexedOrientations const & indexed_orientations,
        EntityID const weapon_entity,
        Math::Float3 const relative_position,
        Velocity const velocity,
        EntityIDGenerator & entity_id_generator,
        std::vector<EntitySpawn>& entities_to_be_spawned);
}