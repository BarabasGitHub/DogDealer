#pragma once
#include "DLL.h"
#include "ResourceManager.h"
#include "AnimatingComponentContainer.h"
#include "StateContainer.h"
#include "AnimationBlenderContainer.h"
#include "AnimationTemplates.h"

#include <Conventions\Orientation.h>
#include <Conventions\Velocity.h>
#include <Conventions\AnimatingInstructions.h>

namespace Animating{

	class ANIMATING_DLL AnimatingWorld{

		AnimatingComponentContainer m_component_container;

		SkeletonContainer m_skeleton_container;
		SequenceContainer m_keyframe_container;

		StateContainer m_state_container;

        IndexedOffsetPoses m_indexed_offset_poses;
		IndexedAbsolutePoses m_indexed_absolute_poses;

	public:

        ResourceManager	m_resource_manager;

        AnimationTemplates m_animation_templates;
        AnimationBlenderContainer m_blender_container;

		void ProcessInstructions(EntityAnimatingInstructions& animating_instructions);
        void UpdateStatesWithExternalParameters(IndexedOrientations const & indexed_orientations, IndexedVelocities const & indexed_velocities, Math::Float3 const camera_angles);

		void UpdateAnimations(const float time_step);

        void CreateAnimatingComponent( std::string const & skeleton, EntityID entity_id );

        SequenceID ProvideSequence( std::string const & sequence_name );
        // also returns the bone count
        SequenceID ProvideSequence( std::string const & sequence_name, uint32_t & frame_count );
        AnimationBlenderID ProvideAnimationBlender(CircleBlenderDescription const & blender_description);

		void ApplyInstructions(EntityAnimatingInstructions const & animating_instructions);

		IndexedOffsetPoses const & GetIndexedOffsetPoses() const;

        IndexedAbsolutePoses const & GetIndexedAbsolutePoses() const;

        void RemoveEntities( Range<EntityID const *> entities );

        unsigned GetBoneCountOfEntity(EntityID const entity_id) const;

        AnimatingWorld();
        ~AnimatingWorld();
	};


    inline IndexedAbsolutePoses const & AnimatingWorld::GetIndexedAbsolutePoses() const
    {
        return m_indexed_absolute_poses;
    }
}