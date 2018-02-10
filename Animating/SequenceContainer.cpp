#include "SequenceContainer.h"

#include <FileLayout\VertexDataType.h>
#include <Utilities\StreamHelpers.h>
#include <Utilities\StdVectorFunctions.h>

using namespace Animating;


SequenceID SequenceContainer::LoadSequence(std::istream& data_stream)
{
	using namespace std;

	auto header = ReadObject<AnimationHeader>(data_stream);

	// This could probably be done better by using the container helpers
	SequenceID id;
	id.index = static_cast<SequenceID::index_t>(m_sequence_infos.size());

	// Fill in sequence data (get context data from header)
	SequenceInfo sequence;

	sequence.frame_offset = static_cast<unsigned>(m_bone_states.size());
	sequence.frame_count = header.frame_count;

    m_bone_counts.push_back( header.bone_count );
    m_sequence_infos.push_back( sequence );

	// Reserve space for keyframes and bone states
	auto bone_state_count = header.bone_count * header.frame_count;
	auto new_data_range = Grow(m_bone_states, bone_state_count);
	// Stream file contents in
	Read(data_stream, begin(new_data_range), Size(new_data_range));

	return id;
}