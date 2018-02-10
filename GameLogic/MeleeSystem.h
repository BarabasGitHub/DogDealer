#pragma once

#include "EntityAbilities.h"
#include "Structures.h"

#include <Conventions\AnimatingInstructions.h>

#include <Math\FloatTypes.h>

#include <vector>

namespace Logic
{
    // Forward declarations
    struct EntityAnimations;
    struct PlayAnimationTimers;

    // set the holding animations for all entities according to the readying direction
    // sorts the wielder_entities and readying_directions according to the directions
    void SetHoldingAnimations(
        EntityAnimations const & entity_animations,
        Range<EntityID *> wielder_entities,
        Range<WielderDirectionType *> readying_directions,
        EntityAnimatingInstructions & animating_instructions);


    void DetermineReadyingDirection(Math::Float2 const mouse_position,
            AnimationStateType & readying_state_type,
            AnimationStateType & holding_state_type,
            WielderDirectionType & wielding_direction);


    void UpdateWielderData(
        std::vector<EntityID> const & melee_striking_entities,
        std::vector<WielderDirectionType> const & wielding_directions,
        EntityWieldingAbilities & wielding_abilities);


    void SetReadyingAnimations(
        std::vector<EntityID> const & readying_entities,
        std::vector<AnimationStateType> const & readying_animation_types,
        EntityAnimations const & entity_animations,
        EntityAnimatingInstructions & animating_instructions);


    void StartReadyingStrikes(
        std::vector<EntityID> const & melee_striking_entities,
        std::vector<Math::Float2> const & melee_striking_directions,
        EntityAnimations const & entity_animations,
        EntityWieldingAbilities & wielding_abilities,
        EntityAnimatingInstructions & animating_instructions);


    void ReleaseStrikes(
        // input
        std::vector<EntityID> const & triggered_entities,
        EntityAnimations const & entity_animations,
        // in/output
        EntityWieldingAbilities & wielding_abilities,
        EntityAnimatingInstructions & animating_instructions);


    void UpdateMeleeWielderStates(
        float const time_step,
        EntityWieldingAbilities & wielding_abilities,
        std::vector<EntityID> & pre_holding_entities,
        std::vector<WielderDirectionType> & directions);


    void ExtractEntitiesWithWielderState(std::vector<EntityID> const & triggered_entities,
                                    EntityWieldingAbilities const & wielding_abilities,
                                    WielderStateType const valid_wielder_state,
                                    std::vector<EntityID> & valid_entities);

    void ExtractEntitiesWithWielderStates( std::vector<EntityID> const & candidate_entities,
                                   EntityWieldingAbilities const & wielding_abilities,
                                   Range<WielderStateType const *> states,
                                   std::vector<EntityID> & valid_entities );

    void ExtractValidReadyingEntitesAndDirections(
        std::vector<EntityID> const & strike_readying_entities,
        std::vector<Math::Float2> const & strike_readying_directions,
        EntityWieldingAbilities const & wielding_abilities,
        std::vector<EntityID> & readying_entities,
        std::vector<Math::Float2> & readying_directions);


    void CancelStrikes(std::vector<EntityID> const & cancelling_entities, EntityWieldingAbilities & wielding_abilities );

    void StartBlocking(std::vector<EntityID> const & blocking_entities,
        Math::Float2 const target_direction,
        EntityAnimations const & entity_animations,
        EntityWieldingAbilities & wielding_abilities,
        EntityAnimatingInstructions & animating_instructions);

    void EnforceMeleeTriggerConditions(EntityWieldingAbilities const & wielding_abilities,
                                    MeleeActionTriggers & melee_action_triggers);

    void ExecuteMeleeTriggerActions(MeleeActionTriggers const & melee_action_triggers,
                                    std::vector<EntityID> const & melee_cancelling_entities,
                                    EntityAnimations const & entity_animations,
                                    Math::Float2 const mouse_position,
                                    EntityWieldingAbilities & wielding_abilities,
                                    EntityAnimatingInstructions & animating_instructions);
}