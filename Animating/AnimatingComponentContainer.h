#pragma once

#include <Utilities\Range.h>

#include "Structures.h"

namespace Animating
{
	struct AnimatingComponentContainer
	{	

        void AddComponent( EntityID entity_id, SkeletonInfo skeleton_info, AnimationInfo animation_info, Range<BoneState const * > pose );
        void RemoveEntities(Range<EntityID const *> entities);       

		std::vector<unsigned> entity_to_element;
		std::vector<EntityID> entity_ids;

		std::vector<SkeletonInfo> skeleton_infos;	
				
		std::vector<AnimationInfo> infos;

        std::vector<unsigned> pose_indices;
        std::vector<BoneState> bone_states;

        AnimatingComponentContainer();

    private:
        unsigned AddPose( Range<BoneState const * > bone_states );
	};
}