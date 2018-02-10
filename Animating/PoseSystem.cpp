#pragma once
#include "PoseSystem.h"

#include "StateContainer.h"
#include "SequenceContainer.h"

#include <Math\FloatOperators.h>
#include <Math\TransformFunctions.h>
#include <Math\MathFunctions.h>
#include <Math\FloatMatrixOperators.h>
#include <Math\VectorAlgorithms.h>

#include <Utilities\IndexedHelp.h>

#include <Conventions\OrientationFunctions.h>

#include <functional>

using namespace Animating;




namespace
{

    BoneState BlendBoneStates( BoneState a, BoneState const& b, const float blend_weight )
    {
        a.position = Lerp( a.position, b.position, blend_weight );
        a.rotation = Slerp( a.rotation, b.rotation, blend_weight );
        return a;
    }


    // result = bone1 + bone2 * weight
    BoneState AddBone( BoneState bone1, BoneState const bone2, float const weight )
    {
        // add positions
        bone1.position += bone2.position * weight;
        // before adding the quaternions, we have to check if they are in the same... half of the quaternion space
        auto const rotation_weight = std::copysign( weight, Dot( bone1.rotation, bone2.rotation ) );
        bone1.rotation += bone2.rotation * rotation_weight;
        return bone1;
    }


    void NormalizeBoneRotations( Range<BoneState * > bone_states )
    {
        for( auto& bone_state : bone_states )
        {
            // Normalize bone rotation
            bone_state.rotation = Normalize( bone_state.rotation );
        }
    }


    // multiply each position and rotation with a scalar
    void WeightBoneStates( Range<BoneState *> bone_states, float weight )
    {
        auto float_range = ReinterpretRange<float>( bone_states );
        Math::Multiply( weight, float_range );
    }
}

namespace Animating
{

    void MakePoseAbsolute( Range<BoneState * > bone_states, Range<int const *> parent_indices )
    {
        assert( Size( bone_states ) == Size( parent_indices ) );
        for( unsigned i = 0; i < Size(bone_states); i++ )
        {
            // Get relative parent bone index
            auto parent_index = parent_indices[i];

            // Only consider if valid parent
            if( parent_index != -1 )
            {
                // Get pose bone state
                auto& pose_bone_state = bone_states[i];
                // Get parent bone orientation
                auto const & parent_orientation = bone_states[parent_index];

                // sum transformations
                pose_bone_state = ToParentFromLocal( parent_orientation, pose_bone_state );
            }
        }
    }


    // sequence ids, sequence time progress, sequence weights and sequence bone weights are contigeous and in the same order
    // pose has the size of the number of bones, which corresponds to the number of bones in the sequence bone weights
    // sequence info points to the sequence bone states, which hold the bone states of all sequences
    // does not check for zero sequence weights, or should it?
    // - sequence ids, sequence info and sequence bone states to look up the actual 'sequence'
    // - time progress (per sequence)
    // - weight (per sequence) CURRENTLY NOT IN USE
    // - bone weights (per sequence)
    // - pose (input & output)
    void UpdatePose( Range< SequenceID const *> sequence_ids, Range< SequenceInfo const *> sequence_infos, Range< BoneState const *> sequence_bone_states,
                     Range<float const *> sequence_time_progresses, /*Range<float const *> sequence_weights, */Range<float const *> sequence_bone_weights, Range<BoneState *> pose )
    {
        assert( Size( sequence_ids ) == Size( sequence_time_progresses ) );
        //assert( Size( sequence_time_progresses ) == Size( sequence_weights ) );
        assert( Size( sequence_time_progresses ) * Size( pose ) == Size( sequence_bone_weights ) );

        // first 'reset' bone states of the pose
        WeightBoneStates( pose, c_pose_blending );

        auto bone_count = unsigned( Size( pose ) );

        auto time_progress = begin( sequence_time_progresses );
        //auto sequence_weight = begin( sequence_weights );
        auto sequence_bone_weight = begin( sequence_bone_weights );
        // Iterate over playing animations by type
        for( auto id = begin( sequence_ids ); id != end( sequence_ids ); ++id, /*++sequence_weight,*/ ++time_progress, sequence_bone_weight += bone_count )
        {
            auto sequence_info = sequence_infos[id->index];

            // calculate the progress in frames
            auto frame_progress = *time_progress * (sequence_info.frame_count - 1);

            // get the sequence
            auto keyframe_bone_states = begin( sequence_bone_states ) + sequence_info.frame_offset;

            // Determine the frames and the blend weight between the frames
            float float_frame;
            auto blend_weight = std::modf( frame_progress, &float_frame );
            auto frame_offset1 = unsigned( float_frame );

            // Set to next frame within sequence
            auto frame_offset2 = ( frame_offset1 + 1 ) % sequence_info.frame_count;

            // adjust for the number of bones
            frame_offset1 *= bone_count;
            frame_offset2 *= bone_count;

            // get the bone states of the frames in the sequence
            auto sequence1 = keyframe_bone_states + frame_offset1;
            auto sequence2 = keyframe_bone_states + frame_offset2;

            // get the weight of the sequence
            auto weight = /**sequence_weight * */( 1 - c_pose_blending );

            // sample, blend and add all nonzero weight bones
            for( auto i = 0u; i < bone_count; i++ )
            {
                auto bone_weight = sequence_bone_weight[i];
                if( bone_weight != 0 )
                {
                    auto sequence_bone = BlendBoneStates( sequence1[i], sequence2[i], blend_weight );
                    bone_weight *= weight;
                    pose[i] = AddBone( pose[i], sequence_bone, bone_weight );
                }
            }
        }

        // finally normalize bone rotations of this pose
        NormalizeBoneRotations( pose );
    }


    void UpdatePoses( Range<unsigned const *> sequence_data_indices, Range<SequenceID const *> nonzero_sequences, Range<float const *> sequence_time_progress,
                      Range<unsigned const *> sequence_bone_data_indices, Range<float const *> sequence_bone_weights,
                      Range<SequenceInfo const *> sequence_infos, Range<BoneState const*> sequence_bone_states,
                      Range<unsigned const *> pose_indices, Range<BoneState *> pose_bone_states )
    {
        // for each component
        auto count = Size(pose_indices) - 1;
        for( auto i = 0u; i < count; i++ )
        {
            auto sequences = CreateRange( nonzero_sequences, sequence_data_indices[i], sequence_data_indices[i + 1] );
            if( IsEmpty( sequences ) ) continue;
            auto time_progress = CreateRange( sequence_time_progress, sequence_data_indices[i], sequence_data_indices[i + 1] );
            auto bone_weights = CreateRange( sequence_bone_weights, sequence_bone_data_indices[i], sequence_bone_data_indices[i + 1] );
            auto pose = CreateRange( pose_bone_states, pose_indices[i], pose_indices[i + 1] );
            UpdatePose( sequences, sequence_infos, sequence_bone_states, time_progress, bone_weights, pose );
        }
    }


    void UpdateOffsetPoses( IndexedOffsetPoses& offset_poses, std::vector<unsigned> const & entity_to_pose, std::vector<SkeletonInfo> const & skeleton_infos, std::vector<BoneState> const & skeleton_bone_states, std::vector<unsigned> const& pose_indices, std::vector<BoneState> const & pose_bone_states )
    {
        // Fill in new bone states and pose infos
        auto new_offsets = GetCurrentOffsetPoses( skeleton_infos, skeleton_bone_states, pose_indices, pose_bone_states );

        // Fill in pose infos
        offset_poses.pose_offsets = pose_indices;
        offset_poses.pose_from_entity = entity_to_pose;

        // Check if a previous state can be created
        //bool state_exists = !offset_poses.bone_states.empty();

        // If yes, copy current to previous, else use new state on both
        //if( state_exists )
        if( new_offsets.size() == offset_poses.previous_bone_states.size() )
        {
            // swap here, so we don't have to copy. the bone_states will be replaced with the new_offsets below anyway
            swap(offset_poses.previous_bone_states, offset_poses.bone_states);
        }
        else
        {
            offset_poses.previous_bone_states = new_offsets;
        }

        // Update new current state
        offset_poses.bone_states = std::move( new_offsets );
    }


    BoneStateVector GetCurrentOffsetPoses( std::vector<SkeletonInfo> const & skeleton_infos, std::vector<BoneState> const & skeleton_bone_states, std::vector<unsigned> const & pose_indices, std::vector<BoneState> pose_bone_states )
    {
        assert( pose_indices.size() == skeleton_infos.size() + 1 );
        // Assuming that SkeletonIds and PoseInfos are in sync
        for( auto i = 0u; i < skeleton_infos.size(); i++ )
        {
            // Get SkeletonInfo and PoseInfo
            auto skeleton_info = skeleton_infos[i];

            auto skeleton_begin = begin( skeleton_bone_states ) + skeleton_info.bone_offset;
            auto pose = CreateRange( pose_bone_states, pose_indices[i], pose_indices[i + 1] );
            // Get offsets of all bones [not to confuse with index offsets]
            std::transform( begin( pose ), end( pose ), skeleton_begin, begin( pose ), GetOffset );
        }
        return pose_bone_states;
    }

    void MakePosesAbsolute( std::vector<BoneState>& pose_bone_states, std::vector<unsigned> const & pose_indices, std::vector<SkeletonInfo> const & skeleton_infos, std::vector<int> const & skeleton_parent_indices )
    {
        assert( pose_indices.size() == skeleton_infos.size() + 1 );
        auto count = Size(skeleton_infos);
        for( auto i = 0u; i < count; i++ )
        {
            // Get SkeletonInfo and PoseInfo
            auto skeleton_info = skeleton_infos[i];
            auto start_index = pose_indices[i];
            auto end_index = pose_indices[i + 1];

            auto bone_states = CreateRange( pose_bone_states, start_index, end_index );
            auto parent_indices = CreateRange( skeleton_parent_indices.data() + skeleton_info.bone_offset, skeleton_info.bone_count );

            MakePoseAbsolute( bone_states, parent_indices );
        }
    }
}
