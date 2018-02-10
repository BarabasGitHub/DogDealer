#pragma once
#include "AnimatingWorld.h"

#include "PoseSystem.h"
#include "StateSystem.h"

//#include "CircleBlenderSystem.h"
#include "ExternalParameterSystem.h"
#include "BlendingSystem.h"

#include <Utilities\VectorHelper.h>
#include <Utilities\IndexedHelp.h>

#include <cassert>

using namespace Animating;

AnimatingWorld::AnimatingWorld()
{
}


AnimatingWorld::~AnimatingWorld()
{
}


SequenceID AnimatingWorld::ProvideSequence( std::string const & sequence_name )
{
    return m_resource_manager.ProvideSequence( sequence_name, m_keyframe_container );
}


SequenceID AnimatingWorld::ProvideSequence( std::string const & sequence_name, uint32_t & frame_count )
{
    auto id = m_resource_manager.ProvideSequence( sequence_name, m_keyframe_container );
    frame_count = m_keyframe_container.m_sequence_infos[id.index].frame_count;
    return id;
}


AnimationBlenderID AnimatingWorld::ProvideAnimationBlender(CircleBlenderDescription const & blender_description)
{
    return m_resource_manager.ProvideAnimationBlender( blender_description, m_blender_container );
}



void AnimatingWorld::CreateAnimatingComponent(std::string const & skeleton, EntityID entity_id )
{
	m_resource_manager.CreateComponent(skeleton, entity_id, m_component_container, m_skeleton_container );
}


void AnimatingWorld::RemoveEntities( Range<EntityID const *> entities )
{
    for( auto e : entities )
    {
        if( e.index < m_component_container.infos.size() )
        {
            m_state_container.RemoveAnimationInfo( m_component_container.infos[e.index] );
        }
    }

    m_component_container.RemoveEntities( entities );

    {
        auto to_be_removed = RemoveIndices( m_indexed_offset_poses.pose_from_entity, entities );
        RemoveEntries( m_indexed_offset_poses.previous_bone_states, m_indexed_offset_poses.pose_offsets, to_be_removed );
        RemoveEntries( m_indexed_offset_poses.bone_states, m_indexed_offset_poses.pose_offsets, to_be_removed );
        RemoveOffsets( m_indexed_offset_poses.pose_offsets, to_be_removed );
    }
    {
        auto to_be_removed = RemoveIndices( m_indexed_absolute_poses.indices, entities );
        RemoveEntries( m_indexed_absolute_poses.bone_states, m_indexed_absolute_poses.pose_offsets, to_be_removed );
        RemoveOffsets( m_indexed_absolute_poses.pose_offsets, to_be_removed );
    }
}


// Use the input data to sample circle blenders and modify animation speeds
void AnimatingWorld::UpdateStatesWithExternalParameters(IndexedOrientations const & indexed_orientations,
                                                    IndexedVelocities const & indexed_velocities, 
                                                    Math::Float3 const camera_angles)
{
    UpdateCircleBlendersAndSpeedModifiers(m_blender_container, 
                                        indexed_orientations, 
                                        indexed_velocities, 
                                        camera_angles,
                                        m_component_container.entity_ids,
                                        m_component_container.infos,
                                        m_state_container.m_state_infos,
                                        m_state_container.m_state_datas,
                                        m_state_container.m_sequence_weights);
}


void AnimatingWorld::UpdateAnimations(const float time_step)
{
    // DEBUG:
    //DebugLogStateWeightsForEntity(2, m_component_container, m_state_container.m_state_infos, m_state_container.m_state_datas, m_state_container.m_state_logics, m_state_container.m_sequence_ids);

    UpdateAnimationInfos( m_component_container.infos, m_state_container, time_step );

    std::vector<unsigned> sequence_data_indices( m_component_container.infos.size() + 1 );
    std::vector<unsigned> sequence_bone_data_indices( m_component_container.infos.size() + 1 );

    std::vector<SequenceID> nonzero_sequences;
    std::vector<float> sequence_time_progress;
    std::vector<float> bone_weights;

    CalculateWeights(
        m_component_container.infos,
        // state data
        m_state_container.m_state_infos, m_state_container.m_state_datas, m_state_container.m_sequence_weights, m_state_container.m_sequence_ids, m_state_container.m_bone_masks,
        // sequence output data
        sequence_data_indices, nonzero_sequences, sequence_time_progress,
        // sequence bone output data
        sequence_bone_data_indices, bone_weights);



    UpdatePoses( sequence_data_indices, nonzero_sequences, sequence_time_progress,
                 sequence_bone_data_indices, bone_weights,
                 m_keyframe_container.m_sequence_infos, m_keyframe_container.m_bone_states,
                 m_component_container.pose_indices, m_component_container.bone_states);

	auto pose_bone_states = m_component_container.bone_states;

    MakePosesAbsolute( pose_bone_states, m_component_container.pose_indices, m_component_container.skeleton_infos, m_skeleton_container.m_parent_bone_indices );

	UpdateOffsetPoses(m_indexed_offset_poses, m_component_container.entity_to_element, m_component_container.skeleton_infos, m_skeleton_container.m_absolute_bone_states, m_component_container.pose_indices, pose_bone_states);

    m_indexed_absolute_poses.indices = m_indexed_offset_poses.pose_from_entity;
    m_indexed_absolute_poses.pose_offsets = m_indexed_offset_poses.pose_offsets;
    m_indexed_absolute_poses.bone_states = move( pose_bone_states );

	m_state_container.MergeGaps();
}


IndexedOffsetPoses const & AnimatingWorld::GetIndexedOffsetPoses() const
{
	return m_indexed_offset_poses;
}


void AnimatingWorld::ApplyInstructions(EntityAnimatingInstructions const & instructions)
{
    // DEBUG:
    //DebugCheckForDuplicateInstructions(instructions);

    FadeOutStates(
        instructions,
        m_component_container.entity_to_element,
        m_component_container.infos,
        m_state_container.m_state_datas,
        m_animation_templates.descriptions,
        m_state_container.m_state_logics);

	AddStates(
        instructions,
        m_component_container.entity_to_element,
        m_component_container.infos,
        m_state_container,
        m_animation_templates.descriptions);
}


unsigned AnimatingWorld::GetBoneCountOfEntity(EntityID const entity_id) const
{
    // Get component and of entity
    auto component_id = m_component_container.entity_to_element[entity_id.index];

    // Get skeleton info for component and return bone count
    auto& skeleton_info = m_component_container.skeleton_infos[component_id];

    return skeleton_info.bone_count;
}