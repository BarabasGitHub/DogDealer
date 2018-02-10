#pragma once

#include "Structures.h"
#include "DeviceContext.h"
#include "2DTerrainConfiguration.h"

#include <Math\FloatTypes.h>
#include <Math\IntegerTypes.h>
#include <Utilities\Range.h>
#include <Utilities\MinMax.h>

#include <cstdint>
#include <utility> // for pair
#include <string>

struct Orientation;

namespace Graphics
{
    enum struct TextureType;
    struct Device;
    struct IndexBufferContainer;
    struct TextureContainer;
    struct VertexBufferContainer;
    struct ConstantBufferContainer;
    struct ResourceViewContainer;
    class ResourceManager;
    struct RenderComponentDescription;
    struct Terrain2DData;
    struct RenderComponentContainer;

    Math::Float3 TerrainStartPosition(Math::Float3 center, Math::Float2 patch_size, uint32_t patch_count);
    Math::Float3 TerrainStartPosition(Terrain2DData const & data);

    uint32_t VertexCountForMeshOn2DGrid(Math::Unsigned2 size);

    uint32_t TriangleCountForMeshOn2DGrid(Math::Unsigned2 size);

    void GenerateTriangleIndices(Math::Unsigned2 size, Range<uint32_t  *> triangle_vertex_indices);

    void Generate3DMeshOn2DGrid(Math::Float2 center, Math::Float2 extent, Math::Unsigned2 size, Range<Math::Float3 *> vertex_positions, Range<uint32_t*> triangle_vertex_indices);

    void GenerateHeightAndNormals(Get2DSampleFunctionType const & sample_function, Range<Math::Float3*> vertex_positions, Range<Math::Float3*> normals);

    struct NormalAndHeight
    {
        Math::Float3 normal;
        float height;
    };

    // returns the min and max height
    MinMax<float> GenerateHeightAndNormals(Math::Float2 start, Math::Float2 size, Math::Unsigned2 dimensions, Get2DSampleFunctionType const & sample_function, Range<NormalAndHeight*> data);

    TextureTypeAndID CreateNormalAndHeightTexture(
        Math::Unsigned2 dimensions,
        Range<NormalAndHeight const*> normals_and_heights,
        DeviceContext& device_context,
        Device& device,
        TextureContainer& texture_container,
        ResourceViewContainer& resource_view_container);

    TextureTypeAndID CreateNormalAndHeightTexture(
        Math::Unsigned2 dimensions,
        Device& device,
        TextureContainer& texture_container,
        ResourceViewContainer& resource_view_container);

    void UpdateNormalAndHeightTexture(
        Math::Unsigned2 patch_size,
        Math::Unsigned2 patch_offset,
        Range<NormalAndHeight const*> normals_and_heights,
        DeviceContext& device_context,
        ID3D11Texture2D* texture
        );

    IndexBufferID MakeBlockIndexBuffer(Math::Unsigned2 dimensions, Device& device, IndexBufferContainer& index_buffer_container);

    std::array<bool, 4> CalculateStichEdge(Math::Unsigned2 index2d, Math::Unsigned2 total_size, uint32_t current_level, Range<uint32_t const*> lod_levels);


    void UpdatePatchBuffer(
        Math::Float3 start3d,
        Math::Unsigned2 dimensions,
        uint32_t lod_level,
        std::array<bool, 4> stitch_edge,
        ID3D11Buffer* constant_buffer,
        DeviceContext& device_context
        );

    void CreateBlock(
        Math::Float3 start,
        Math::Float2 extent,
        Math::Unsigned2 dimensions,
        uint32_t lod_level,
        // lower x, upper x, lower y, upper y; can only stitch either lower or upper per dimension
        std::array<bool, 4> stitch_edge,
        Get2DSampleFunctionType const & sample_function,
        Device& device,
        DeviceContext& device_context,
        TextureContainer& texture_container,
        ResourceViewContainer& resource_view_container,
        ConstantBufferContainer& constant_buffer_container,
        TextureTypeAndID& height_texture_info,
        ConstantBufferID& constant_buffer,
        BoundingShapes::AxisAlignedBox& bounding_box);

    void InitializeTerrainData(
        // input forms
        EntityID entity_id,
        Math::Float3 terrain_center,
        Range<float const *> lod_distances,
        Get2DSampleFunctionType get_sample_function,
        Math::Unsigned2 patch_dimensions,
        Math::Float2 patch_size,
        // size in highest resolution patches
        Math::Unsigned2 total_size,
        RenderComponentDescription const & component_description,
        // overpaid managers
        ResourceManager& resource_manager,
        ConstantBufferContainer& constant_buffer_container,
        TextureContainer& texture_container,
        ResourceViewContainer& resource_view_container,
        IndexBufferContainer& index_buffer_container,
        Device& device,
        DeviceContext& device_context,
        // actual output
        Terrain2DData& terrain_data
        );


    void CalculateLODLevels(
        // position of the highest detail
        Math::Float3 position,
        Math::Float3 terrain_center,
        Range<float const *> lod_distances,
        Math::Float2 patch_size,
        Range<uint32_t *> morton_lod_levels
        );


    void InitializePatchData(
        Device& device,
        DeviceContext& device_context,
        TextureContainer& texture_container,
        ResourceViewContainer& resource_view_container,
        ConstantBufferContainer& constant_buffer_container,
        Terrain2DData& terrain_data
        );


    void UpdatePatchData(
        Math::Float3 position,
        Device& device,
        DeviceContext& device_context,
        TextureContainer& texture_container,
        ResourceViewContainer& resource_view_container,
        ConstantBufferContainer& constant_buffer_container,
        Terrain2DData& terrain_data
        );


    void AddRenderComponents(
        Terrain2DData const & terrain,
        RenderComponentContainer& container
        );


    void InitializeGrassData(
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
        std::vector<float> & grass_lod_counts,
        std::vector<float> & grass_lod_distances
        );

    // Calculates the grass orientations for one square part of the terrain
    void CalculateGrassOrientations(Math::Float3 start, Math::Float2 size, Get2DSampleFunctionType const & sample_function, uint32_t seed, Range<Orientation *> orientations);

    void UpdateGrassBuffers(
        Math::Float3 position,
        uint32_t batch_size,
        Terrain2DData & terrain,
        VertexBufferContainer & vertex_buffer_container,
        Device& device,
        DeviceContext& device_context
        );

    void AddGrassRenderComponents(Terrain2DData const & terrain, RenderComponentContainer& container);
}
