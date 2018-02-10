#include "StateSystem.h"

#include "StateContainer.h"
#include "AnimationBlenderContainer.h"
#include "AnimationTemplates.h"

#include <Utilities\Range.h>
#include <Math\MathFunctions.h>

#include <algorithm> // for sorting in PruneStateInfos

namespace Animating{

	void UpdateAnimationInfos(std::vector<AnimationInfo> & animation_infos, StateContainer & state_container, const float time_step)
	{
        auto& state_logics = state_container.m_state_logics;
        auto& state_datas = state_container.m_state_datas;

		std::vector<uint32_t> expired_states;
		std::vector<uint32_t> faded_states;

		AdvanceStateProgress(state_datas, expired_states, time_step);
        AdvanceStateWeights(state_logics, state_datas, faded_states, time_step);

		// Start fading out non-cyclic expired states at their last frame
		FadeExpiredStates(state_datas, state_logics, expired_states);

        // Remove blended-out states that reached weight 0
		PruneStateInfos(animation_infos, faded_states, state_container);
	}


    // Advance time progress and weight fading for all states
    // Store indices of:
    //		states that are non-cyclic and finished their loop (expired_states)
    void AdvanceStateProgress(std::vector<StateData>& state_datas, std::vector<uint32_t>& expired_states, const float time_step)
    {
        for (auto i = 0u; i < state_datas.size(); i++)
        {
            // Get current state info
            auto& state_data = state_datas[i];

            // ADD PROGRESS
            auto progress = state_data.speed_factor * time_step / state_data.duration;

            state_data.time_progress += progress;

            // Store index if expired
            if (state_data.time_progress >= 1 && !state_data.cyclic)
            {
                state_data.time_progress = 1 - std::numeric_limits<float>::epsilon();
                expired_states.push_back( i );
            }
            else
            {
                // loop animation progress
                state_data.time_progress = std::fmod(state_data.time_progress, 1.0f);
            }
        }
    }


    // Advance time progress and weight fading for all states
    // Store indices of:
    //		states that were fading in their weight and assumed 0 (faded_states)
    void AdvanceStateWeights(std::vector<StateLogic>& state_logics,
                            std::vector<StateData>& state_datas,
                            std::vector<uint32_t>& faded_states,
                            const float time_step)
    {
        for (auto i = 0u; i < state_logics.size(); i++)
        {
            // Get current state logic and data
            auto& state_logic = state_logics[i];
            auto& state_data = state_datas[i];

            // Skip state if already at target value and not faded out
            if ((state_data.total_weight == state_logic.target_weight)
                && (state_data.total_weight != 0.0f)) continue;

            // Otherwise advance weight
            auto weight_step = time_step / state_logic.blend_time;
            state_data.total_weight += weight_step;

            // Check if target weight reached or exceeded
            auto signed_weight = state_data.total_weight * std::copysign(1.f, weight_step);
            auto signed_target = std::copysign(state_logic.target_weight, weight_step);

            if (signed_weight > signed_target)
            {
                // Reaching target value
                // Set state weight to target value
                state_data.total_weight = state_logic.target_weight;

                // If at 0 weight, store index as faded-out
                if (state_logic.target_weight == 0.0f)
                {
                    faded_states.push_back(i);
                }
            }
        }
    }


	// Remove the states indexed
	void PruneStateInfos(Range<AnimationInfo *> animation_infos, Range<uint32_t *> faded_states, StateContainer & state_container)
	{
		// Sort expired states by ascending index
		std::sort(begin(faded_states), end(faded_states));

		// Iterate over animation infos
		for (auto& animation_info : animation_infos)
		{

			// Get start and end indices for state infos used
			auto start_index = animation_info.offset;
			auto end_index = animation_info.offset + animation_info.count - 1;

            // Search for block of states associated to animation info
            // search lower bound
            auto lower = std::lower_bound( begin( faded_states ), end( faded_states ), start_index );

			// find the maximum upper bound
            auto upper = lower + std::min( animation_info.count, uint32_t(end( faded_states ) - lower) );

			// find the exact upper bound
            upper = std::upper_bound( lower, upper, end_index );

			if (lower < upper)
            {
                animation_info = state_container.RemoveStates( animation_info, { lower, upper } );
            }
		}
	}

    // Freeze all non-cyclic frames that expired by reaching their last frame
    // and start fading them out
	void FadeExpiredStates(std::vector<StateData>& state_datas,
                            std::vector<StateLogic>& state_logics,
                            std::vector<uint32_t> const & expired_states )
	{
		// Iterate over expired states
		for (auto index : expired_states)
		{
			// Get state logic and data
            auto& state_logic = state_logics[index];
            auto& state_data = state_datas[index];

			// Freeze at last frame if not cyclic, otherwise continue playing the animation
            if( !state_data.cyclic )
            {
                state_data.speed_factor = 0.0f;
            }

			// Fade out
            state_logic.target_weight = 0.0f;
            state_logic.blend_time = -Math::Abs(state_logic.blend_time);
		}
	}

    // Create potentially redundant states for the animation template.
    // If the template has multiple bone masks, create one state info, data and logic for each of them
    void AddStatesForAllBoneMasks(Animating::AnimationTemplate const & animation_template,
                                    StateContainer & state_container,
                                    AnimationInfo & animation_info,
                                    StateInfo state_info,
                                    StateData state_data,
                                    StateLogic state_logic)
    {
        // Get number of bone masks and their priorities
        auto bone_mask_count = animation_template.priorities.size();

        // Iterate over all bone masks
        for (auto i = 0u; i < bone_mask_count; i++)
        {
            // Store the sequences redundantly
            state_info.offset = state_container.InsertNewSequencesAndWeights(animation_template.sequence_ids);

            // Create the bone mask on the state container
            // and set its offset for the state info
            state_info.bone_weight_offset = state_container.InsertBoneWeights(animation_template.bone_masks[i]);

            // Set the priority for the current bone mask on the state data
            state_data.priority = animation_template.priorities[i];

            // Add the states using the current bone mask for the animation info.
            animation_info = state_container.AddState(animation_info, state_info, state_logic, state_data);
        }
    }


	void AddStates(EntityAnimatingInstructions const & instructions,
                    std::vector<uint32_t> const & entity_to_data,
                    std::vector<AnimationInfo> & animation_infos,
                    StateContainer & state_container,
                    std::vector<AnimationTemplate> const & templates)
	{
		auto size = instructions.entity_ids.size();
		for (auto i = 0u; i < size; ++i)
		{
			// Get instruction
			auto instruction = instructions.instructions[i];
			auto entity_id = instructions.entity_ids[i];

			auto data_index = entity_to_data[entity_id.index];

            auto const & animation_template = templates[instruction.template_id.index];

			StateInfo state_info;
            state_info.count = uint32_t(animation_template.sequence_ids.size());
            state_info.bone_count = uint32_t(animation_template.bone_masks[0].size());

            StateLogic state_logic;
            state_logic.target_weight = 1;
            state_logic.blend_time = animation_template.blend_time;
            state_logic.layer_index = instruction.layer_index;

            StateData state_data;
            state_data.blender = animation_template.blender_id;
            state_data.blend_mode = instruction.blend_mode;
            state_data.total_weight = 0;
            state_data.time_progress = 0;
            state_data.speed_factor = 1;
            state_data.duration = animation_template.duration;
            state_data.cyclic = animation_template.cyclic;

            state_data.external_parameter_type = animation_template.external_parameter_type;
            state_data.external_parameter_factor = animation_template.external_parameter_factor;

            // If necessary, copy these states for all used bone masks and their priorities
            AddStatesForAllBoneMasks(animation_template, state_container, animation_infos[data_index], state_info, state_data, state_logic);
		}
	}

    // For all instructed entities, check whether another animation
    // is already running on the same layer and replace it if necessary.
    // The replaced animation is set to fade out.
    void FadeOutStates(EntityAnimatingInstructions const & instructions,
                        std::vector<uint32_t> const & entity_to_data,
                        std::vector<AnimationInfo> const & animation_infos,
                        std::vector<StateData> const & state_datas,
                        std::vector<AnimationTemplate> const & templates,
                        std::vector<StateLogic> & state_logics)
	{
		auto size = instructions.entity_ids.size();
		for (auto i = 0u; i < size; ++i)
		{
			// Get instruction
			auto instruction = instructions.instructions[i];
            auto entity_id = instructions.entity_ids[i];
            // Get template of animation set by the instruction
            auto instruction_blend_time = templates[instruction.template_id.index].blend_time;

            auto data_index = entity_to_data[entity_id.index];

            // Get animation info and logics for component
            auto animation_info = animation_infos[data_index];

            // Iterate over all current animation states of the instructed entity.
            // Blend out existing animations over the time required
            // by the instructed animation to reach full weight.
            for (auto j = animation_info.offset; j < (animation_info.offset + animation_info.count); j++)
            {
                // Get absolute state index and state logic
                auto & state_logic = state_logics[j];

                // If state is on the right layer
                if (state_logic.layer_index == instruction.layer_index)
                {
                    // Get the total weight of the current state
                    auto const & state_data = state_datas[j];
                    auto total_weight = state_data.total_weight;

                    // Blend out the existing state in the time needed
                    // by the new state to blend in
                    state_logic.target_weight = 0.0f;
                    state_logic.blend_time = -Math::Abs(instruction_blend_time / total_weight);
                }
            }
		}
	}
}

