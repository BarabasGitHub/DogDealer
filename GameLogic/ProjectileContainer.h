#pragma once

#include <vector>

#include <Conventions\EntityID.h>
#include <Utilities\Range.h>

namespace Logic
{
    struct ProjectileContainer
    {
        // Parallel vectors of projectile and emitting entities.
        // accessed by data index
        std::vector<EntityID> projectile_entities;
        std::vector<EntityID> emitter_entities;

        // Projectile entity index -> data index
        std::vector<unsigned> entity_to_data;

        void AddProjectile(EntityID const projectile_entity, EntityID const emitter_entity);
        void RemoveProjectile(EntityID const projectile_entity);
        void RemoveProjectiles(Range<EntityID const *> projectile_entities);
    };
}