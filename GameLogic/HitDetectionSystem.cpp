#include "HitDetectionSystem.h"

#include "PropertyContainer.h"
#include "EntityAnimations.h"
#include "TimerSystem.h"

#include "Structures.h"

#include <Math\MathFunctions.h>
#include <Math\MathToString.h>
#include <Math\VectorAlgorithms.h>

#include <Utilities\VectorHelper.h>
#include <Utilities\StdVectorFunctions.h>
#include <Utilities\StringUtilities.h>
#include <Utilities\Logger.h>


namespace Logic{

    // Set valid_hit to true if the input collision involves at least one entity
    // which has a valid entry in inflictor_data_indices.
    // If both entities have a valid entry, set inflictor_vs_inflictor to true.
    void DetermineInflictorAndTarget(EntityPair const collision_entities,
                                std::vector<unsigned> const & inflictor_data_indices,
                                EntityID & inflicting_entity,
                                EntityID & target_entity,
                                bool & valid_hit,
                                bool & inflictor_vs_inflictor)
    {
        // Get inflictor data indices for both entities
        auto data_index1 = GetOptional(inflictor_data_indices, collision_entities.id1.index);
        auto data_index2 = GetOptional(inflictor_data_indices, collision_entities.id2.index);

        auto entity1_is_inflicter = data_index1 != c_invalid_index;
        auto entity2_is_inflicter = data_index2 != c_invalid_index;

        // hit is valid if there is at least one inflicter involved
        valid_hit = entity1_is_inflicter | entity2_is_inflicter;
        inflictor_vs_inflictor = entity1_is_inflicter & entity2_is_inflicter;

        if( entity1_is_inflicter )
        {
            target_entity = collision_entities.id2;
            inflicting_entity = collision_entities.id1;
        }
        else if( entity2_is_inflicter )
        {
            target_entity = collision_entities.id1;
            inflicting_entity = collision_entities.id2;
        }
    }

    // For all input collisions and their parallel weapon entities, calculate
    // the hit direction, relative position and velocity based on the weapon physics
    void DetermineHitProperties(
        CollisionEvents const & collisions,
        Range<EntityID const *> weapon_entities,
        IndexedVelocities const & indexed_velocities,
        Range<Math::Float3 *> hit_directions,
        Range<Math::Float3 *> relative_hit_positions,
        Range<Velocity *> hit_velocities)
    {
        assert(Size(hit_velocities) == Size(collisions.entities));
        assert(Size(hit_velocities) == Size(collisions.relative_positions));
        assert(Size(hit_velocities) == Size(collisions.manifolds));
        assert(Size(hit_velocities) == Size(hit_directions));
        assert(Size(hit_velocities) == Size(relative_hit_positions));

        // Iterate over all collisions
        for (auto i = 0u; i < collisions.entities.size(); i++)
        {
            // Get current collision and involved weapon
            auto const collision_entities = collisions.entities[i];
            auto const & manifold = collisions.manifolds[i];
            auto const weapon_entity = weapon_entities[i];


            // Calculate hit direction and position,
            // assuming that second entity is the target
            auto direction = GetAverageSeparationDirection(manifold);
            auto relative_position = GetAveragePosition(manifold);

            // If the second entity is not the target, the relative
            // hit detection and direction have to be inverted
            if (collision_entities.id2 == weapon_entity)
            {
                direction = -direction;
                relative_position -= collisions.relative_positions[i];
            }

            hit_directions[i] = direction;
            relative_hit_positions[i] = relative_position;
        }


        for( auto i = 0u; i < Size(weapon_entities); ++i )
        {
            auto const weapon_entity = weapon_entities[i];
            auto const relative_position = relative_hit_positions[i];

            // Calculate weapon velocity
            auto velocity = CalculateVelocityAtPosition(indexed_velocities, weapon_entity, relative_position);

            hit_velocities[i] = velocity;

            Log([velocity, relative_position]()
            {
                return "Hit velocity at relative position " + ToString(relative_position) + " is " + ToString(velocity);
            });
        }
    }

    // Spawn an axis model at the hit location
    // TODO: Un-hardcode entity template etc
    void DebugSpawnHitVisualization(IndexedOrientations const & indexed_orientations,
        EntityID const weapon_entity,
        Math::Float3 const relative_position,
        Velocity const velocity,
        EntityIDGenerator & entity_id_generator,
        std::vector<EntitySpawn>& entities_to_be_spawned)
    {
        // Get absolute position of hit
        auto weapon_orientation_index = indexed_orientations.indices[weapon_entity.index];
        auto weapon_orientation = indexed_orientations.orientations[weapon_orientation_index];

        auto hit_direction = Math::RotationBetweenAxes(Math::Float3(0.0f, 0.0f, 1.0f), Normalize(velocity));

        Orientation spawn_orientation = Math::Identity();
        spawn_orientation.position = weapon_orientation.position + relative_position;
        spawn_orientation.rotation = hit_direction;

        EntitySpawn spawn_instruction;
        spawn_instruction.orientation = spawn_orientation;
        spawn_instruction.velocity = 0.0f;

        spawn_instruction.template_id.index = 0; // hardcoded stuff business
        spawn_instruction.entity_id = entity_id_generator.NewID();

        entities_to_be_spawned.push_back(spawn_instruction);
    }
}
