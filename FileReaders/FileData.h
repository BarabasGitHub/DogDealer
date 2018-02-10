#pragma once
#include "FileStructs.h"

// Container class
class FileData{

public:

	// name
	std::string name;

	// Meshes
	FileReading::PositionsContainer vertex_positions;
	FileReading::UVsContainer		vertex_uv_data;
	FileReading::NormalsContainer	vertex_normals;
	FileReading::IndicesContainer	vertex_indices;
	FileReading::ColorsContainer	vertex_colors;
	FileReading::TangentContainer   vertex_tangents;

	D3D_PRIMITIVE_TOPOLOGY			topology;
	FileReading::BoneWeightsAndIndicesContainer bone_weights_and_indices;

	// Skeletons
	//FileReading::BoneContainer	bones;
	FileReading::BoneOrientationContainer	bone_orientations;
	FileReading::BoneParentIndexContainer	bone_parent_indices;
	std::vector<Math::Float3>				bone_orientation_euler_rotations;
	std::vector<std::string>				bone_names;

	// Animations
	FileReading::KeyFrameContainer keyframes;
	FileReading::BoneStateContainer bone_states;
	std::vector<Math::Float3> bone_state_euler_rotations;
};