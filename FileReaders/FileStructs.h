#pragma once
#include <FileLayout\VertexDataType.h>

#include <Math\FloatTypes.h>
#include <Math\IntegerTypes.h>

#include <Conventions\Orientation.h>

#include <vector>
#include <cstdint>



namespace FileReading{
	struct Vertex{
		Math::Float3 pos;
		Math::Float3 norm;
		Math::Float2 uv;
		unsigned     index;
	};


	struct Bone
	{
		int parent_index;
		std::string name;

		Orientation orientation;

		Bone():
			parent_index(-1),
			name(),
			orientation()
		{}
	};

	struct Keyframe{
		//unsigned time;
		unsigned bone_count;
	};

	typedef std::vector<Math::Float3>		PositionsContainer;
	typedef std::vector<Math::Float3>		NormalsContainer;
	typedef std::vector<TangentDirections>	TangentContainer;

	typedef std::vector<Math::Float2> UVsContainer;
	typedef std::vector<Math::Float4> ColorsContainer;

	typedef std::vector<unsigned>				IndicesContainer;
	typedef std::vector<BoneWeightsAndIndices>	BoneWeightsAndIndicesContainer;


	typedef std::vector<Bone>			BoneContainer;
	typedef std::vector<Orientation>	BoneOrientationContainer;
	typedef std::vector<int>			BoneParentIndexContainer;


	typedef std::vector<Keyframe>	KeyFrameContainer;
	typedef std::vector<Orientation>	BoneStateContainer;
}