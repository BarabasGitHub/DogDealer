#pragma once
#include "EntityAbilities.h"

#include <Conventions\Orientation.h>
#include <Conventions\Velocity.h>
#include <Conventions\Force.h>

#include <Conventions\CollisionEvent.h>
//#include <Conventions\AnimatingInstructions.h>

#include <Conventions\EntityIDGenerator.h>
#include <Conventions\EntitySpawnDescription.h> // Currently for hit visualization


namespace Logic
{
    // Forward declarations
    struct NumericPropertyContainer;
    struct EntityAnimations;
    struct PlayAnimationTimers;
	struct AnimatingTriggers;

    void ProcessWeaponCollisionEvents(
        CollisionEvents const & collision_events,
        EntityWieldingAbilities & wielding_abilities,
        NumericPropertyContainer const & numeric_property_container,
        //EntityAnimations const & entity_animations,
        IndexedVelocities const & indexed_velocities,
        //EntityAnimatingInstructions & animating_instructions,
		AnimatingTriggers & entity_animating_triggers,
        std::vector<EntityID> & target_entities,
        std::vector<EntityID> & items_that_hit_something,
        std::vector<EntityID> & entities_that_hit_something,
        std::vector<Math::Float3> & impact_velocities,
        CollisionEvents & output_weapon_collisions);
}