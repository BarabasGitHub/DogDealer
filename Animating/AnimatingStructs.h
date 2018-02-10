#pragma once
#include <Math\FloatTypes.h>

#include <array>

// Defines shared constants and types for the AnimatingWorld
namespace Animating{
	
	const unsigned MAX_BONE_COUNT = 4096;

	struct BoneOrientation
	{
		Math::Float3		position;
		Math::Quaternion	rotation;
	};

	typedef std::array<BoneOrientation, MAX_BONE_COUNT> BoneArray;

}