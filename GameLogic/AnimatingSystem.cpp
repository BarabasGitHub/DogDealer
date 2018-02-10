#include "AnimatingSystem.h"

#include "AnimatingStateMachineContainer.h"
#include "AnimatingTriggers.h"
#include "EntityAnimations.h"

#include <Utilities/VectorHelper.h>

namespace Logic
{
	void GenerateTurningTriggers(IndexedVelocities const & indexed_velocities,
								AnimatingStateMachineContainer & animating_state_machine_container,
								std::vector<EntityID> & now_turning_entities,
								std::vector<EntityID> & no_longer_turning_entities)
	{
		for (auto entity_id : animating_state_machine_container.entity_ids)
		{
			auto velocity_data_index = GetOptional(indexed_velocities.indices, entity_id.index);
			if (velocity_data_index != unsigned(-1))
			{
				auto angular_velocity = indexed_velocities.angular_velocity[velocity_data_index];
		
				auto state_data_index = animating_state_machine_container.entity_to_data[entity_id.index];
				auto was_turning = animating_state_machine_container.entity_is_turning[state_data_index];

				if (was_turning)
				{
					auto is_turning = (std::abs(angular_velocity.z) > 0.01f); 
					if (!is_turning)
					{
						no_longer_turning_entities.push_back(entity_id);
					}
				}
				else
				{
					auto is_turning = (std::abs(angular_velocity.z) > 1.0f);
					if (is_turning)
					{
						now_turning_entities.push_back(entity_id);
					}
				}
			}
		}
	}


	void GenerateAnimatingInstructions(AnimatingTriggers const & animating_triggers, 
									   EntityAnimations const & entity_animations,
									   AnimatingStateMachineContainer & animating_state_machine_container,
									   EntityAnimatingInstructions & animating_instructions)
	{		
		AddAnimations(animating_triggers.throwing_entities, entity_animations.animation_template_ids, AnimationStateType::Throw, 1, animating_instructions );
		AddAnimations(animating_triggers.now_idle_entities, entity_animations.animation_template_ids, AnimationStateType::Idle, 0, animating_instructions );
		AddAnimations(animating_triggers.now_moving_entities, entity_animations.animation_template_ids, AnimationStateType::Motion, 0, animating_instructions );
		AddAnimations(animating_triggers.strike_cancelling_entities, entity_animations.animation_template_ids, AnimationStateType::Idle, 1, animating_instructions );
		AddAnimations(animating_triggers.strike_rebounding_entities, entity_animations.animation_template_ids, AnimationStateType::StrikeRebound, 1, animating_instructions);
		AddAnimations(animating_triggers.struck_entities, entity_animations.animation_template_ids, AnimationStateType::Flinch, 1, animating_instructions);
	
		AddAnimations(animating_triggers.killed_entities, entity_animations.animation_template_ids, AnimationStateType::Dying, 2, animating_instructions);
		AddAnimations(animating_triggers.killed_entities, entity_animations.animation_template_ids, AnimationStateType::Idle, 3, animating_instructions);

		AddAnimations(animating_triggers.died_entities, entity_animations.animation_template_ids, AnimationStateType::Dead, 0, animating_instructions);
		AddAnimations(animating_triggers.died_entities, entity_animations.animation_template_ids, AnimationStateType::Dead, 1, animating_instructions);
	
		// State transitions
		for (auto entity_id : animating_triggers.now_moving_entities)
		{
			auto data_index = animating_state_machine_container.entity_to_data[entity_id.index];
			animating_state_machine_container.entity_is_stationary[data_index] = false;
		}
				
		for (auto entity_id : animating_triggers.now_idle_entities)
		{
			auto data_index = animating_state_machine_container.entity_to_data[entity_id.index];
			animating_state_machine_container.entity_is_stationary[data_index] = true;
		}

		// Output from state machine
		for (auto entity_id : animating_triggers.now_turning_entities)
		{
			auto data_index = animating_state_machine_container.entity_to_data[entity_id.index];
			auto is_stationary = animating_state_machine_container.entity_is_stationary[data_index];
			if (is_stationary)
			{
				auto entity_vector = std::vector<EntityID>(1, entity_id);
				AddAnimations(entity_vector, entity_animations.animation_template_ids.at(AnimationStateType::Turning), 0, animating_instructions);

				animating_state_machine_container.entity_is_turning[data_index] = true;
			}
		}

		for (auto entity_id : animating_triggers.no_longer_turning_entities)
		{
			auto data_index = animating_state_machine_container.entity_to_data[entity_id.index];
			auto is_stationary = animating_state_machine_container.entity_is_stationary[data_index];
			if (is_stationary)
			{
				auto entity_vector = std::vector<EntityID>(1, entity_id);
				AddAnimations(entity_vector, entity_animations.animation_template_ids.at(AnimationStateType::Idle), 0, animating_instructions);

				animating_state_machine_container.entity_is_turning[data_index] = false;
			}
		}
	}
}