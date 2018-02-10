#include "AbilityAlgorithms.h"

#include "EntityAbilities.h"
#include "TimerSystem.h"

#include "ProjectileContainer.h"

#include <Conventions\EntitySpawnDescription.h>

#include <Conventions\Orientation.h>
#include <Conventions\Velocity.h>
#include <Conventions\PoseInfo.h>

#include <Conventions\OrientationFunctions.h>

#include <Conventions\EntityIDGenerator.h>

#include <Math\FloatOperators.h>
#include <Math\MathFunctions.h>

namespace Logic
{
    void ProcessThrowerEntities(EntityIDGenerator& entity_id_generator,
                                std::vector<EntityID> const & entities_that_released_a_throw,
                                std::vector<Math::Float3> const & throwing_directions,
                                std::vector<uint32_t> const & entity_to_ammunition, std::vector<EntityTemplateID> const & ammunition, std::vector<ThrowProperties> const & throw_properties,
                                IndexedOrientations const & orientations, IndexedVelocities const & velocities, IndexedAbsolutePoses const & poses,
                                ProjectileContainer & projectile_container,
                                std::vector<EntitySpawn> & output)
    {
        assert( entities_that_released_a_throw.size() == throwing_directions.size() );
        assert( ammunition.size() == throw_properties.size() );

        output.reserve(output.size() + entities_that_released_a_throw.size());

        for (auto i = 0u; i < entities_that_released_a_throw.size(); i++)
        {
            // Otherwise get throwing entity
            auto entity = entities_that_released_a_throw[i];

            assert( entity.index < entity_to_ammunition.size() );
            assert( entity_to_ammunition[entity.index] < ammunition.size() );

            auto const index = entity_to_ammunition[entity.index];

            auto properties = throw_properties[index];

            auto launch_direction = throwing_directions[i];
            launch_direction.z += properties.upward_launch_offset;
            launch_direction = Normalize(launch_direction);
            auto velocity = launch_direction * properties.launch_speed;
            velocity += GetOptionalVelocity( velocities, entity, 0 );

            auto projectile_entity = entity_id_generator.NewID();

            projectile_container.AddProjectile(projectile_entity, entity);

            auto offset = poses.indices[entity.index];
            offset = poses.pose_offsets[offset];
            offset += properties.bone;
            auto & bone_orientation = poses.bone_states[offset];
            auto orientation = orientations.orientations[orientations.indices[entity.index]];
            orientation = ToParentFromLocal( orientation, bone_orientation );
            orientation = ToParentFromLocal( orientation, properties.offset );

            auto const ammo_template_id = ammunition[index];
            output.emplace_back(EntitySpawn{ projectile_entity, ammo_template_id, orientation, velocity });
        }
    }
}