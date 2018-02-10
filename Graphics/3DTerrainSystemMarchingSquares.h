#pragma once

#include <Math\FloatTypes.h>
#include <Math\IntegerTypes.h>

#include <array>
#include <vector>
#include <cstdint>

namespace Graphics
{
    struct DensityAndGradient;
    // using the convention:
    // 1--2 
    // :  :
    // 0--3
    // INPUT:
    // offset is the position of corner 0
    // scale is the size of the square sides
    // values are the values of corner 0, 1, 2, 3
    // OUPTUT:
    // vertices, the vertices form pairs which are the begin and end of line segments
	void CreateVerticesForCell(Math::Float2 const offset, Math::Float2 const scale, std::array<float, 4> const values, float const iso_value, std::vector<Math::Float2> & vertices);

    // similar but different, no offset or scaling, interpolates 3d normals
    void CreateVerticesForCell( std::array<Math::Unsigned2, 4> const & corner_coordinates, std::array<DensityAndGradient, 4> const values, float const iso_value, std::vector<Math::Float2> & vertices, std::vector<Math::Float3> & normals );

    // makes vertices for a tile (multiple cells)
	void CreateVerticesForTile(Math::Float2 const offset, Math::Float2 const cell_scale, Math::Unsigned2 cell_count, std::vector<float> const & values, float const iso_value, std::vector<Math::Float2> &  vertices);

    // similar to above but doesn't do scaling/offset but does interpolate 3d normals
    void CreateVerticesForTile( Math::Unsigned2 cell_count, std::vector<DensityAndGradient> const & values, float const iso_value, std::vector<Math::Float2> &  vertices, std::vector<Math::Float3> & normals );

    // input is vertices in pairs, output is indices to the input
    void SortVertices( std::vector<Math::Float2> const & input_vertices, std::vector<uint32_t> & output_indices );
}