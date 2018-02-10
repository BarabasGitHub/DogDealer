#pragma once
#include "IDs.h"

#include <BoundingShapes\AxisAlignedBox.h>

#include <FileLayout\VertexDataType.h> // for number of vertex data types

#include <array>
namespace Graphics
{
    struct Mesh
    {
        IndexBufferID index_id;
        std::array<VertexBufferID, c_number_of_vertex_data_types> vertex_ids;
    };

    inline bool operator==( Mesh const & first, Mesh const & second )
    {
        return first.index_id == second.index_id && first.vertex_ids == second.vertex_ids;
    }

    struct MeshData
    {
        Mesh mesh;
        BoundingShapes::AxisAlignedBox bounding_box;
    };


    inline bool operator==( MeshData const & first, MeshData const & second )
    {
        return first.mesh == second.mesh;
    }
}