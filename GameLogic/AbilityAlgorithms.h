#pragma once

#include <Conventions\EntityID.h>
#include <Conventions\EntityTemplateID.h>
#include <Conventions\EntityIDGenerator.h>

#include <Math\ForwardDeclarations.h>

#include <vector>

struct ReleaseThrowTimers;

struct IndexedOrientations;
struct IndexedVelocities;
struct IndexedAbsolutePoses;
struct EntitySpawn;


namespace Logic
{
    struct ThrowProperties;
    struct ProjectileContainer;

    // should probably be move elsewhere
    void ProcessThrowerEntities(EntityIDGenerator& entity_id_generator,
                                std::vector<EntityID> const & entities_that_released_a_throw,
                                std::vector<Math::Float3> const & throwing_directions,
                                std::vector<uint32_t> const & entity_to_ammunition, std::vector<EntityTemplateID> const & ammunition, std::vector<ThrowProperties> const & throw_properties,
                                IndexedOrientations const & orientations, IndexedVelocities const & velocities, IndexedAbsolutePoses const & poses,
                                ProjectileContainer & projectile_container,
                                std::vector<EntitySpawn> & output);
}
