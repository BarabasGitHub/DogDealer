#pragma once
#include "Structures.h"
#include <istream>

namespace Animating{

	class SkeletonContainer{

	public:		
		SkeletonInfo LoadSkeleton(std::istream& data_stream);

        SkeletonInfo InsertBoneData( std::vector<Orientation> const & relative_bone_states, std::vector<Orientation> const & absolute_bone_states, std::vector<int> const & parent_bone_indices );

		std::vector<Orientation>	m_relative_bone_states;
		std::vector<Orientation>	m_absolute_bone_states;
		std::vector<int>			m_parent_bone_indices;
	
    private:
		// Map of gap size to gap position index
		std::multimap<unsigned, unsigned> m_gaps;
	};
}

