#include "SkeletonContainer.h"

#include "PoseSystem.h"

#include <FileLayout\VertexDataType.h>

#include <Math\FloatOperators.h>
#include <Math\TransformFunctions.h>

#include <Utilities\ContainerHelpers.h>
#include <Utilities\StreamHelpers.h>

using namespace Animating;

SkeletonInfo SkeletonContainer::LoadSkeleton(std::istream& data_stream)
{
	auto header = ReadObject<SkeletonHeader>(data_stream);

	std::vector<Orientation> relative_bone_states;
	std::vector<int> parent_bone_indices;

	relative_bone_states.resize(header.bone_count);
	parent_bone_indices.resize(header.bone_count);

	ReadVector(data_stream, relative_bone_states);
	ReadVector(data_stream, parent_bone_indices);

    auto absolute_bone_states = relative_bone_states;
    MakePoseAbsolute(absolute_bone_states, parent_bone_indices);

	return InsertBoneData(relative_bone_states, absolute_bone_states, parent_bone_indices);
}


SkeletonInfo SkeletonContainer::InsertBoneData(std::vector<Orientation> const & relative_bone_states, std::vector<Orientation> const & absolute_bone_states, std::vector<int> const & parent_bone_indices)
{
    auto bone_count = unsigned(relative_bone_states.size());
    auto offset = FindNewOffset( m_gaps, bone_count, m_relative_bone_states, m_absolute_bone_states, m_parent_bone_indices );

    std::copy_n( begin( relative_bone_states ), relative_bone_states.size(), begin( m_relative_bone_states ) + offset );
    std::copy_n( begin( absolute_bone_states ), absolute_bone_states.size(), begin( m_absolute_bone_states ) + offset );
    std::copy_n( begin( parent_bone_indices ), parent_bone_indices.size(), begin( m_parent_bone_indices ) + offset );

	// Update AnimationStateInfo
    SkeletonInfo skeleton_info;
	skeleton_info.bone_count = bone_count;
	skeleton_info.bone_offset = offset;
    return skeleton_info;
}