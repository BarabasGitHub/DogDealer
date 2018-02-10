#include "ResourceManager.h"

#include "AnimatingComponentContainer.h"

#include "AnimationBlenderContainer.h"
#include "StateContainer.h"

#include <Utilities\IndexedHelp.h>

#include <fstream>


using namespace std;

namespace {
	string FilePathFromSkeletonName(string const & skeleton_name)
	{
		auto file_path = "Resources\\" + skeleton_name + ".skel";
		return file_path;
	}
	string FilePathFromAnimationName(string const & animation_name)
	{
		auto file_path = "Resources\\" + animation_name + ".anim";
		return file_path;
	}
}

namespace Animating{
	
	void ResourceManager::CreateComponent(std::string const & skeleton, EntityID entity_id, AnimatingComponentContainer& component_container, SkeletonContainer& skeleton_container)
	{
        // Provide skeleton
		auto skeleton_info = ProvideSkeleton(skeleton, skeleton_container);
        
		// Provide and store pose
        auto skeleton_pose = CreateRange( skeleton_container.m_relative_bone_states, skeleton_info.bone_offset, skeleton_info.bone_offset + skeleton_info.bone_count );
        auto animation_info = StateContainer::CreateAnimationInfo();

        component_container.AddComponent( entity_id, skeleton_info, animation_info, skeleton_pose );		
	}

	SkeletonInfo ResourceManager::ProvideSkeleton(std::string const & skeleton_name, SkeletonContainer& skeleton_container)
	{
		auto result = m_skeleton_dictionary.find(skeleton_name);
		// check if already in the dictionary
		if (result != m_skeleton_dictionary.end())
		{
			return result->second;
		}

		// Otherwise load from file
		auto skeleton_file_path = FilePathFromSkeletonName(skeleton_name);
		ifstream data_stream(skeleton_file_path, std::ios::binary);
		assert(data_stream.good());

		auto skeleton = skeleton_container.LoadSkeleton(data_stream);
		data_stream.close();

		// add or overwrite if the ids were invalid.
		m_skeleton_dictionary[skeleton_name] = skeleton;
		return skeleton;
	}

	SequenceID ResourceManager::ProvideSequence(std::string const & animation_name, SequenceContainer& keyframe_container)
	{
		auto result = m_sequence_dictionary.find(animation_name);
		// check if already in the dictionary
		if (result != m_sequence_dictionary.end())
		{
			return result->second;
		}

		// Otherwise load from file
		auto skeleton_file_path = FilePathFromAnimationName(animation_name);
		ifstream data_stream(skeleton_file_path, std::ios::binary);
		assert(data_stream.good());

		auto animation_sequence = keyframe_container.LoadSequence(data_stream);
		data_stream.close();

		// add or overwrite if the ids were invalid.
		m_sequence_dictionary[animation_name] = animation_sequence;
		return animation_sequence;
	}

    AnimationBlenderID ResourceManager::ProvideAnimationBlender(CircleBlenderDescription const & blender_description, AnimationBlenderContainer& blender_container)
	{
		auto blender_id = blender_container.CreateCircleBlender(blender_description);
		return blender_id;
	}
}