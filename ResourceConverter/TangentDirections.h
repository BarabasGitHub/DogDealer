#include <FileLayout\VertexDataType.h>

#include <Math\FloatTypes.h>

#include <vector>


void CalculateTangents( const std::vector<Math::Float3> & vertex_positions, const std::vector<Math::Float3> & vertex_normals, const std::vector<Math::Float2> & vertex_texture_coordinates, std::vector<uint32_t> & vertex_indices, std::vector<TangentDirections> & vertex_tangents, std::vector<uint32_t> & duplicate_indices /*indices to the original elements that need to be duplicated and appended to the original vectors*/ );
