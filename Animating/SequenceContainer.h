#pragma once
#include <Conventions\Orientation.h>
#include "Structures.h"

namespace Animating{

	class SequenceContainer
    {

	public:

		SequenceID LoadSequence(std::istream& data_stream);

        // contains the offset and number of frames
		std::vector<SequenceInfo> m_sequence_infos;
        // in sync with the sequence_infos
        std::vector<unsigned> m_bone_counts;

		std::vector<BoneState> m_bone_states;
	};
}