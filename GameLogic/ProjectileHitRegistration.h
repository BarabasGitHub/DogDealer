#pragma once
#include <Conventions\Orientation.h>
#include <Conventions\Velocity.h>
#include <Conventions\Force.h>

#include <Conventions\CollisionEvent.h>
#include <Conventions\AnimatingInstructions.h>

#include <Conventions\EntityIDGenerator.h>
#include <Conventions\EntitySpawnDescription.h> // Currently for hit visualization

#include <vector>

namespace Logic{

    struct ProjectileContainer;
    struct PlayAnimationTimers;
    struct NumericPropertyContainer;
    struct EntityAnimations;

    void ExtractValidProjectileCollisions(
        CollisionEvents const & collision_events,
        ProjectileContainer const & projectile_container,
        CollisionEvents & projectile_collisions,
        std::vector<EntityID> & projectile_entities,
        std::vector<EntityID> & target_entities,
        std::vector<EntityID> & wielder_entities);

    void ProcessProjectileCollisionEvents(
        CollisionEvents const & collision_events,
        ProjectileContainer & projectile_container,
        IndexedVelocities const & indexed_velocities,
        std::vector<EntityID> & target_entities,
        std::vector<EntityID> & projectiles_that_hit_something,
        std::vector<EntityID> & entities_that_hit_something,
        std::vector<Math::Float3> & impact_velocities,
        CollisionEvents & output_weapon_collisions);
}