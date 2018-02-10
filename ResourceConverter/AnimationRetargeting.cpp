#include "AnimationRetargeting.h"

#include <FileReaders\FileData.h>

#include <Math\FloatOperators.h>

namespace
{
	Orientation DetermineTransformation(Orientation a, Orientation const & b)
	{
		// For now just ignore the rotation
		a.position -= b.position;

		return a;
	}

	Orientation TransformBoneState(Orientation rest_pose, Orientation const & transformation)
	{		
		rest_pose.position += transformation.position;

		// For now just use the full transforming rotation
		rest_pose.rotation = transformation.rotation;

		return rest_pose;
	}
}


// Calculate the offsets of the bones from their rest pose in each frame
FileData CalculateAnimationTransformation(FileData const & file_data)
{
	// Initialize the transformation with a copy
	FileData transformation = file_data;

	auto const frame_count = file_data.keyframes.size();
	auto const bone_count = file_data.keyframes.front().bone_count;
	
	// Determine the transformation in each keyframe
	for (auto f = 0u; f < frame_count; f++)
	{
		for (auto b = 0u; b < bone_count; b++)
		{
			// Get bone orientation (or pose) index in current frame
			auto bone_state_index = f * bone_count + b;
			
			auto rest_pose = file_data.bone_orientations[b];
			auto current_pose = file_data.bone_states[bone_state_index];
			
			auto offset_pose = DetermineTransformation(current_pose, rest_pose);

			transformation.bone_states[bone_state_index] = offset_pose;
		}
	}

	return transformation;
}


// Generate bone states for all frames of the transformation data
// by applying the transformations to the rest pose of the skeleton
FileData ApplyTransformationToSkeleton(FileData skeleton_data,
									FileData const & transformation_data)
{
	auto const frame_count = transformation_data.keyframes.size();
	auto const bone_count = transformation_data.keyframes.front().bone_count;

	skeleton_data.keyframes = transformation_data.keyframes;
	skeleton_data.bone_state_euler_rotations = transformation_data.bone_state_euler_rotations;

	skeleton_data.bone_states.resize(frame_count * bone_count);


	// Determine the transformation in each keyframe
	for (auto f = 0u; f < frame_count; f++)
	{
		for (auto b = 0u; b < bone_count; b++)
		{
			// Get bone orientation (or pose) index in current frame
			auto bone_state_index = f * bone_count + b;

			auto rest_pose = skeleton_data.bone_orientations[b];
			auto transformation = transformation_data.bone_states[bone_state_index];
			
			auto result_pose = TransformBoneState(rest_pose, transformation);
						
			skeleton_data.bone_states[bone_state_index] = result_pose;
		}
	}

	return skeleton_data;
}