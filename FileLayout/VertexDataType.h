#pragma once
#include <cstdint>
#include <array>
#include <Math\FloatTypes.h>
#include <Math\IntegerTypes.h>

#include <Graphics\DirectX\Direct3D11.h>
#include <Graphics\VertexBufferType.h>

const auto c_number_of_vertex_data_types = 6;

struct MeshHeader
{
    unsigned index_count;
    unsigned vertex_count;
	D3D_PRIMITIVE_TOPOLOGY topology;
	std::array<Graphics::VertexBufferType,c_number_of_vertex_data_types> vertex_types;
};

auto const c_positions_index = 0;
auto const c_normals_index = c_positions_index + 1;
auto const c_texture_index = c_normals_index + 1;
auto const c_tangent_index = c_texture_index + 1;
auto const c_colors_index = c_tangent_index + 1;
auto const c_bone_index = c_colors_index + 1;

// Meshfile Layout:
//
// MeshHeader
// axis aligned bounding box
// indices
// positions
// normals
// uv,
// tangents,
// color
// bones
// ...

struct SkeletonHeader
{
	unsigned bone_count;
};
// SkeletonFile Layout:
//
// SkeletonHeader
// bones

struct AnimationHeader
{		
	unsigned frame_count;
	unsigned bone_count;
};
// AnimationFile Layout:
//
// AnimationHeader
// keyframes (their index being their time, each containing the number of bones)
// bone states


struct CollisionHeader
{
};
// CollisionFile layout:
//
// Axis-Aligned bounding box (center and dimensions as Float3)
//

typedef Math::Float3 PositionType;
typedef Math::Float3 NormalType;
typedef Math::Float2 UVType;
typedef Math::Float4 ColorType;


struct TangentDirections
{
    Math::Float3 tangent;
    Math::Float3 bitangent;
    int32_t      handedness; // handedness 1 right, -1 left
    TangentDirections() : tangent(0,0,0), bitangent(0,0,0), handedness(0){}
};


struct BoneWeightsAndIndices
{
    Math::Unsigned4 m_indices;
    Math::Float4	m_weights;
};