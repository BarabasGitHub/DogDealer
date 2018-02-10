#pragma once
#include "3DTerrainSystemStructs.h"
#include <Math\FloatTypes.h>

#include <array>
#include <vector>


namespace Graphics{

	struct EdgeList{
		bool vertex[12];
	};

	struct Vertices{
		std::array<PositionType, 12> pos;
		std::array<NormalType, 12> norm;
	};

	struct PreviousFace{
		std::array<int, 4>		vertex_indices;
		bool calculated = false;

		// Default initialization
		PreviousFace(){
			vertex_indices.fill(-1);
		}
	};

	void GenerateBlock(MarchingCubesParameters const & parameters, VertexData & vertex_data);

	void GenerateCuberille(std::array<DensityAndGradient, 8> const & densities_and_gradients, std::array<Math::Float3, 8> const & scaled_corners, VertexData & vertex_data, PreviousFace & previous_face);

	void GenerateTrianglesFromCase(uint32_t case_index, std::array<DensityAndGradient, 8> const & densities_and_gradients, std::array<Math::Float3, 8> const & scaled_corners, PreviousFace & previous_face, VertexData & vertex_data);

	void CalculateVertex(Math::Float3 const pos1, Math::Float3 const pos2, Math::Float3 norm1, Math::Float3 norm2, float const density1, float const density2, NormalType & normal, PositionType & position);

	void PruneKnownVertices(EdgeList & edges);


	void CalculateVertices(Vertices & vertices, EdgeList const & edges, std::array<DensityAndGradient, 8> const & densities_and_gradients, std::array<Math::Float3, 8> const & scaled_corners);

	void StoreVertices(EdgeList const & edges, Vertices const & vertices, std::array<unsigned, 12> & edges_to_indices, VertexData & vertex_data);

	void AddKnownVertexIndices(PreviousFace const & previous_face, std::array<unsigned, 12> & edges_to_indices);

	void StoreSideFace(std::array<unsigned, 12> const & edges_to_indices, PreviousFace & previous_face);

	EdgeList GetEdgesFromCase(unsigned case_index);

}