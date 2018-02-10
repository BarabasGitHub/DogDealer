#pragma once

#include "Structures.h"

#include <Utilities\Range.h>

namespace Animating
{
	class StateContainer
        {

        // Map of gap size to gap position index
        std::multimap<uint32_t, uint32_t> m_sequence_gaps;
        std::multimap<uint32_t, uint32_t> m_state_gaps;
        std::multimap<uint32_t, uint32_t> m_bone_mask_gaps;

	public:
        // Currently played animation types and weights
        std::vector<SequenceID> m_sequence_ids;
        std::vector<float> m_sequence_weights;

        std::vector<float> m_bone_masks;

        std::vector<StateInfo> m_state_infos;
        std::vector<StateLogic> m_state_logics;
        std::vector<StateData> m_state_datas;

        static AnimationInfo CreateAnimationInfo();

        void RemoveAnimationInfo( AnimationInfo info );

        // returns the offset, also makes room for the weights
        uint32_t InsertNewSequencesAndWeights( Range<SequenceID const *> sequences );

        // returns the offset
        uint32_t InsertNewSequence( SequenceID sequence, float weight );

        // inserts new bone weights and returns the offset
        uint32_t InsertBoneWeights( Range<float const *> bone_weights );

        // adds a state to an animation
        AnimationInfo AddState( AnimationInfo animation_info, StateInfo state_info, StateLogic state_logic, StateData state_data );

        // removes the states from the container, including all the sequence ids, weights and bone weights
        void RemoveStates( uint32_t offset, uint32_t count );

        AnimationInfo RemoveStates( AnimationInfo animation_info, Range<uint32_t const *> absolute_indices );

        void SortStates( uint32_t offset, uint32_t count );

        void MergeGaps();

	private:

        void ClearStates( uint32_t const offset, uint32_t const count );
        void ClearState(StateInfo const & state_info);
	};
}