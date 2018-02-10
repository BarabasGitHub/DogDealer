#pragma once
#include "MarchingCubes.h"

#include "MarchingCubesTables.h"

#include <Math\TransformFunctions.h>
#include <Math\IntegerOperators.h>
#include <Math\FloatOperators.h>
#include <Math\Conversions.h>
#include <Math\MathFunctions.h>

#include <Utilities\IndexUtilities.h>



namespace Graphics{

	// Translate edge index to relative indices of both corners forming the edge
	// The corner entries of every pair are sorted along the axis of the edge
	// by convention in sync to MarchingSquares, to avoid issues by rounding errors with swapped corners
	const std::array<std::array< uint8_t, 2>, 12> c_edge_to_corners =
	{ {
		{ { 0, 1 } },
		{ { 2, 1 } },
		{ { 3, 2 } },
		{ { 3, 0 } },

		{ { 4, 5 } },
		{ { 6, 5 } },
		{ { 7, 6 } },
		{ { 7, 4 } },

		{ { 0, 4 } },
		{ { 1, 5 } },
		{ { 2, 6 } },
		{ { 3, 7 } }
	} };

	// according to these coordinate offsets:
	const std::array<Math::Unsigned3 const, 8> c_corner_offets{ {
		{ 0u, 1u, 0u },
		{ 1u, 1u, 0u },
		{ 1u, 0u, 0u },
		{ 0u, 0u, 0u },

		{ 0u, 1u, 1u },
		{ 1u, 1u, 1u },
		{ 1u, 0u, 1u },
		{ 0u, 0u, 1u }
	} };


	// Generate one block of <cube_count> cuberilles
	void GenerateBlock(MarchingCubesParameters const & parameters, VertexData & vertex_data)
	{
		// Reset last calculated side face for each new row
		PreviousFace previous_face;
        auto const grid_strides = Math::Unsigned3(1u, parameters.grid_size.x, parameters.grid_size.y * parameters.grid_size.x);
        auto scaled_corners = c_default_corners;
        for( auto& corner : scaled_corners )
        {
            corner = parameters.point_offset + corner * parameters.cube_scale;
        }

        std::array<uint32_t, 8> data_offsets;
        for( auto i = 0u; i < 8; i++ )
        {
            data_offsets[i] = Dot( grid_strides, c_corner_offets[i] );
        }

		// Iterate over all three dimensions, creating cuberilles
        for (auto z = 0u; z < parameters.cube_counts.z; z++)
		{
            for (auto y = 0u; y < parameters.cube_counts.y; y++)
			{
				previous_face.calculated = false;

                for (auto x = 0u; x < parameters.cube_counts.x; x++)
				{
					// Calculate position of cuberille
					// Calculate scaled positions of default corners
                    Math::Unsigned3 index3d = { x, y, z };
					auto translation = Math::Float3FromUnsigned3(index3d);
                    translation *= parameters.cube_scale;

                    std::array<Math::Float3, 8> corners;
                    for( auto i = 0u; i < 8; ++i )
					{
						corners[i] = scaled_corners[i] + translation;
					}

					std::array<DensityAndGradient, 8> cube_densities_and_gradients;

                    auto data_start = begin(parameters.densities_and_gradients) + Dot(index3d, grid_strides);
                    for( auto i = 0u; i < 8; ++i )
					{
                        cube_densities_and_gradients[i] = data_start[data_offsets[i]];
					}

					GenerateCuberille(cube_densities_and_gradients, corners, vertex_data, previous_face);
				}
			}
		}
	}

	void GenerateCuberille(std::array<DensityAndGradient, 8> const & densities_and_gradients, std::array<Math::Float3, 8> const & scaled_corners, VertexData & vertex_data, PreviousFace & previous_face)
	{
		// Get triangulation case from corner densities
		uint32_t case_index = 0;
		for (auto i = 0u; i < 8; i++)
		{
			if (densities_and_gradients[i].density > c_surface_threshold)
			{
				case_index += 1 << i;
			}
		}

		GenerateTrianglesFromCase(case_index, densities_and_gradients, scaled_corners, previous_face, vertex_data);
	}


	void GenerateTrianglesFromCase(uint32_t case_index, std::array<DensityAndGradient, 8> const & densities_and_gradients, std::array<Math::Float3, 8> const & scaled_corners, PreviousFace & previous_face, VertexData & vertex_data)
	{
		// Get edges on which to place vertices from case
		auto edges = GetEdgesFromCase(case_index);

		// Get triangles to place from case
		auto & triangles = triangle_table[case_index];

		// Avoid recomputing known vertices
		if (previous_face.calculated)
		{
			PruneKnownVertices(edges);
		}

		// Get Normals and Positions of new vertices
		Vertices vertices;
		CalculateVertices(vertices, edges, densities_and_gradients, scaled_corners);

		std::array<unsigned, 12> edges_to_indices;

		// Store new vertices and keep track of their indices
		StoreVertices(edges, vertices, edges_to_indices, vertex_data);

		// Add previously adjacent cube vertex indices for triangulation
		AddKnownVertexIndices(previous_face, edges_to_indices);

		// Store indices, using the absolute indices of the edges_to_indices
		for (auto i = 0u; i < 16 && triangles[i] != -1; i++)
		{
			auto edge = triangles[i];
			auto index = edges_to_indices[edge];
			vertex_data.indices.push_back(index);
		}

		// Store face shared with next cuberille
		StoreSideFace(edges_to_indices, previous_face);
	}


	// Remove known vertices from edge list if they were calculated before already
	void PruneKnownVertices(EdgeList & edges)
	{
		// Prune edges of the left face vertices from edge list
		for (auto edge : c_edge_indices_left) edges.vertex[edge] = false;

	}


	void StoreVertices(EdgeList const & edges, Vertices const & vertices, std::array<unsigned, 12> & edges_to_indices, VertexData & vertex_data)
	{
		// Store vertex positions, remember indices
		for (auto i = 0u; i < 12; i++)
		{
			if (edges.vertex[i])
			{
				auto index = (unsigned)vertex_data.positions.size();

				vertex_data.positions.push_back(vertices.pos[i]);
				vertex_data.normals.push_back(vertices.norm[i]);

				edges_to_indices[i] = index;
			}
		}
	}


	// Calculate vertex positions by interpolation along any neighbouring
	// vertices of differing evaluation
	void CalculateVertices(Vertices & vertices, EdgeList const & edges, std::array<DensityAndGradient, 8> const & densities_and_gradients, std::array<Math::Float3, 8> const & scaled_corners)
	{
		// Loop over edges
		for (auto i = 0u; i < 12; i++)
		{
			// Check if vertex should be placed on edge
			if (edges.vertex[i])
			{
				// Get pair of corners forming the edge
				auto corners = c_edge_to_corners[i];

				// Get data of both corners
				auto data_0 = densities_and_gradients[corners[0]];
				auto data_1 = densities_and_gradients[corners[1]];

				CalculateVertex(scaled_corners[corners[0]], scaled_corners[corners[1]], data_0.gradient, data_1.gradient, data_0.density, data_1.density, vertices.norm[i], vertices.pos[i]);
			}
		}


		/*
		// HORIZONTAL:
		// Bottom square
		for (auto i = 0u; i < 3; i++)
		{
			if (edges.vertex[i])
			{
				auto data1 = densities_and_gradients[i];
				auto data2 = densities_and_gradients[i + 1];
				CalculateVertex(scaled_corners[i], scaled_corners[i + 1], data1.gradient, data2.gradient, data1.density, data2.density, vertices.norm[i], vertices.pos[i]);
			}
		}
		// Bottom left edge
		if (edges.vertex[3])
		{
			auto data1 = densities_and_gradients[3];
			auto data2 = densities_and_gradients[0];
			CalculateVertex(scaled_corners[3], scaled_corners[0], data1.gradient, data2.gradient, data1.density, data2.density, vertices.norm[3], vertices.pos[3]);
		}

		// Top square
		for (auto i = 4; i < 7; i++)
		{
			if (edges.vertex[i])
			{
				auto data1 = densities_and_gradients[i];
				auto data2 = densities_and_gradients[i + 1];
				CalculateVertex(scaled_corners[i], scaled_corners[i + 1], data1.gradient, data2.gradient, data1.density, data2.density, vertices.norm[i], vertices.pos[i]);
			}
		}
		// Top left edge
		if (edges.vertex[7])
		{
			auto data1 = densities_and_gradients[7];
			auto data2 = densities_and_gradients[4];
			CalculateVertex(scaled_corners[7], scaled_corners[4], data1.gradient, data2.gradient, data1.density, data2.density, vertices.norm[7], vertices.pos[7]);
		}

		// VERTICAL
		for (auto i = 8; i < 12; i++)
		{
			if (edges.vertex[i])
			{
				auto data1 = densities_and_gradients[i - 8];
				auto data2 = densities_and_gradients[i - 4];
				CalculateVertex(scaled_corners[i - 8], scaled_corners[i - 4], data1.gradient, data2.gradient, data1.density, data2.density, vertices.norm[i], vertices.pos[i]);
			}
		}
		*/
	}


	// Calculate the position of a vertex on the edge between
	// two corners by linear interpolation of the density
	void CalculateVertex(Math::Float3 const pos1, Math::Float3 const pos2, Math::Float3 norm1, Math::Float3 norm2, float const density1, float const density2, NormalType & normal, PositionType & position)
	{
		auto factor = (c_surface_threshold - density1) / (density2 - density1);

		position = Lerp(pos1, pos2, factor);
		normal = -Nlerp(norm1, norm2, factor);
	}


	// Retrieve indices of adjacent previously computed cuberilles
	void AddKnownVertexIndices(PreviousFace const & previous_face, std::array<unsigned, 12> & edges_to_indices)
	{
		if (!previous_face.calculated) return;

		// Iterate over left face, adding known indices
		// for vertices placed on the edges
		for (auto i = 0u; i < 4; i++)
		{
			// Get index of current edge
			auto index = c_edge_indices_left[i];

			// Write vertex index to edges_to_indices
			edges_to_indices[index] = previous_face.vertex_indices[i];
		}
	}


	// Convert index to bool array
	EdgeList GetEdgesFromCase(unsigned case_index)
	{
		// Convert hex to bool array
		uint_fast16_t edges = edge_table[case_index];
		EdgeList edge_list = EdgeList();

		// Check for all powers of 2
		for (auto i = 11; i >= 0; i--)
		{
			// Get i-th power of 2
			auto power = 1u << i;

			if (edges >= power)
			{
				edge_list.vertex[i] = true;
				edges -= power;
			}
		}
		return edge_list;
	}


	void StoreSideFace(std::array<unsigned, 12> const & edges_to_indices, PreviousFace & previous_face)
	{
		// Iterate over side face edges
		for (auto i = 0u; i < 4; i++)
		{
			auto right_edge_index = c_edge_indices_right[i];

			previous_face.vertex_indices[i] = edges_to_indices[right_edge_index];
		}

		previous_face.calculated = true;
	}

}
