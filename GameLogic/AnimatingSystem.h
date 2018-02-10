#pragma once
#include <conventions/AnimatingInstructions.h>
#include <conventions/Velocity.h>

namespace Logic
{
	struct AnimatingTriggers;
	struct EntityAnimations;
	struct AnimatingStateMachineContainer;

	void GenerateTurningTriggers(IndexedVelocities const & indexed_velocities,
								AnimatingStateMachineContainer & animating_state_machine_container,
								std::vector<EntityID> & now_turning_entities,
								std::vector<EntityID> & no_longer_turning_entities);

	void GenerateAnimatingInstructions(AnimatingTriggers const & animating_triggers, 
									   EntityAnimations const & entity_animations, 
									   AnimatingStateMachineContainer & animating_state_machine_container,
									   EntityAnimatingInstructions & animating_instructions);
}