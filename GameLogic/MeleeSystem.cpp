#include "MeleeSystem.h"

#include "TimerSystem.h"
#include "Controllers.h"


//#include "EntityAbilities.h"
#include "EntityAnimations.h"

//#include <Conventions\AnimatingInstructions.h>
#include <Conventions\EntityID.h>

#include <Utilities\VectorHelper.h>
#include <Utilities\ParallelIterator.h>

namespace Logic{

    // Use the Float2 target direction to determine a resulting animation type for
    // the readying and holding animations as well as the eventual striking direction
    // to be played by the respective entity
    // TODO: SOMETHING
    void DetermineReadyingDirections(std::vector<Math::Float2> const & target_directions,
                                    std::vector<AnimationStateType> & readying_state_types,
                                    std::vector<WielderDirectionType> & wielding_directions)
    {
        for (auto i = 0u; i < target_directions.size(); i++)
        {
            // Get current target direction
            auto target_direction = target_directions[i];

            // Get resulting entries
            auto& readying_state_type = readying_state_types[i];
            auto& wielding_direction = wielding_directions[i];

            if (std::abs(target_direction.x) > std::abs(target_direction.y))
            {
                if (target_direction.x <= 0)
                {
                    readying_state_type = AnimationStateType::ReadyStrikeRight;
                    wielding_direction = WielderDirectionType::Right;
                }
                if (target_direction.x > 0)
                {
                    readying_state_type = AnimationStateType::ReadyStrikeLeft;
                    wielding_direction = WielderDirectionType::Left;
                }
            }
            else
            {
                if (target_direction.y <= 0)
                {
                    readying_state_type = AnimationStateType::ReadyStrikeDown;
                    wielding_direction = WielderDirectionType::Down;
                }
                if (target_direction.y > 0)
                {
                    readying_state_type = AnimationStateType::ReadyStrikeUp;
                    wielding_direction = WielderDirectionType::Up;
                }
            }
        }
    }

    // Set the previously determined wielding direction and update the wielder state
    // for all the input wielder data indices, assuming the two vectors are in parallel
    void UpdateWielderData(
        std::vector<EntityID> const & melee_striking_entities,
        std::vector<WielderDirectionType> const & wielding_directions,
        EntityWieldingAbilities & wielding_abilities)
    {
        // Loop over all valid readying entity entries
        for (auto i = 0u; i < melee_striking_entities.size(); i++)
        {
            // Get wielder data index and direction
            auto entity_id = melee_striking_entities[i];
            auto wielder_data_index = wielding_abilities.entity_to_data[entity_id.index];
            auto wielding_direction = wielding_directions[i];

            wielding_abilities.wielder_direction[wielder_data_index] = wielding_direction;
            wielding_abilities.wielder_states[wielder_data_index] = WielderStateType::Readying;
            wielding_abilities.wielder_state_times[wielder_data_index] = 0.3f;
        }
    }


    void SetReadyingAnimations(
        std::vector<EntityID> const & readying_entities,
        std::vector<AnimationStateType> const & readying_animation_types,
        EntityAnimations const & entity_animations,
        EntityAnimatingInstructions & animating_instructions)
    {
         // Get count of resulting instructions
        auto previous_instruction_count = animating_instructions.entity_ids.size();
        auto result_instruction_count = previous_instruction_count + readying_entities.size();

        // Reserve instruction entries
        animating_instructions.instructions.resize(result_instruction_count);

        // Store all readying entities as instruction targets
        animating_instructions.entity_ids.insert(animating_instructions.entity_ids.end(), readying_entities.begin(), readying_entities.end());

        // Create individual instructions
        for (auto i = 0u; i < readying_entities.size(); i++)
        {
            // Get instruction and target entity
            auto instruction_index = i + previous_instruction_count;
            auto& instruction = animating_instructions.instructions[instruction_index];
            auto entity = readying_entities[i];

            // Get animation corresponding to readying animation type
            auto animation_type = readying_animation_types[i];
            auto& animation_templates = entity_animations.animation_template_ids.at(animation_type);

            // Set right animation from entity templates for instruction on layer 1
            instruction.template_id = animation_templates.at(entity);
            instruction.layer_index = 1;
        }

        assert(animating_instructions.entity_ids.size() == animating_instructions.instructions.size());
    }


    void StartReadyingStrikes(
        std::vector<EntityID> const & melee_striking_entities,
        std::vector<Math::Float2> const & melee_striking_directions,
        EntityAnimations const & entity_animations,
        EntityWieldingAbilities & wielding_abilities,
        EntityAnimatingInstructions & animating_instructions)
    {
        assert( melee_striking_directions.size() == melee_striking_entities.size() );
        // Determine readying, holding and striking directions from target direction
        std::vector<AnimationStateType> readying_animation_types(melee_striking_entities.size());
        std::vector<WielderDirectionType> wielding_directions(melee_striking_entities.size());

        DetermineReadyingDirections(melee_striking_directions, readying_animation_types, wielding_directions);

        // Set the striking direction and update the wielder state to 'striking'
        UpdateWielderData(melee_striking_entities, wielding_directions, wielding_abilities);

        // Set appropriate readying animation for all striking entities
        SetReadyingAnimations(melee_striking_entities, readying_animation_types, entity_animations, animating_instructions);
    }


    void ReleaseStrikes(std::vector<EntityID> const & triggered_entities,
                        EntityAnimations const & entity_animations,
                        EntityWieldingAbilities & wielding_abilities,
                        EntityAnimatingInstructions & animating_instructions )
    {
        // Loop over all wielder entities
        for (auto i = 0u; i < triggered_entities.size(); i++)
        {
            // Entity and used data
            auto entity_id = triggered_entities[i];
            auto wielder_data_index = GetOptional(wielding_abilities.entity_to_data, entity_id.index);
            // make sure we only process valid entities, that is entities who are holding
            assert(GetOptional<WielderStateType>(wielding_abilities.wielder_states, wielder_data_index, WielderStateType::Striking) == WielderStateType::Holding);

            wielding_abilities.wielder_states[wielder_data_index] = WielderStateType::Striking;
            wielding_abilities.wielder_state_times[wielder_data_index] = 0.7f;

            // Get correct animation according to attack direction
            auto direction = wielding_abilities.wielder_direction[wielder_data_index];
            AnimationStateType strike_animation;
            if (direction == WielderDirectionType::Left) strike_animation = AnimationStateType::StrikeLeft;
            else if (direction == WielderDirectionType::Right) strike_animation = AnimationStateType::StrikeRight;
            else if (direction == WielderDirectionType::Up) strike_animation = AnimationStateType::StrikeUp;
            else if (direction == WielderDirectionType::Down) strike_animation = AnimationStateType::StrikeDown;

            // Set correct striking animation for entity
            auto entity_range = CreateRange(&entity_id, 1);
            AddAnimations(entity_range, entity_animations.animation_template_ids.at(strike_animation), 1, animating_instructions);
        }
    }

    // set the holding animations for all entities according to the readying direction
    // sorts the wielder_entities and readying_directions according to the directions
    void SetHoldingAnimations(
        EntityAnimations const & entity_animations,
        Range<EntityID *> wielder_entities,
        Range<WielderDirectionType *> readying_directions,
        EntityAnimatingInstructions & animating_instructions)
    {
        // sort entities and directions according to the directions
        std::sort(
            CreateParallelIterator(begin(wielder_entities), begin(readying_directions)),
            CreateParallelIterator(end(wielder_entities), end(readying_directions)),
            [](auto a, auto b){ return std::get<1>(a) < std::get<1>(b); }
            );

        auto const c_number_of_directions = 4;
        WielderDirectionType direction_types[4] = {WielderDirectionType::Left, WielderDirectionType::Right, WielderDirectionType::Up, WielderDirectionType::Down};
        AnimationStateType animation_types[4] = {AnimationStateType::HoldStrikeLeft, AnimationStateType::HoldStrikeRight, AnimationStateType::HoldStrikeUp, AnimationStateType::HoldStrikeDown};

        for (auto i = 0u; i < c_number_of_directions; ++i)
        {
            auto equal_range = std::equal_range(begin(readying_directions), end(readying_directions), direction_types[i]);
            auto entity_range = CreateRange(wielder_entities, equal_range.first - begin(readying_directions), equal_range.second - begin(readying_directions));
            AddAnimations(entity_range, entity_animations.animation_template_ids, animation_types[i], 1, animating_instructions);
        }
    }


    void UpdateMeleeWielderStates(
        float const time_step,
        EntityWieldingAbilities & wielding_abilities,
        std::vector<EntityID> & pre_holding_entities,
        std::vector<WielderDirectionType> & directions)
    {
        // Update remaining times for all states and find expired timers
        for (auto i = 0u; i < wielding_abilities.wielder_state_times.size(); i++)
        {
            // Get remaining time for state of current wielder
            auto& remaining_time = wielding_abilities.wielder_state_times[i];

            // Skip if inactive
            if (remaining_time == -std::numeric_limits<float>::max()) continue;

            // Otherwise subtract time step
            remaining_time -= time_step;

            // If expired, set to inactive and store index
            if (remaining_time <= 0)
            {
                auto& wielder_state = wielding_abilities.wielder_states[i];
                switch(wielder_state)
                {
                    case WielderStateType::Readying:
                    {
                        // Set state to holding and set holding animation
                        wielder_state = WielderStateType::PreHolding;
                        remaining_time = 0.2f;
                        directions.push_back(wielding_abilities.wielder_direction[i]);
                        pre_holding_entities.push_back(wielding_abilities.entities[i]);
                        break;
                    }
                    case WielderStateType::PreHolding:
                        wielder_state = WielderStateType::Holding;
                        remaining_time = -std::numeric_limits<float>::max();
                        break;
                    case WielderStateType::Striking:
                        wielder_state = WielderStateType::Idle;
                        remaining_time = -std::numeric_limits<float>::max();
                        break;
                    case WielderStateType::Recovering:
                        wielder_state = WielderStateType::Idle;
                        remaining_time = -std::numeric_limits<float>::max();
                        break;
                }
            }
        }
    }


    void ExtractEntitiesWithWielderStates( std::vector<EntityID> const & candidate_entities,
                                   EntityWieldingAbilities const & wielding_abilities,
                                   Range<WielderStateType const *> states,
                                   std::vector<EntityID> & valid_entities )
    {
        auto old_size = valid_entities.size();
        valid_entities.resize( old_size + candidate_entities.size() );
        auto last = std::copy_if(begin(candidate_entities), end(candidate_entities), begin(valid_entities) + old_size,
                      [&]( EntityID entity_id )
        {
            auto wielder_data_index = GetOptional( wielding_abilities.entity_to_data, entity_id.index );
            auto wielder_state = GetOptional<WielderStateType>( wielding_abilities.wielder_states, wielder_data_index, WielderStateType::Invalid);
            return std::any_of(begin(states), end(states), [wielder_state](WielderStateType s){return s == wielder_state;});
        } );
        valid_entities.resize( last - begin( valid_entities ) );
    }

    // Remove all those entities from the triggered entity input vector
    // that are not in the specified wielder state
    void ExtractEntitiesWithWielderState(std::vector<EntityID> const & triggered_entities,
        EntityWieldingAbilities const & wielding_abilities,
        WielderStateType const valid_wielder_state,
        std::vector<EntityID> & valid_entities
        )
    {
        auto old_size = valid_entities.size();
        valid_entities.resize(old_size + triggered_entities.size());
        auto last = std::copy_if(begin(triggered_entities), end(triggered_entities), begin(valid_entities) + old_size,
            [&](EntityID entity_id)
        {
            auto wielder_data_index = GetOptional(wielding_abilities.entity_to_data, entity_id.index);
            auto wielder_state = GetOptional<WielderStateType>(wielding_abilities.wielder_states, wielder_data_index, WielderStateType::Invalid);

            // Only keep entities that are in the desired state
            return wielder_state == valid_wielder_state;
        });
        valid_entities.resize(last - begin(valid_entities));
    }


    void ExtractValidReadyingEntitesAndDirections(
        std::vector<EntityID> const & strike_readying_entities,
        std::vector<Math::Float2> const & strike_readying_directions,
        EntityWieldingAbilities const & wielding_abilities,
        std::vector<EntityID> & readying_entities,
        std::vector<Math::Float2> & readying_directions)
    {
        assert(strike_readying_entities.size() == strike_readying_directions.size());
        // Remove those entities that are either striking already or have no wielding ability
        for (auto i = 0u; i < strike_readying_entities.size(); i++)
        {
            auto triggered_entity = strike_readying_entities[i];
            auto wielder_data_index = GetOptional(wielding_abilities.entity_to_data, triggered_entity.index);
            // Skip if entity is already striking, holding or readying
            auto wielding_state = GetOptional<WielderStateType>(wielding_abilities.wielder_states, wielder_data_index, WielderStateType::Invalid);
            if (wielding_state == WielderStateType::Idle)
            {
                // Store entity as valid readying entity
                readying_entities.push_back(triggered_entity);
                readying_directions.push_back(strike_readying_directions[i]);
            }
        }
    }


    void CancelStrikes(
        std::vector<EntityID> const & cancelling_entities,
        EntityWieldingAbilities & wielding_abilities)
    {
        // Reset all wielder states and timers for the cancelling entities
        for (auto i = 0u; i < cancelling_entities.size(); i++)
        {
            // Get entity and wielder data index
            auto entity_id = cancelling_entities[i];
            auto wielder_data_index = wielding_abilities.entity_to_data[entity_id.index];

            // Do not allow cancelling if wielder is mid-swing
            assert( wielding_abilities.wielder_states[wielder_data_index] != WielderStateType::Striking );

            // Set wielder state to idle and reset timer
            wielding_abilities.wielder_states[wielder_data_index] = WielderStateType::Idle;
            wielding_abilities.wielder_state_times[wielder_data_index] = std::numeric_limits<float>::lowest();
        }
    }

    // Set a blocking animation and wielder state for all input entities
    // TODO: Get individual directions for all entities
    void StartBlocking(std::vector<EntityID> const & blocking_entities,
                        Math::Float2 const target_direction,
                        EntityAnimations const & entity_animations,
                        EntityWieldingAbilities & wielding_abilities,
                        EntityAnimatingInstructions & animating_instructions)
    {
        // Get the right wielder direction and animation from the Float2 direction
        WielderDirectionType wielding_direction = WielderDirectionType::Up;;
        AnimationStateType blocking_state_type = AnimationStateType::BlockUp;

        // Now its getting ugly
        if (std::abs(target_direction.x) > std::abs(target_direction.y))
        {
            if (target_direction.x <= 0)
            {
                blocking_state_type = AnimationStateType::BlockLeft;
                wielding_direction = WielderDirectionType::Left;
            }
            else //if (target_direction.x > 0)
            {
                blocking_state_type = AnimationStateType::BlockRight;
                wielding_direction = WielderDirectionType::Right;
            }
        }
        else
        {
            if (target_direction.y <= 0)
            {
                blocking_state_type = AnimationStateType::BlockUp;
                wielding_direction = WielderDirectionType::Up;
            }
            else //if (target_direction.y > 0)
            {
                blocking_state_type = AnimationStateType::BlockDown;
                wielding_direction = WielderDirectionType::Down;
            }
        }

        // Set the direction and animation for all triggered entities
        for (auto i = 0u; i < blocking_entities.size(); i++)
        {
            // Get entity
            auto& entity_id = blocking_entities[i];

            // Get wielder data index
            auto wielder_data_index = GetOptional(wielding_abilities.entity_to_data, entity_id.index);
            assert(wielder_data_index != c_invalid_index);

            // Set wielder state and reset state time
            wielding_abilities.wielder_states[wielder_data_index] = WielderStateType::Blocking;
            wielding_abilities.wielder_state_times[wielder_data_index] = -std::numeric_limits<float>::max();

            // Set wielding direction
            wielding_abilities.wielder_direction[wielder_data_index] = wielding_direction;

            // Set new animation
            auto entity_range = CreateRange(&entity_id, 1);
            AddAnimations(entity_range, entity_animations.animation_template_ids.at(blocking_state_type), 1, animating_instructions);
        }
    } 

    // Prune all those entities from the triggered entity vectors
    // which can not perform the triggered action
    void EnforceMeleeTriggerConditions(EntityWieldingAbilities const & wielding_abilities,
									   MeleeActionTriggers & melee_action_triggers)
    {
        std::vector<EntityID> valid_strike_readying_entities;
        std::vector<Math::Float2> valid_strike_readying_directions;
        ExtractValidReadyingEntitesAndDirections(
            melee_action_triggers.strike_readying_entities,
            melee_action_triggers.strike_readying_directions,
            wielding_abilities,
            valid_strike_readying_entities,
            valid_strike_readying_directions);

        // TODO: Double check filtering of directions
        // Only allow entities to cancel a strike
        // if they are not already striking, blocking, idle or recovering
        std::vector<EntityID> valid_strike_cancelling_entities;
        std::array<WielderStateType, 3> valid_cancel_states = {{WielderStateType::Readying, WielderStateType::PreHolding, WielderStateType::Holding }};
        ExtractEntitiesWithWielderStates(
            melee_action_triggers.strike_cancelling_entities,
            wielding_abilities,
            valid_cancel_states,
            valid_strike_cancelling_entities);

        // Only allow entities to cancel a block
        // if they are currently holding one
        std::vector<EntityID> valid_block_cancelling_entities;
        ExtractEntitiesWithWielderState(
            melee_action_triggers.block_cancelling_entities,
            wielding_abilities,
            WielderStateType::Blocking,
            valid_block_cancelling_entities);

        // Only allow entities to start a weapon block
        // if they are currently idle
        std::vector<EntityID> valid_block_starting_entities;
        ExtractEntitiesWithWielderState(
            melee_action_triggers.block_starting_entities,
            wielding_abilities,
            WielderStateType::Idle,
            valid_block_starting_entities);

        // Only allow entities to start releasing their strike
        // if they are currently holding
        std::vector<EntityID> valid_strike_releasing_entities;
        ExtractEntitiesWithWielderState(
            melee_action_triggers.strike_releasing_entities,
            wielding_abilities,
            WielderStateType::Holding,
            valid_strike_releasing_entities
            );

        // Overwrite the triggered vectors with those meeting the conditions
        melee_action_triggers.strike_readying_entities = std::move(valid_strike_readying_entities);
        melee_action_triggers.strike_readying_directions = std::move(valid_strike_readying_directions);
        melee_action_triggers.strike_cancelling_entities = std::move(valid_strike_cancelling_entities);
        melee_action_triggers.block_cancelling_entities = std::move(valid_block_cancelling_entities);
        melee_action_triggers.block_starting_entities = std::move(valid_block_starting_entities);
        melee_action_triggers.strike_releasing_entities = std::move(valid_strike_releasing_entities);
    }

    // For all triggered input entities, run the desired action
    void ExecuteMeleeTriggerActions(MeleeActionTriggers const & melee_action_triggers,
                                    std::vector<EntityID> const & melee_cancelling_entities,
                                    EntityAnimations const & entity_animations,
                                    Math::Float2 const mouse_position,
                                    EntityWieldingAbilities & wielding_abilities,
                                    EntityAnimatingInstructions & animating_instructions)
    {
        CancelStrikes(melee_cancelling_entities, wielding_abilities);
        StartBlocking(melee_action_triggers.block_starting_entities, mouse_position, entity_animations, wielding_abilities, animating_instructions);
        ReleaseStrikes(melee_action_triggers.strike_releasing_entities, entity_animations, wielding_abilities, animating_instructions);
        StartReadyingStrikes(
            melee_action_triggers.strike_readying_entities,
            melee_action_triggers.strike_readying_directions,
            entity_animations,
            // in/out
            wielding_abilities,
            // out
            animating_instructions);
    }

}

