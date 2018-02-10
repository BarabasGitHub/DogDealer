#pragma once

#include "ResourceDescriptions.h"
#include "Structures.h"

#include "SkeletonContainer.h"
#include "SequenceContainer.h"


namespace Animating{

	struct AnimatingComponentContainer;
	class StateContainer;
	class AnimationBlenderContainer;

	class ResourceManager{

	public:

        void CreateComponent( std::string const & skeleton, EntityID entity_id, AnimatingComponentContainer& component_container, SkeletonContainer& skeleton_container );


		SkeletonInfo ProvideSkeleton(std::string const & skeleton_name, SkeletonContainer& skeleton_container);

		SequenceID ProvideSequence(std::string const & animation_name, SequenceContainer& keyframe_container);

        static AnimationBlenderID ProvideAnimationBlender(CircleBlenderDescription const & blender_description, AnimationBlenderContainer& blender_container);

    private:
		std::map<std::string, AnimationBlenderID>	m_blender_dictionary;
		std::map<std::string, SkeletonInfo>			m_skeleton_dictionary;
		std::map<std::string, SequenceID>			m_sequence_dictionary;
	};
}
