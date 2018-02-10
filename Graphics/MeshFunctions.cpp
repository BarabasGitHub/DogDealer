#include "MeshFunctions.h"

#include "VertexBufferContainer.h"
#include "IndexBufferContainer.h"
#include "Device.h"

#include <BoundingShapes\AxisAlignedBoxFunctions.h> // For generated meshes

#include <Utilities\IntegerRange.h>
#include <Utilities\StreamHelpers.h>
#include <Utilities\ContainerHelpers.h>

#include <algorithm>
#include <functional>
#include <numeric>

using namespace Graphics;

namespace
{
    IndexBufferID LoadIndexBufferData( Device& device, IndexBufferContainer& index_buffer_container, MeshHeader const & header, std::istream& data_stream )
    {
        std::vector<unsigned> indices( header.index_count );

        ReadVector( data_stream, indices );

        auto index_buffer = device.CreateIndexBuffer( indices );

        IndexBufferInfo index_info;
        index_info.type = IndexBufferType::Normal;
        index_info.topology = header.topology;
        index_info.index_count = header.index_count;
        index_info.offset = 0;
        return index_buffer_container.AddNewIndexBuffer( index_info, index_buffer );
    }

	IndexBufferID GenerateIndexBufferData(Device& device, IndexBufferContainer& index_buffer_container, std::vector<unsigned> const & indices, D3D_PRIMITIVE_TOPOLOGY const topology)
	{
		auto index_buffer = device.CreateIndexBuffer(indices);

		IndexBufferInfo index_info;
		index_info.type = IndexBufferType::Normal;
		index_info.topology = topology;
		index_info.index_count = (unsigned)indices.size();
		index_info.offset = 0;
		return index_buffer_container.AddNewIndexBuffer(index_info, index_buffer);
	}

    std::array<unsigned, c_number_of_vertex_data_types> GetStrides( MeshHeader const & header )
    {
        std::array<unsigned, c_number_of_vertex_data_types> strides = {};
        for( auto i : CreateIntegerRange( c_number_of_vertex_data_types ) )
        {
            strides[i] = GetStride( header.vertex_types[i] );
        }
        return strides;
    }


    std::array<VertexBufferID, c_number_of_vertex_data_types> LoadVertexBufferData( Device& device, VertexBufferContainer& container, MeshHeader const & header, std::istream & data_stream )
    {
        using namespace std;
        // then determine the size of all vertex data
        auto strides = GetStrides( header );
        auto vertex_memory_size = accumulate( strides.begin(), strides.end(), 0 ) * header.vertex_count;

        // fill a cpu vector with the vertex data
        vector<uint8_t> vertex_data( vertex_memory_size );

        auto data_pointer = vertex_data.data();
        // keep track of the offsets
        // + 1 for spillage
        array<unsigned, c_number_of_vertex_data_types + 1> offsets = { 0 };
        for( auto i : CreateIntegerRange( strides.size() ) )
        {
            auto data_size = strides[i] * header.vertex_count;
            offsets[i + 1] = offsets[i] + data_size;
            Read( data_stream, data_pointer, data_size );
            data_pointer += data_size;
        }
        assert( offsets.back() == vertex_memory_size );


        auto vertex_buffer = device.CreateVertexBuffer( vertex_memory_size, vertex_data.data() );

        array<VertexBufferID, c_number_of_vertex_data_types> vertex_buffer_ids;
        VertexBufferID fake_id;
        fake_id.index = std::numeric_limits<decltype( fake_id.index )>::max();
        fake_id.generation = 0;
        fill( vertex_buffer_ids.begin(), vertex_buffer_ids.end(), fake_id );

        // we assume there are positions
        assert( header.vertex_types[c_positions_index] == VertexBufferType::Position );

        // first add the positions
        VertexBufferID vertex_buffer_id;
        {
            VertexBufferInfo vertex_info;
            vertex_info.type = header.vertex_types[c_positions_index];
            vertex_info.vertex_count = header.vertex_count;
            vertex_info.offset = offsets[c_positions_index];
            vertex_buffer_id = container.Add(vertex_info, vertex_buffer);
            vertex_buffer_ids[c_positions_index] = vertex_buffer_id;
        }

        // then add info for the other data types
        for( auto index : CreateIntegerRange( 1, header.vertex_types.size() ) )
        {
            auto type = header.vertex_types[index];
            if( type != VertexBufferType::None )
            {
                VertexBufferInfo vertex_info;
                vertex_info.type = type;
                vertex_info.vertex_count = header.vertex_count;
                vertex_info.offset = offsets[index];
                vertex_buffer_ids[index] = container.Add( vertex_info, vertex_buffer_id );
            }
        }

        return vertex_buffer_ids;
    }

	// Generate a mesh with vertex positions and normals
	std::array<VertexBufferID, c_number_of_vertex_data_types> GenerateVertexBufferData(Device& device, VertexBufferContainer& container, std::vector<PositionType> vertex_positions, std::vector<NormalType> const & vertex_normals)
	{
		using namespace std;

		auto vertex_count = (unsigned)vertex_positions.size();

		// Calculate needed memory for positions and normals
		unsigned vertex_memory_size = 2 * vertex_count;

		// Create buffer from data
		vertex_positions.insert(vertex_positions.end(), vertex_normals.begin(), vertex_normals.end());
		auto vertex_buffer = device.CreateVertexBuffer(vertex_memory_size, vertex_positions.data());

		// Create default invalid buffers for unused types
		array<VertexBufferID, c_number_of_vertex_data_types> vertex_buffer_ids;
		VertexBufferID fake_id;
		fake_id.index = std::numeric_limits<decltype(fake_id.index)>::max();
		fake_id.generation = 0;
		fill(vertex_buffer_ids.begin(), vertex_buffer_ids.end(), fake_id);

		// Create info for positions
		VertexBufferInfo vertex_info;
		vertex_info.type = VertexBufferType::Position;
		vertex_info.vertex_count = vertex_count;
		vertex_info.offset = 0;
		auto vertex_buffer_id = container.Add(vertex_info, vertex_buffer);
		vertex_buffer_ids[c_positions_index] = vertex_buffer_id;

		// Calculate offset, placed after position data
		unsigned offset = vertex_count * sizeof(PositionType);

		// Create info for normals
		vertex_info.type = VertexBufferType::Normal;
		vertex_info.vertex_count = vertex_count;
		vertex_info.offset = offset;
		vertex_buffer_ids[c_normals_index] = container.Add(vertex_info, vertex_buffer_id);

		return vertex_buffer_ids;
	}
}

MeshData Graphics::LoadMesh( Device& device, IndexBufferContainer & index_buffer_container, VertexBufferContainer & vertex_buffer_container, std::istream& data_stream )
{
    using namespace std;
    auto header = ReadObject<MeshHeader>( data_stream );

    auto box = ReadObject<BoundingShapes::AxisAlignedBox>( data_stream );

    auto index_buffer_id = LoadIndexBufferData( device, index_buffer_container, header, data_stream );

    auto vertex_buffer_ids = LoadVertexBufferData( device, vertex_buffer_container, header, data_stream );

    return{ { index_buffer_id, vertex_buffer_ids }, box };
}

MeshData Graphics::GenerateMesh(Device& device, IndexBufferContainer & index_buffer_container, VertexBufferContainer & vertex_buffer_container, std::vector<unsigned> const & indices, std::vector<PositionType> vertex_positions, std::vector<NormalType> const & vertex_normals)
{
	using namespace std;

	auto box = BoundingShapes::CreateAxisAlignedBox(vertex_positions);

	auto topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	auto index_buffer_id = GenerateIndexBufferData(device, index_buffer_container, indices, topology);

	auto vertex_buffer_ids = GenerateVertexBufferData(device, vertex_buffer_container, std::move(vertex_positions), vertex_normals);

	return{ { index_buffer_id, vertex_buffer_ids }, box };
}

void Graphics::UnloadMesh(Mesh const & mesh, IndexBufferContainer & index_buffer_container, VertexBufferContainer & vertex_buffer_container)
{
    index_buffer_container.RemoveIndexBuffer(mesh.index_id);
    vertex_buffer_container.Remove(mesh.vertex_ids);
}
