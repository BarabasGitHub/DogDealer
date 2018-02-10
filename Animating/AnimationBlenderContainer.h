#pragma once

#include "Structures.h"
#include "ResourceDescriptions.h"

namespace Animating{

	class AnimationBlenderContainer{
		
	public:
		
        AnimationBlenderID CreateCircleBlender(CircleBlenderDescription const & decription);
		
        std::vector<AnimationBlenderInfo> m_blender_infos;
        
        // Nodes for the blenders, refered to by the blender info
        std::vector<CircleBlenderNode> m_nodes;
    };


}