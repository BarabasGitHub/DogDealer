#include "MeshProcessing.h"
#include "TangentDirections.h"

#include <Utilities\FaceReordering.h>
#include <Utilities\VertexReordering.h>

namespace
{
    template<typename DataType>
    void AppendDuplicateData( std::vector<uint32_t> const & to_be_duplicated_data_indices, std::vector<DataType> & data )
    {
        auto destination_index = data.size();
        auto indices_size = to_be_duplicated_data_indices.size();
        data.resize( data.size() + indices_size );
        for( auto i = 0u; i < indices_size; ++i )
        {
            data[destination_index] = data[to_be_duplicated_data_indices[i]];
            ++destination_index;
        }
    }
}

FileData ProcessMesh( FileData data )
{
    if( data.vertex_tangents.empty() &&
        !data.vertex_positions.empty() &&
        !data.vertex_normals.empty() &&
        !data.vertex_uv_data.empty() )
    {
        std::vector<uint32_t> duplicated_data_indices;
        CalculateTangents( data.vertex_positions, data.vertex_normals, data.vertex_uv_data, data.vertex_indices, data.vertex_tangents, duplicated_data_indices );

        AppendDuplicateData( duplicated_data_indices, data.vertex_positions );
        AppendDuplicateData( duplicated_data_indices, data.vertex_normals );
        AppendDuplicateData( duplicated_data_indices, data.vertex_uv_data );
        if( !data.vertex_colors.empty() ) AppendDuplicateData( duplicated_data_indices, data.vertex_colors );
        if( !data.bone_weights_and_indices.empty() ) AppendDuplicateData( duplicated_data_indices, data.bone_weights_and_indices );
    }

    if( data.topology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST )
    {
        data.vertex_indices = ReorderFaces( move( data.vertex_indices ), unsigned( data.vertex_positions.size() ) );
    }

    auto new_vertex_indices = CalculateNewIndices( data.vertex_indices, unsigned( data.vertex_positions.size() ) );

    if( !data.vertex_positions.empty() ) data.vertex_positions = ReorderData( data.vertex_positions, data.vertex_indices, new_vertex_indices );
    if( !data.vertex_normals.empty() )  data.vertex_normals = ReorderData( data.vertex_normals, data.vertex_indices, new_vertex_indices );
    if( !data.vertex_colors.empty() ) data.vertex_colors = ReorderData( data.vertex_colors, data.vertex_indices, new_vertex_indices );
    if( !data.vertex_uv_data.empty() ) data.vertex_uv_data = ReorderData( data.vertex_uv_data, data.vertex_indices, new_vertex_indices );
    if( !data.vertex_tangents.empty() ) data.vertex_tangents = ReorderData( data.vertex_tangents, data.vertex_indices, new_vertex_indices );
    if( !data.bone_weights_and_indices.empty() ) data.bone_weights_and_indices = ReorderData( data.bone_weights_and_indices, data.vertex_indices, new_vertex_indices );

    data.vertex_indices = std::move( new_vertex_indices );

    return data;
}

