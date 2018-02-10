#pragma once
#include "Orientation.h"


// Vector of current and previous bone states,
// segmented into poses by the PoseInfo
// with entity id -> pose_from_entity -> pose_offsets -> bone states
struct IndexedOffsetPoses
{
	std::vector<Orientation> bone_states;
	std::vector<Orientation> previous_bone_states;
	std::vector<unsigned> pose_offsets;
	std::vector<unsigned> pose_from_entity;
};

struct IndexedAbsolutePoses
{
	std::vector<Orientation> bone_states;

    std::vector<unsigned> pose_offsets;
	std::vector<unsigned> indices;
};