#include "MeleeHitRegistration.h"

#include "AnimatingTriggers.h"
#include "DamageCalculation.h"
//#include "EntityAnimations.h" 
#include "AnimatingTriggers.h"
#include "HitDetectionSystem.h"
#include "PropertyContainer.h"
#include "Structures.h"
#include "TimerSystem.h"
#include "EntityAbilities.h"

#include <Math\MathFunctions.h>
#include <Math\VectorAlgorithms.h>

#include <Utilities\Range.h>
#include <Utilities\StdVectorFunctions.h>

namespace
{
    using namespace Logic;

    // Return true if the weapon entity is used by a wielder who is currently striking
    bool GetWielderAndCheckWhetherStriking(
        EntityWieldingAbilities const & wielding_abilities,
        EntityID const weapon_entity,
        EntityID & wielder_entity)
    {
        // Get wielder and data index from weapon
        auto wielder_data_index = wielding_abilities.wielded_entity_to_data[weapon_entity.index];
        wielder_entity = wielding_abilities.entities[wielder_data_index];

        // Return true if wielder is actually striking
        auto wielder_state = wielding_abilities.wielder_states[wielder_data_index];
        return (wielder_state == WielderStateType::Striking);
    }




    void ExtractValidWeaponCollisions(CollisionEvents const & collision_events,
                                    EntityWieldingAbilities const & wielding_abilities,
                                    CollisionEvents & weapon_collisions,
                                    std::vector<EntityID> & weapon_entities,
                                    std::vector<EntityID> & target_entities,
                                    std::vector<EntityID> & wielder_entities)
    {
        for (auto i = 0u; i < collision_events.manifolds.size(); i++)
        {
            auto collision_entity_pair = collision_events.entities[i];

            // Get weapon and target
            EntityID weapon_entity, target_entity;
            bool valid_weapon_hit, weapon_vs_weapon;
            DetermineInflictorAndTarget(collision_entity_pair,
                                        wielding_abilities.wielded_entity_to_data,
                                        weapon_entity,
                                        target_entity,
                                        valid_weapon_hit,
                                        weapon_vs_weapon);

            // Skip collision if no wielded weapon is involved
            if (!valid_weapon_hit) continue;

            // Ignore hit if wielder is not actually striking
            EntityID wielder_entity;
            bool wielder_is_swinging = GetWielderAndCheckWhetherStriking(wielding_abilities, weapon_entity, wielder_entity);
            if (wielder_is_swinging)
            {
                // Otherwise accept the collision as a weapon hit
                // store collision and involved entities
                weapon_collisions.entities.push_back(collision_entity_pair);
                weapon_collisions.relative_positions.push_back(collision_events.relative_positions[i]);
                weapon_collisions.manifolds.push_back(collision_events.manifolds[i]);

                weapon_entities.push_back(weapon_entity);
                target_entities.push_back(target_entity);
                wielder_entities.push_back(wielder_entity);
            }

            // If weapon vs weapon collision, do the same check with flipped roles,
            // creating two hits if both wielders are striking
            if (weapon_vs_weapon)
            {
                // Ignore hit if the other wielder is not actually striking
                EntityID other_wielder_entity;
                bool other_wielder_is_swinging = GetWielderAndCheckWhetherStriking(wielding_abilities, target_entity, other_wielder_entity);
                if (other_wielder_is_swinging)
                {
                    // Otherwise accept the collision as a weapon hit
                    // store collision and involved entities:
                    weapon_collisions.entities.push_back(collision_entity_pair);
                    weapon_collisions.relative_positions.push_back(collision_events.relative_positions[i]);
                    weapon_collisions.manifolds.push_back(collision_events.manifolds[i]);

                    weapon_entities.push_back(target_entity);
                    target_entities.push_back(weapon_entity);
                    wielder_entities.push_back(other_wielder_entity);
                }
            }
        }
    }
}

namespace Logic
{
    void ProcessWeaponCollisionEvents(CollisionEvents const & collision_events,
                                EntityWieldingAbilities & wielding_abilities,
                                NumericPropertyContainer const & numeric_property_container,
                                //EntityAnimations const & entity_animations,
                                IndexedVelocities const & indexed_velocities,
								AnimatingTriggers & animating_triggers,
                                //EntityAnimatingInstructions & animating_instructions,
                                std::vector<EntityID> & target_entities,
                                std::vector<EntityID> & items_that_hit_something,
                                std::vector<EntityID> & entities_that_hit_something,
                                std::vector<Math::Float3> & impact_velocities,
                                CollisionEvents & output_weapon_collisions)
    {
        // Prepare the parameters for the registered, valid weapon hits
        WeaponHitParameters hit_parameters;

        // Remove all collisions but those involving a weapon and assign
        // the involved weapon, target and wielder entities
        CollisionEvents weapon_collisions;
        ExtractValidWeaponCollisions(
            collision_events,
            wielding_abilities,
            weapon_collisions,
            items_that_hit_something,
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
            items_that_hit_something,
            indexed_velocities,
            hit_parameters.directions,
            hit_parameters.relative_positions,
            new_impact_velocities);

        // SCAMP: Stun hit entities
        for (auto i = 0u; i < weapon_collisions.entities.size(); i++)
        {
            // Get current target
            auto target_entity = target_entities[i];

            // Get hitpoints of target and skip if it has none
            // Skip remaining steps if target is killed by hit
            auto target_health_property_index = GetOptional(numeric_property_container.entity_to_property, target_entity.index);
            auto target_hit_points = GetOptional<float>(numeric_property_container.current_hitpoints, target_health_property_index, 0.f);
            if (target_hit_points > 0)
            {
                // Otherwise set flinching animation for target
                //auto entity_range = CreateRange(&target_entity, 1);
                //AddAnimations(entity_range, entity_animations.animation_template_ids.at(AnimationStateType::Flinch), 1, animating_instructions);
				animating_triggers.struck_entities.push_back(target_entity);

                // Check if target is a wielder and skip remaining steps if not
                auto target_wielder_data_index = GetOptional(wielding_abilities.entity_to_data, target_entity.index);
                if (target_wielder_data_index != c_invalid_index)
                {
                    // Otherwise set wielder to recovering
                    wielding_abilities.wielder_states[target_wielder_data_index] = WielderStateType::Recovering;
                    wielding_abilities.wielder_state_times[target_wielder_data_index] = 0.5f;
                }
            }
        }
    }
}