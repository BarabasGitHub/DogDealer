#pragma once

#include "Mesh.h"

#include <array>
#include <utility> // for pair
#include <vector>

namespace Graphics
{
    // forward declarations
    struct Device;
    struct IndexBufferContainer;
    struct VertexBufferContainer;


    MeshData LoadMesh( Device& device, IndexBufferContainer & index_buffer_container, VertexBufferContainer & vertex_buffer_container, std::istream& data_stream );

	MeshData GenerateMesh(Device& device, IndexBufferContainer & index_buffer_container, VertexBufferContainer & vertex_buffer_container, std::vector<unsigned> const & indices, std::vector<PositionType> vertex_positions, std::vector<NormalType> const & vertex_normals);

    void UnloadMesh(Mesh const & mesh, IndexBufferContainer & index_buffer_container, VertexBufferContainer & vertex_buffer_container);
}
