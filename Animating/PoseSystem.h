#pragma once
#include <Conventions\PoseInfo.h>
#include <Conventions\EntityID.h>

#include "Structures.h"

#include <Utilities\Range.h>

namespace Animating{

	typedef std::vector<BoneState> BoneStateVector;

	class StateContainer;
	class SequenceContainer;

	const float c_pose_blending = 0.0f;

    void MakePoseAbsolute( Range<BoneState * > bone_states, Range<int const *> parent_indices );

    // sequence ids, sequence time progress, sequence weights and sequence bone weights are contigeous and in the same order
    // pose has the size of the number of bones, which corresponds to the number of bones in the sequence bone weights
    // sequence info points to the sequence bone states, which hold the bone states of all sequences
    // does not check for zero sequence weights, or should it?
    // - sequence ids, sequence info and sequence bone states to look up the actual 'sequence'
    // - time progress (per sequence)
    // - weight (per sequence) CURRENTLY NOT IN USE
    // - bone weights (per sequence)
    // - pose (input & output)
    void UpdatePoses( Range<unsigned const *> sequence_data_indices, Range<SequenceID const *> nonzero_sequences, Range<float const *> sequence_time_progress,
                      Range<unsigned const *> sequence_bone_data_indices, Range<float const *> sequence_bone_weights,
                      Range<SequenceInfo const *> sequence_infos, Range<BoneState const*> sequence_bone_states,
                      Range<unsigned const *> pose_indices, Range<BoneState *> pose_bone_states );

    void AddBoneStates( Range<BoneState const *> source1, Range<BoneState const *> source2, Range<float const*> bone_weights, Range<BoneState *> destination );

    void UpdateOffsetPoses( IndexedOffsetPoses& offset_poses, std::vector<unsigned> const & entity_to_pose, std::vector<SkeletonInfo> const & skeleton_infos, std::vector<BoneState> const & skeleton_bone_states, std::vector<unsigned> const& pose_indices, std::vector<BoneState> const & pose_bone_states );

    BoneStateVector GetCurrentOffsetPoses( std::vector<SkeletonInfo> const & skeleton_infos, std::vector<BoneState> const & skeleton_bone_states, std::vector<unsigned> const & pose_indices, std::vector<BoneState> pose_bone_states );

    void MakePosesAbsolute( std::vector<BoneState>& pose_bone_states, std::vector<unsigned> const & pose_indices, std::vector<SkeletonInfo> const & skeleton_infos, std::vector<int> const & skeleton_parent_indices );
}