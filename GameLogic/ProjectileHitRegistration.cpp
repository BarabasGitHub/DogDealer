#include "ProjectileHitRegistration.h"

#include "DamageCalculation.h"
#include "EntityAnimations.h"
#include "HitDetectionSystem.h"
#include "ProjectileContainer.h"
#include "PropertyContainer.h"
#include "Structures.h"
#include "TimerSystem.h"

#include <Utilities\StdVectorFunctions.h>

float const c_impact_velocity_modifier = 7.0f;

using namespace Logic;

void Logic::ExtractValidProjectileCollisions(
    // std::vector<EntityPair> const & collision_etities,
    CollisionEvents const & collision_events,
    ProjectileContainer const & projectile_container,
    CollisionEvents & weapon_collisions,
    std::vector<EntityID> & weapon_entities,
    std::vector<EntityID> & target_entities,
    std::vector<EntityID> & emitter_entities)
{

    for (auto i = 0u; i < collision_events.manifolds.size(); i++)
    {
        auto collision_entities_pair = collision_events.entities[i];

        // Get projectile and target and skip collision if no
        // actual projectile is involved
        EntityID projectile_entity, target_entity;
        bool valid_weapon_hit, projectile_vs_projectile;
        DetermineInflictorAndTarget(collision_entities_pair,
                                    projectile_container.entity_to_data,
                                    projectile_entity,
                                    target_entity,
                                    valid_weapon_hit,
                                    projectile_vs_projectile);

        // Ignore hits without any or between projectiles
        if (valid_weapon_hit && !projectile_vs_projectile)
        {

            // Otherwise accept the collision as a projectile hit and get emitter of projectile
            auto projectile_data_index = projectile_container.entity_to_data[projectile_entity.index];
            auto emitter_entity = projectile_container.emitter_entities[projectile_data_index];

            // Store collision and involved entities
            weapon_collisions.entities.push_back(collision_entities_pair);
            weapon_collisions.relative_positions.push_back(collision_events.relative_positions[i]);
            weapon_collisions.manifolds.push_back(collision_events.manifolds[i]);

            weapon_entities.push_back(projectile_entity);
            target_entities.push_back(target_entity);
            emitter_entities.push_back(emitter_entity);
        }
    }
}


void Logic::ProcessProjectileCollisionEvents(
    CollisionEvents const & collision_events,
    ProjectileContainer & projectile_container,
    IndexedVelocities const & indexed_velocities,
    std::vector<EntityID> & target_entities,
    std::vector<EntityID> & projectiles_that_hit_something,
    std::vector<EntityID> & entities_that_hit_something,
    std::vector<Math::Float3> & impact_velocities,
    CollisionEvents & output_weapon_collisions)
{
    // Prepare the parameters for the registered, valid weapon hits
    WeaponHitParameters hit_parameters;

    // Remove all collisions but those involving a weapon and assign
    // the involved weapon, target and wielder entities
    CollisionEvents weapon_collisions;

    auto old_size = projectiles_that_hit_something.size();
    ExtractValidProjectileCollisions(
        collision_events,
        projectile_container,
        weapon_collisions,
        projectiles_that_hit_something,
        target_entities,
        entities_that_hit_something);

    Append(output_weapon_collisions, weapon_collisions);

    // Reserve parameter entries for all hits
    hit_parameters.directions.resize(weapon_collisions.entities.size());
    hit_parameters.relative_positions.resize(weapon_collisions.entities.size());
    auto new_impact_velocities = Grow(impact_velocities, weapon_collisions.entities.size());

    // Calculate all hit properties
    DetermineHitProperties(
        weapon_collisions,
        CreateRange(projectiles_that_hit_something, old_size, Size(projectiles_that_hit_something)),
        indexed_velocities,
        hit_parameters.directions,
        hit_parameters.relative_positions,
        new_impact_velocities
        );

    // Remove projectile status from all those projectiles that collided
    projectile_container.RemoveProjectiles(projectiles_that_hit_something);
}
