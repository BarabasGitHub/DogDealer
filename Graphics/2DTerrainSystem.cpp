#include "2DTerrainSystem.h"

#include "2DTerrainConfiguration.h"
#include "ConstantBufferContainer.h"
#include "ConstantBuffers.h"
#include "Device.h"
#include "FillConstantBuffer.h"
#include "IndexBufferContainer.h"
#include "VertexBufferContainer.h"
#include "RenderComponentContainer.h"
#include "ResourceManager.h"
#include "ResourceViewContainer.h"
#include "TextureContainer.h"
#include "TextureType.h"

#include <BoundingShapes\AxisAlignedBoxFunctions.h>

#include <Conventions\Orientation.h>

#include <Math\Conversions.h>
#include <Math\FloatOperators.h>
#include <Math\MathFunctions.h>
#include <Math\TransformFunctions.h>
#include <Math\MortonOrder.h>

#include <Utilities\IndexUtilities.h>
#include <Utilities\StdVectorFunctions.h>
#include <Utilities\VectorHelper.h>
#include <Utilities\MinMaxFunctions.h>
#include <Utilities\Memory.h>

#include <random>

using namespace Graphics;
using namespace Math;

uint32_t Graphics::VertexCountForMeshOn2DGrid(Unsigned2 size)
{
    return Product(size);
}


uint32_t Graphics::TriangleCountForMeshOn2DGrid(Unsigned2 size)
{
    size -= 1;
    return Product(size) * 2;
}


namespace
{
    Float2 CalculateVertexPosition(Float2 start, Float2 step, Unsigned2 size, uint32_t vertex_index)
    {
        auto index2d = Calculate2DindexFrom1D(vertex_index, size.x);
        return start + step * Float2FromUnsigned2(index2d);
    }


    void GenerateVertexPositions(Float2 center, Float2 extent, Unsigned2 size, Range<Float3 *> vertex_positions)
    {
        auto start = center - extent;
        auto step = (extent * 2) / Float2FromUnsigned2(size - 1);
        for( auto i = 0u; i < Size(vertex_positions); ++i )
        {
            auto position2d = CalculateVertexPosition(start, step, size, i);
            vertex_positions[i] = {position2d.x, position2d.y, 0};
        }
    }

    template<typename IndexType>
    void BackwardSlashSquare(Unsigned2 index, uint32_t size_x, Range<IndexType *> triangle_vertex_indices)
    {
        auto vertex_index = Calculate1DindexFrom2D(index, size_x);
        // triangle 1
        {
            auto vertex0 = vertex_index;
            auto vertex1 = vertex0 + 1;
            auto vertex2 = vertex1 + size_x;
            triangle_vertex_indices[0] = IndexType(vertex0);
            triangle_vertex_indices[1] = IndexType(vertex1);
            triangle_vertex_indices[2] = IndexType(vertex2);
        }
        // triangle 2
        {
            auto vertex0 = vertex_index;
            auto vertex2 = vertex0 + size_x;
            auto vertex1 = vertex2 + 1;
            triangle_vertex_indices[3] = IndexType(vertex0);
            triangle_vertex_indices[4] = IndexType(vertex1);
            triangle_vertex_indices[5] = IndexType(vertex2);
        }
    }


    template<typename IndexType>
    void ForwardSlashSquare(Unsigned2 index, uint32_t size_x, Range<IndexType *> triangle_vertex_indices)
    {
        auto vertex_index = Calculate1DindexFrom2D(index, size_x);
        // triangle 1
        {
            auto vertex0 = vertex_index;
            auto vertex1 = vertex0 + 1;
            auto vertex2 = vertex0 + size_x;
            triangle_vertex_indices[0] = IndexType(vertex0);
            triangle_vertex_indices[1] = IndexType(vertex1);
            triangle_vertex_indices[2] = IndexType(vertex2);
        }
        // triangle 2
        {
            auto vertex0 = vertex_index + 1;
            auto vertex1 = vertex0 + size_x;
            auto vertex2 = vertex1 - 1;
            triangle_vertex_indices[3] = IndexType(vertex0);
            triangle_vertex_indices[4] = IndexType(vertex1);
            triangle_vertex_indices[5] = IndexType(vertex2);
        }
    }




    template<typename IndexType>
    void GenerateTriangleIndicesImpl(Unsigned2 size, Range<IndexType  *> triangle_vertex_indices)
    {
        assert(TriangleCountForMeshOn2DGrid(size) * 3 == Size(triangle_vertex_indices));
        assert(VertexCountForMeshOn2DGrid(size) <= std::numeric_limits<IndexType>::max());
        auto triangle_vertex_indices_iterator = begin(triangle_vertex_indices);
        auto index_index = 0u;
        for( auto i = 0u; i < (size.y - 1); ++i )
        {
            for( auto j = 0u; j < (size.x - 1); ++j )
            {
                BackwardSlashSquare({j, i}, size.x, CreateRange(triangle_vertex_indices_iterator + index_index, 6));
                index_index += 6;
            }
        }
    }
}


void Graphics::GenerateTriangleIndices(Unsigned2 size, Range<uint32_t  *> triangle_vertex_indices)
{
    GenerateTriangleIndicesImpl(size, triangle_vertex_indices);
}



void Graphics::Generate3DMeshOn2DGrid(Float2 center, Float2 extent, Unsigned2 size, Range<Math::Float3 *> vertex_positions, Range<uint32_t*> triangle_vertex_indices)
{
    assert(VertexCountForMeshOn2DGrid(size) == Size(vertex_positions));
    assert(TriangleCountForMeshOn2DGrid(size) * 3 == Size(triangle_vertex_indices));

    GenerateVertexPositions(center, extent, size, vertex_positions);
    GenerateTriangleIndices(size, triangle_vertex_indices);
}


void Graphics::GenerateHeightAndNormals(Get2DSampleFunctionType const & sample_function, Range<Math::Float3*> vertex_positions, Range<Math::Float3*> normals)
{
    assert(Size(vertex_positions) == Size(normals));

    auto count = Size(vertex_positions);
    for( auto i = 0u; i < count; ++i )
    {
        auto & pos = vertex_positions[i];
        Float2 gradient;
        sample_function({pos.x, pos.y}, pos.z, gradient);
        normals[i] = Normalize(Float3(-gradient.x, -gradient.y, 1.f));
    }
}


MinMax<float> Graphics::GenerateHeightAndNormals(Float2 start, Float2 size, Unsigned2 dimensions, Get2DSampleFunctionType const & sample_function, Range<NormalAndHeight*> data)
{
    auto step = size / Float2FromUnsigned2(dimensions - 1);
    auto count = Size(data);
    MinMax<float> minmax_height = {std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest()};
    for( auto i = 0u; i < count; ++i )
    {
        auto position = CalculateVertexPosition(start, step, dimensions, i);
        Float2 gradient;
        float height;
        sample_function(position, height, gradient );
        minmax_height = Update(minmax_height, height);
        data[i].normal = Normalize(Float3(-gradient.x, -gradient.y, 1.f));
        data[i].height = height;
    }
    return minmax_height;
}


namespace
{
    template<typename IndexType>
    IndexBufferID MakeIndexBuffer(Range<IndexType const*> indices, Device& device, IndexBufferContainer& index_buffer_container)
    {
        assert(Size(indices) % 3 == 0);
        auto index_buffer = device.CreateIndexBuffer(indices);
        IndexBufferInfo index_info;
        index_info.type = IndexBufferTypeFromIntegerType<IndexType>();
        index_info.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        index_info.index_count = UINT(Size(indices));
        index_info.offset = 0;

        auto index_buffer_id = index_buffer_container.AddNewIndexBuffer( index_info, index_buffer );
        return index_buffer_id;
    }


}

TextureTypeAndID Graphics::CreateNormalAndHeightTexture(
    Unsigned2 size,
    Range<NormalAndHeight const*> normals_and_heights,
    DeviceContext& device_context,
    Device& device,
    TextureContainer& texture_container,
    ResourceViewContainer& resource_view_container)
{
    auto texture_stuff = CreateNormalAndHeightTexture(size, device, texture_container, resource_view_container);

    auto texture = texture_container.GetTexture2D(texture_stuff.data_id);
    device_context->UpdateSubresource(texture, 0, nullptr, begin(normals_and_heights), size.x*sizeof(NormalAndHeight), UINT(Size(normals_and_heights) * sizeof(NormalAndHeight)));

    // with mip-maps
    // m_immediate_context->GenerateMips(texture_resource_view);

    return texture_stuff;
}


TextureTypeAndID Graphics::CreateNormalAndHeightTexture(
    Unsigned2 size,
    Device& device,
    TextureContainer& texture_container,
    ResourceViewContainer& resource_view_container)
{
    D3D11_TEXTURE2D_DESC texture_description;
    texture_description.Width = size.x;
    texture_description.Height = size.y;
    texture_description.ArraySize = 1;
    texture_description.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texture_description.SampleDesc = {1, 0};
    texture_description.Usage = D3D11_USAGE_DEFAULT;
    texture_description.CPUAccessFlags = 0;

    // without mip-maps
    texture_description.MipLevels = 1;
    texture_description.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texture_description.MiscFlags = 0;
    auto texture = device.CreateTexture2D(texture_description);

    // with mip-maps
    // texture_description.MipLevels = 0;
    // texture_description.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET; // render target so we can generate mip maps
    // texture_description.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    // auto texture = device.CreateTexture2D(texture_description, nullptr);

    auto texture_resource_view = device.CreateShaderResourceView(texture);

    auto texture_id = texture_container.AddTexture( texture );
    auto texture_resource_view_id = resource_view_container.AddShaderResourceView(texture_resource_view);

    return {TextureType::Height, texture_id, texture_resource_view_id};
}


void Graphics::UpdateNormalAndHeightTexture(
    Unsigned2 patch_size,
    Unsigned2 patch_offset,
    Range<NormalAndHeight const*> normals_and_heights,
    DeviceContext& device_context,
    ID3D11Texture2D* texture
    )
{
    D3D11_TEXTURE2D_DESC texture_description;
    texture->GetDesc(&texture_description);
    assert(texture_description.ArraySize == 1);
    assert(texture_description.SampleDesc.Count == 1);
    assert(texture_description.SampleDesc.Quality == 0);
    assert(texture_description.Format == DXGI_FORMAT_R32G32B32A32_FLOAT);
    Unsigned2 texture_size = {texture_description.Width, texture_description.Height};

    D3D11_BOX box;
    box.left = patch_offset.x;
    box.right = patch_offset.x + patch_size.x;
    box.top = patch_offset.y;
    box.bottom = patch_offset.y + patch_size.y;
    box.front = 0;
    box.back = 1;

    device_context->UpdateSubresource(texture, 0, &box, begin(normals_and_heights), patch_size.x*sizeof(NormalAndHeight), UINT(Size(normals_and_heights) * sizeof(NormalAndHeight)));
}


IndexBufferID Graphics::MakeBlockIndexBuffer(Unsigned2 size, Device& device, IndexBufferContainer& index_buffer_container)
{
    auto triangle_count = TriangleCountForMeshOn2DGrid(size);
    IndexBufferID index_buffer_id;
    if (VertexCountForMeshOn2DGrid(size) <= std::numeric_limits<uint16_t>::max())
    {
        std::vector<uint16_t> indices(triangle_count * 3);
        GenerateTriangleIndicesImpl<uint16_t>(size, indices);
        index_buffer_id = MakeIndexBuffer<uint16_t>(indices, device, index_buffer_container);
    }
    else
    {
        std::vector<uint32_t> indices(triangle_count * 3);
        GenerateTriangleIndicesImpl<uint32_t>(size, indices);
        index_buffer_id = MakeIndexBuffer<uint32_t>(indices, device, index_buffer_container);
    }
    return index_buffer_id;
}


void Graphics::UpdatePatchBuffer(
    Float3 start3d,
    Unsigned2 dimensions,
    uint32_t lod_level,
    std::array<bool, 4> stitch_edge,
    ID3D11Buffer* constant_buffer,
    DeviceContext& device_context
    )
{
    Float2 start = {start3d.x, start3d.y};
    Unsigned2 edges_to_double;
    if(stitch_edge[0])
    {
        edges_to_double.x = 0;
    }
    else if(stitch_edge[1])
    {
        edges_to_double.x = dimensions.x - 1;
    }
    else
    {
        edges_to_double.x = uint32_t(-1);
    }
    if(stitch_edge[2])
    {
        edges_to_double.y = 0;
    }
    else if(stitch_edge[3])
    {
        edges_to_double.y = dimensions.y - 1;
    }
    else
    {
        edges_to_double.y = uint32_t(-1);
    }

    FillConstantBuffer<ConstantBuffers::Terrain2DPatch>(constant_buffer, device_context, [&start, edges_to_double, lod_level](auto buffer_data)
    {
        buffer_data->start = start;
        buffer_data->texture_offset = 0;
        buffer_data->lod_level = lod_level;
        buffer_data->edges_to_double = edges_to_double;
    });
}


void Graphics::CreateBlock(
    Float3 start3d,
    Float2 size,
    Unsigned2 dimensions,
    uint32_t lod_level,
    std::array<bool, 4> stitch_edge,
    Get2DSampleFunctionType const & sample_function,
    Device& device,
    DeviceContext& device_context,
    TextureContainer& texture_container,
    ResourceViewContainer& resource_view_container,
    ConstantBufferContainer& constant_buffer_container,
    TextureTypeAndID& height_texture_info,
    ConstantBufferID& constant_buffer,
    BoundingShapes::AxisAlignedBox& bounding_box)
{
    size *= float(1 << lod_level);
    Float2 start = {start3d.x, start3d.y};
    auto vertex_count = VertexCountForMeshOn2DGrid(dimensions);
    std::vector<NormalAndHeight> normals_and_heights(vertex_count);

    auto buffer_size = uint32_t(sizeof(ConstantBuffers::Terrain2DPatch));
    auto patch_buffer = device.CreateConstantBuffer(buffer_size);

    height_texture_info = CreateNormalAndHeightTexture(dimensions, device, texture_container, resource_view_container);
    auto height_texture = texture_container.GetTexture2D(height_texture_info.data_id);
    auto minmax_height = GenerateHeightAndNormals(start, size, dimensions, sample_function, normals_and_heights);
    UpdateNormalAndHeightTexture(dimensions, 0, normals_and_heights, device_context, height_texture);

    UpdatePatchBuffer(start3d, dimensions, lod_level, stitch_edge, patch_buffer, device_context);

    constant_buffer = constant_buffer_container.Add(patch_buffer);

    auto extent = size / 2;
    auto height_extent = ( minmax_height.max - minmax_height.min ) / 2;
    auto height_center = ( minmax_height.max + minmax_height.min ) / 2;
    bounding_box.extent = {extent.x, extent.y, height_extent};
    bounding_box.center = {start.x, start.y, 0};
    bounding_box.center.x += extent.x;
    bounding_box.center.y += extent.y;
    bounding_box.center.z += height_center;
}


void Graphics::InitializeTerrainData(
    EntityID entity_id,
    Math::Float3 terrain_center,
    Range<float const *> lod_distances,
    Get2DSampleFunctionType sample_function,
    Math::Unsigned2 patch_dimensions,
    Math::Float2 patch_size,
    Math::Unsigned2 total_size,
    RenderComponentDescription const & component_description,
    ResourceManager& resource_manager,
    ConstantBufferContainer& constant_buffer_container,
    TextureContainer& texture_container,
    ResourceViewContainer& resource_view_container,
    IndexBufferContainer& index_buffer_container,
    Device& device,
    DeviceContext& device_context,
    Terrain2DData& terrain_data
    )
{
    assert(terrain_center.z == 0);
    assert(!IsEmpty(lod_distances));
    assert(total_size.x == total_size.y);
    assert(total_size.x % (1u << Size(lod_distances)) == 0);

    terrain_data.entity_id = entity_id;
    terrain_data.sample_function = std::move(sample_function);
    terrain_data.patch_dimensions = patch_dimensions;
    terrain_data.patch_size = patch_size;
    terrain_data.center = terrain_center;
    terrain_data.total_size = total_size;
    terrain_data.lod_distances.assign(begin(lod_distances), end(lod_distances));

    terrain_data.graphics.display_technique = resource_manager.ProvideDisplayTechnique( component_description.technique );
    terrain_data.graphics.shadow_technique = resource_manager.ProvideDisplayTechnique( component_description.shadow_technique );
    terrain_data.graphics.alpha_to_coverage = component_description.alpha_to_coverage;

    auto constant_buffer_size = uint32_t(sizeof(ConstantBuffers::Terrain2D));
    auto terrain_constant_buffer = device.CreateConstantBuffer(constant_buffer_size);
    terrain_data.graphics.persistent_terrain_data_buffer = constant_buffer_container.Add(terrain_constant_buffer);

    auto terrain_2d = constant_buffer_container.GetBuffer(terrain_data.graphics.persistent_terrain_data_buffer);
    FillConstantBufferWithData(terrain_2d, device_context, ConstantBuffers::Terrain2D{terrain_data.patch_dimensions, terrain_data.patch_size});

    terrain_data.graphics.universal_patch_index_buffer = MakeBlockIndexBuffer(terrain_data.patch_dimensions, device, index_buffer_container);

    terrain_data.graphics.global_textures.resize(Size(component_description.textures));
    resource_manager.ProvideTextureList(
        component_description.textures,
        texture_container,
        resource_view_container,
        device,
        terrain_data.graphics.global_textures);

    auto max_number_of_terrain_blocks = Math::Product(total_size);
    terrain_data.lod_levels.resize(max_number_of_terrain_blocks, uint32_t(-1));
    terrain_data.patch_data.resize(max_number_of_terrain_blocks);
    Fill(uint8_t(0xff), reinterpret_cast<uint8_t*>(terrain_data.patch_data.data()), Size(terrain_data.patch_data) * sizeof(Terrain2DData::PatchData));
}


void Graphics::InitializeGrassData(
    Range<Graphics::RenderComponentDescription const *> descriptions,
    Range<uint32_t const *> count_and_distance_offsets,
    Range<float const *> densities,
    Range<float const *> distances,
    Math::Unsigned2 total_terrain_size,
    ResourceManager & resource_manager,
    IndexBufferContainer & index_buffer_container,
    VertexBufferContainer & vertex_buffer_container,
    TextureContainer & texture_container,
    ResourceViewContainer & resource_view_container,
    ConstantBufferContainer & constant_buffer_container,
    Device & device,
    std::vector<Terrain2DData::GrassParameters> & grass_parameters,
    std::vector<uint32_t> & grass_lod_data_offsets,
    std::vector<float> & grass_lod_densities,
    std::vector<float> & grass_lod_distances
    )
{
    assert(Size(descriptions) + 1 == Size(count_and_distance_offsets));
    assert(Size(densities) == Size(distances));

    grass_lod_data_offsets.assign(begin(count_and_distance_offsets), end(count_and_distance_offsets));
    grass_lod_densities.assign(begin(densities), end(densities));
    grass_lod_distances.assign(begin(distances), end(distances));

    grass_parameters.resize(Size(descriptions));

    auto object_constant_buffer_size = uint32_t(sizeof(ConstantBuffers::Object));
    // Get bounds for individual grass types
    for (auto i = 0u; i < Size(descriptions); ++i)
    {
        auto & grass = grass_parameters[i];
        auto & description = descriptions[i];
        // get mesh data
        auto mesh_data = resource_manager.ProvideMeshData(description.mesh, index_buffer_container, vertex_buffer_container, device);
        // set the mesh
        grass.graphics.mesh = mesh_data.mesh;
        // begin with no grass instance buffers
        grass.graphics.instance_buffers.clear();
        // create object constant buffer
        auto object_buffer = device.CreateConstantBuffer(object_constant_buffer_size);
        grass.graphics.object_buffer = constant_buffer_container.Add(object_buffer);
        // Get grass texture list
        grass.graphics.textures.resize(Size(description.textures));
        resource_manager.ProvideTextureList(description.textures, texture_container, resource_view_container, device, grass.graphics.textures);
        // Get techniques
        grass.graphics.display_technique = resource_manager.ProvideDisplayTechnique(description.technique);
        grass.graphics.shadow_technique = resource_manager.ProvideDisplayTechnique(description.shadow_technique);
        // alpha to coverage
        grass.graphics.alpha_to_coverage = description.alpha_to_coverage;
        // bounding box
        grass.original_bounds = mesh_data.bounding_box;
        // start without batches
        grass.batch_bounds.clear();
        // initialize the lod levels with invalid ones
        grass.lod_levels.resize(Product(total_terrain_size), uint32_t(-1));
    }
}


namespace
{
    bool MaxOneDifferenceBetweenLODs(Range<uint32_t const *> morton_lod_levels)
    {
        auto dimensions = uint32_t(Sqrt(float(Size(morton_lod_levels))));
        for( auto y = 1u; y < dimensions - 1; ++y )
        {
            for( auto x = 1u; x < dimensions - 1; ++x )
            {
                auto index = MortonIndex32Bit({x, y});
                uint32_t neighbours[4] = {
                    MortonIndex32Bit({x + 1, y}),
                    MortonIndex32Bit({x, y + 1}),
                    MortonIndex32Bit({x - 1, y}),
                    MortonIndex32Bit({x, y - 1}),
                    };
                for( auto i = 0u; i < 4; ++i )
                {

                    if (AbsoluteDifference(int64_t(morton_lod_levels[neighbours[i]]), int64_t(morton_lod_levels[index])) > 1)
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }


    auto TileStart(uint32_t index, uint32_t level)
    {
        // remove the insignificant part
        index = index >> (level * 2);
        // move back the significant part
        index = index << (level * 2);
        return index;
    };


    constexpr auto TileEdgeLength(uint32_t level)
    {
        return (1u << level);
    }


    constexpr auto TileSize(uint32_t level)
    {
        return TileEdgeLength(level) * TileEdgeLength(level);
    }
}


void Graphics::CalculateLODLevels(
    // position of the highest detail
    Math::Float3 position,
    Math::Float3 terrain_center,
    Range<float const *> lod_distances,
    Math::Float2 patch_size,
    Range<uint32_t *> morton_lod_levels
    )
{
    auto count = uint32_t(Size(morton_lod_levels));
    // adjust for the corner of the terrain
    auto terrain_start = TerrainStartPosition(terrain_center, patch_size, count);
    auto relative_position = position - terrain_start;
    // subtract half a patch size so we can use the corner points of the patches as if they're the center
    relative_position -= Float3(patch_size.x, patch_size.y, 0) / 2;
    auto levels = Size(lod_distances);
    for( auto i = 0u; i < count; )
    {
        auto index2d = Index2DFromMorton(i);
        auto patch_position = Float2FromUnsigned2(index2d) * patch_size;
        auto squared_distance = SquaredDistance(Float3(patch_position.x, patch_position.y, 0), relative_position);
        // Do it temporarily like this:
        // auto squared_distance = GetMaxElement(Float3(SquaredDistance(patch_position.x, relative_position.x), SquaredDistance(patch_position.y, relative_position.y), relative_position.z * relative_position.z));
        uint32_t level = 0;
        for( auto d = 0u; d < levels; ++d )
        {
            level += (lod_distances[d] * lod_distances[d]) < squared_distance;
        }

        // figure out where to start filling
        auto start = TileStart(i, level);
        auto element_count = TileSize(level);
        Fill(level, begin(morton_lod_levels) + start, element_count);
        // move i to the end of the whole block we just filled
        i = start + element_count;
    }

    //OutputDebugStringA(Math::ToString2D(morton_lod_levels).c_str());
    //OutputDebugStringA("\n");
    // assert(MaxOneDifferenceBetweenLODs(morton_lod_levels));

}


std::array<bool, 4> Graphics::CalculateStichEdge(Unsigned2 index2d, Unsigned2 total_size, uint32_t current_level, Range<uint32_t const*> lod_levels)
{
    auto tile_edge_length = TileEdgeLength(current_level);
    // only go back if we're not at the edge already
    Unsigned2 back = {index2d.x - (index2d.x != 0), index2d.y - (index2d.y != 0)};
    // only go forward if we're not at the edge already
    Unsigned2 forward = index2d + tile_edge_length;
    forward -= {forward.x == total_size.x, forward.y == total_size.y};
    return{{
        lod_levels[MortonIndex32Bit({back.x, index2d.y})] > current_level,
        lod_levels[MortonIndex32Bit({forward.x, index2d.y})] > current_level,
        lod_levels[MortonIndex32Bit({index2d.x, back.y})] > current_level,
        lod_levels[MortonIndex32Bit({index2d.x, forward.y})] > current_level,
    }};
}


void Graphics::InitializePatchData(
    Device& device,
    DeviceContext& device_context,
    TextureContainer& texture_container,
    ResourceViewContainer& resource_view_container,
    ConstantBufferContainer& constant_buffer_container,
    Terrain2DData& terrain_data
    )
{
    CalculateLODLevels(
        terrain_data.center,
        terrain_data.center,
        terrain_data.lod_distances,
        terrain_data.patch_size,
        terrain_data.lod_levels
        );

    auto count = uint32_t(Size(terrain_data.lod_levels));
    auto terrain_start = TerrainStartPosition(terrain_data);
    for( auto i = 0u; i < count; )
    {
        auto level = terrain_data.lod_levels[i];
        auto start = TileStart(i, level);
        auto index2d = Index2DFromMorton(start);
        auto patch_position = Float2FromUnsigned2(index2d) * terrain_data.patch_size;
        auto patch_position3d = terrain_start + Float3(patch_position.x, patch_position.y, 0);

        auto& patch_data = terrain_data.patch_data[start];
        // check if the neighbouring levels are the different
        auto stitch_edge = CalculateStichEdge(index2d, terrain_data.total_size, level, terrain_data.lod_levels);
        CreateBlock(
            patch_position3d,
            terrain_data.patch_size,
            terrain_data.patch_dimensions,
            level,
            stitch_edge,
            terrain_data.sample_function,
            device,
            device_context,
            texture_container,
            resource_view_container,
            constant_buffer_container,
            patch_data.texture_id,
            patch_data.constant_buffer_id,
            patch_data.bounding_box
            );

        // move i to the end of the whole block we just filled
        i = start + TileSize(level);
    }
}


void Graphics::AddRenderComponents(
    Terrain2DData const & terrain,
    RenderComponentContainer& container
    )
{
    RenderComponent component;
    component.mesh.vertex_ids.fill(c_invalid_vertex_buffer_id);
    component.mesh.index_id = terrain.graphics.universal_patch_index_buffer;

    component.textures.reserve(Size(terrain.graphics.global_textures) + 1);
    component.textures = terrain.graphics.global_textures;
    component.textures.emplace_back(); // for the height texture

    component.buffers.reserve(2); // terrain 2x
    component.buffers.emplace_back(ConstantBufferType::Terrain2D, terrain.graphics.persistent_terrain_data_buffer);
    component.buffers.emplace_back(); // for the patch terrain buffer

    assert(Size(terrain.lod_levels) == Size(terrain.patch_data));
    auto count = Size(terrain.lod_levels);
    for( auto i = 0u; i < count; )
    {
        auto level = terrain.lod_levels[i];
        auto start = TileStart(i, level);
        auto& patch_data = terrain.patch_data[start];

        Last(component.textures) = patch_data.texture_id;
        Last(component.buffers) = {ConstantBufferType::Terrain2DPatch, patch_data.constant_buffer_id};
        AddRenderComponent(
            terrain.entity_id,
            component,
            terrain.graphics.display_technique,
            terrain.graphics.shadow_technique,
            patch_data.bounding_box,
            terrain.graphics.alpha_to_coverage,
            {{c_invalid_vertex_buffer_id, c_invalid_vertex_buffer_id}},
            Math::Identity(),
            container);

        // move i to the end of the whole block we just processed
        i = start + TileSize(level);
    }
}



void Graphics::UpdatePatchData(
    Math::Float3 position,
    Device& device,
    DeviceContext& device_context,
    TextureContainer& texture_container,
    ResourceViewContainer& resource_view_container,
    ConstantBufferContainer& constant_buffer_container,
    Terrain2DData& terrain_data
    )
{
    auto count = uint32_t(Size(terrain_data.lod_levels));
    std::vector<uint32_t> new_lod_levels(count);
    CalculateLODLevels(
        position,
        terrain_data.center,
        terrain_data.lod_distances,
        terrain_data.patch_size,
        new_lod_levels
        );

    auto terrain_start = TerrainStartPosition(terrain_data);
    for( auto i = 0u; i < count; )
    {
        auto old_level = terrain_data.lod_levels[i];
        auto new_level = new_lod_levels[i];
        auto new_start = TileStart(i, new_level);
        auto index2d = Index2DFromMorton(new_start);
        // check if the neighbouring levels are the different
        auto new_stitch_edge = CalculateStichEdge(index2d, terrain_data.total_size, new_level, new_lod_levels);
        if (old_level != new_level)
        {
            // first remove the old stuff in the area of the new tile
            for( auto j = 0u; j < TileSize(new_level); ++j )
            {
                auto& patch_data = terrain_data.patch_data[j + new_start];
                texture_container.RemoveTexture2D(patch_data.texture_id.data_id);
                resource_view_container.RemoveShaderResourceView(patch_data.texture_id.view_id);
                constant_buffer_container.Remove(patch_data.constant_buffer_id);
            }
            // then proceed as normal
            auto patch_position = Float2FromUnsigned2(index2d) * terrain_data.patch_size;
            auto patch_position3d = terrain_start + Float3(patch_position.x, patch_position.y, 0);

            auto& patch_data = terrain_data.patch_data[new_start];
            CreateBlock(
                patch_position3d,
                terrain_data.patch_size,
                terrain_data.patch_dimensions,
                new_level,
                new_stitch_edge,
                terrain_data.sample_function,
                device,
                device_context,
                texture_container,
                resource_view_container,
                constant_buffer_container,
                patch_data.texture_id,
                patch_data.constant_buffer_id,
                patch_data.bounding_box
                );
        }
        else // check if we have to update the buffer due to a change in the neighbouring lod levels
        {
            auto old_stitch_edge = CalculateStichEdge(index2d, terrain_data.total_size, old_level, terrain_data.lod_levels);
            if(old_stitch_edge != new_stitch_edge)
            {
                auto patch_position = Float2FromUnsigned2(index2d) * terrain_data.patch_size;
                auto patch_position3d = terrain_start + Float3(patch_position.x, patch_position.y, 0);
                auto& patch_data = terrain_data.patch_data[new_start];
                UpdatePatchBuffer(
                    patch_position3d,
                    terrain_data.patch_dimensions,
                    new_level,
                    new_stitch_edge,
                    constant_buffer_container.GetBuffer(patch_data.constant_buffer_id),
                    device_context
                    );
            }
        }
        // always move i to the end of the whole new block
        i = new_start + TileSize(new_level);
    }
    // store the new lod levels
    terrain_data.lod_levels = std::move(new_lod_levels);
}


Math::Float3 Graphics::TerrainStartPosition(Math::Float3 center, Math::Float2 patch_size, uint32_t patch_count)
{
    auto terrain_size = Float2FromUnsigned2(Index2DFromMorton(patch_count - 1) + 1) * patch_size;
    auto terrain_start = center - Float3(terrain_size.x, terrain_size.y, 0) / 2;
    return terrain_start;
}


Math::Float3 Graphics::TerrainStartPosition(Terrain2DData const & data)
{
    auto count = uint32_t(Size(data.lod_levels));
    return TerrainStartPosition(data.center, data.patch_size, count);
}


void Graphics::CalculateGrassOrientations(Math::Float3 start, Math::Float2 size, Get2DSampleFunctionType const & sample_function, uint32_t seed, Range<Orientation *> orientations)
{
    // Seed random generator with position and seed
    seed ^= reinterpret_cast<uint32_t&>(start.x) ^ reinterpret_cast<uint32_t&>(start.y) ^ reinterpret_cast<uint32_t&>(start.z);
    std::default_random_engine random_engine(seed);

    std::uniform_real_distribution<float> distrubution_x(0, size.x);
    std::uniform_real_distribution<float> distrubution_y(0, size.y);
    std::uniform_real_distribution<float> rotation_distribution(0, Math::c_2PI);


    auto count = Size(orientations);
    for (auto i = 0u; i < count; ++i)
    {
        Math::Float3 position = start;
        position.x += distrubution_x(random_engine);
        position.y += distrubution_y(random_engine);
        Float2 gradient;
        float height;
        sample_function(Math::Float2(position.x, position.y), height, gradient);
        position.z += height;
        auto normal = Normalize(Float3(-gradient.x, -gradient.y, 1.f));
        // Calculate orientation, to be orthogonal to face
        auto rotation = RotationBetweenNormals(Math::Float3(0, 0, 1), normal);
        // Add random rotation around own z axis
        rotation *= Math::ZAngleToQuaternion(rotation_distribution(random_engine));
        orientations[i] = {position, rotation};
    }
}


namespace
{
    void CalculateGrassOrientationsForTerrain(
        Math::Float2 patch_size,
        Math::Float3 terrain_center,
        uint32_t seed,
        Get2DSampleFunctionType const & sample_function,
        Range<float const *> grass_lod_densities,
        Range<uint32_t const *> grass_lod_levels,
        std::vector<Orientation> & grass_orientations
        )
    {
        auto patch_area = Product(patch_size);
        auto block_count = uint32_t(Size(grass_lod_levels));
        auto terrain_start = TerrainStartPosition(terrain_center, patch_size, block_count);
        for (auto i = 0u; i < block_count; ++i)
        {
            auto lod_level = grass_lod_levels[i];
            auto grass_density = GetOptional(grass_lod_densities, lod_level, 0.f);
            auto grass_count = uint32_t(Math::Round(grass_density * patch_area));
            if(grass_count > 0)
            {
                auto index2d = Index2DFromMorton(i);
                auto patch_position = Float2FromUnsigned2(index2d) * patch_size;
                auto patch_position3d = terrain_start + Float3(patch_position.x, patch_position.y, 0);
                auto patch_orientations = Grow(grass_orientations, grass_count);
                CalculateGrassOrientations(patch_position3d, patch_size, sample_function, seed, patch_orientations);
            }
        }
    }
}


void Graphics::UpdateGrassBuffers(
    Math::Float3 position,
    uint32_t batch_size,
    Terrain2DData & terrain,
    VertexBufferContainer & vertex_buffer_container,
    Device& device,
    DeviceContext& device_context
    )
{
    std::vector<uint32_t> new_grass_lod_levels(Math::Product(terrain.total_size));
    std::vector<Orientation> grass_orientations;
    typedef HLSL::float3x4 ShaderAffineTransformType;

    auto& grasses = terrain.grass;
    auto count = Size(grasses);
    for (auto i = 0u; i < count; ++i)
    {
        uint32_t data_offsets[2] = {terrain.grass_lod_data_offsets[i], terrain.grass_lod_data_offsets[i + 1]};
        CalculateLODLevels(
            position,
            terrain.center,
            CreateRange(terrain.grass_lod_distances, data_offsets[0], data_offsets[1]),
            terrain.patch_size,
            new_grass_lod_levels
            );

        auto& grass = grasses[i];
        if(!Math::Equal(grass.lod_levels, new_grass_lod_levels))
        {
            swap(grass.lod_levels, new_grass_lod_levels);

            grass_orientations.clear();
            auto seed = i;
            CalculateGrassOrientationsForTerrain(
                terrain.patch_size,
                terrain.center,
                seed,
                terrain.sample_function,
                CreateRange(terrain.grass_lod_densities, data_offsets[0], data_offsets[1]),
                grass.lod_levels,
                grass_orientations
                );

            auto grass_count = uint32_t(Size(grass_orientations));
            auto no_buffers = grass.graphics.instance_buffers.empty();
            auto make_new_buffer = no_buffers;
            if(!no_buffers)
            {
                D3D11_BUFFER_DESC buffer_description;
                auto buffer = vertex_buffer_container.GetBuffer(First(grass.graphics.instance_buffers));
                buffer->GetDesc(&buffer_description);
                make_new_buffer = buffer_description.ByteWidth < grass_count * sizeof(ShaderAffineTransformType);
            }
            VertexBufferInfo buffer_info;
            buffer_info.type = VertexBufferType::WorldMatrix;
            buffer_info.vertex_count = Math::Min(batch_size, grass_count);
            buffer_info.offset = 0;
            VertexBufferID instance_buffer_id = c_invalid_vertex_buffer_id;
            if(make_new_buffer)
            {
                vertex_buffer_container.Remove(grass.graphics.instance_buffers);
                grass.graphics.instance_buffers.clear();
                auto transform_buffer = device.CreateVertexBuffer(uint32_t(sizeof(ShaderAffineTransformType) * grass_count));
                instance_buffer_id = vertex_buffer_container.Add(buffer_info, transform_buffer);
                grass.graphics.instance_buffers.emplace_back(instance_buffer_id);
            }
            else if(!no_buffers)
            {
                assert(grass_count > 0);
                assert(!grass.graphics.instance_buffers.empty());
                vertex_buffer_container.Remove(CreateRange(grass.graphics.instance_buffers, 1, Size(grass.graphics.instance_buffers)));
                grass.graphics.instance_buffers.resize(1);
                instance_buffer_id = First(grass.graphics.instance_buffers);
                vertex_buffer_container.UpdateInfo(buffer_info, instance_buffer_id);
            }
            grass.batch_bounds.clear();
            auto first_batch_bounds = Merge(grass.original_bounds, CreateRange(grass_orientations, 0, buffer_info.vertex_count));
            grass.batch_bounds.emplace_back(first_batch_bounds);
            for (auto offset = batch_size; offset < grass_count; offset += batch_size)
            {
                buffer_info.vertex_count = Math::Min(batch_size, grass_count - offset);
                buffer_info.offset = offset * sizeof(ShaderAffineTransformType);
                auto new_instance_buffer_id = vertex_buffer_container.Add(buffer_info, instance_buffer_id);
                grass.graphics.instance_buffers.emplace_back(new_instance_buffer_id);
                auto batch_bounds = Merge(grass.original_bounds, CreateRange(grass_orientations, offset, offset + buffer_info.vertex_count));
                grass.batch_bounds.emplace_back(batch_bounds);
            }


            auto buffer = vertex_buffer_container.GetBuffer(instance_buffer_id);
            FillConstantBuffer<ShaderAffineTransformType>(buffer, device_context,[&grass_orientations](auto data)
            {
                for (auto i = 0u; i < Size(grass_orientations); ++i)
                {
                    data[i] = AffineTransform(grass_orientations[i].position, grass_orientations[i].rotation);
                }
            });
        }
    }

}


void Graphics::AddGrassRenderComponents(Terrain2DData const & terrain, RenderComponentContainer& container)
{
    auto& grasses = terrain.grass;
    auto count = Size(grasses);
    for (auto i = 0u; i < count; ++i)
    {
        auto& grass = grasses[i];
        RenderComponent component;
        component.mesh = grass.graphics.mesh;
        component.textures = grass.graphics.textures;
        component.buffers.emplace_back(ConstantBufferType::Object, grass.graphics.object_buffer);

        assert(Size(grass.graphics.instance_buffers) == Size(grass.batch_bounds));
        for (auto b = 0u; b < Size(grass.batch_bounds); ++b)
        {
            AddRenderComponent(
                terrain.entity_id,
                component,
                grass.graphics.display_technique,
                grass.graphics.shadow_technique,
                grass.batch_bounds[b],
                grass.graphics.alpha_to_coverage,
                {{grass.graphics.instance_buffers[b], c_invalid_vertex_buffer_id}},
                Math::Identity(),
                container);
        }
    }
}
