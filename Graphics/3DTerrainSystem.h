#pragma once
#include "3DTerrainSystemStructs.h"

#include <vector>
#include <array>

#include <Conventions\Orientation.h> // For grass generation
#include <Utilities\Range.h>

namespace Graphics{

	struct Device;

	struct VertexBufferContainer;
	struct IndexBufferContainer;


    //void GenerateTerrainBlocks(Get3DSampleFunctionType get_sample, Math::Float3 const terrain_center, std::vector<unsigned> const & block_indices, std::vector<unsigned> const & block_lod_levels, TerrainBlockParameters& block_parameters, float const block_dimensions, std::vector<VertexData>& vertex_data_vector, std::vector<Orientation>& new_grass_orientations);
    void GenerateNewLODMeshes(Terrain3DData & terrain,
                            std::vector<unsigned> const & block_indices,
                            IndexBufferContainer& index_container,
                            VertexBufferContainer& vertex_container,
                            Device& device);

    void InitializeTerrain3DData(Math::Float3 const & terrain_center,
                            float const update_distance,
                            Math::Unsigned3 const & terrain_block_count,
                            Math::Float3 const & terrain_block_dimensions,
                            Math::Unsigned3 const & terrain_block_cube_count,
                            std::vector<float> const & degradation_thresholds,
                            Get3DSampleFunctionType get_sample,
                            RenderComponentDescription const & render_component_description,
                            EntityID const & terrain_entity_id,
                            Terrain3DData& terrain);

    void ExtractDisplayMeshes(std::vector<TerrainBlockMeshes> const & meshes,
                            std::vector<GrassBufferVector> const & grass_buffers,
                            std::vector<GrassBoundsVector> const & grass_buffer_bounds,
                            std::vector<unsigned> const & block_data_indices,
                            std::vector<unsigned> const & lod_levels,
                            std::vector<MeshData> & output_meshes,
                            std::vector<GrassBufferVector> & output_grass_buffers,
                            std::vector<GrassBoundsVector>& output_grass_buffer_bounds );

    void UpdateTerrainBlocks(Math::Float3 const & position,
                            Terrain3DData& terrain,
                            std::vector<MeshData>& display_meshes,
                            std::vector<GrassBufferVector>& display_grass_buffers,
                            std::vector<GrassBoundsVector>& display_grass_buffer_bounds,
                            std::vector<MeshData>& unload_meshes,
                            std::vector<VertexBufferID>& unload_instance_buffers,
                            IndexBufferContainer& index_container,
                            VertexBufferContainer& vertex_container,
                            Device& device);


    void SampleBlockGrid(
        Math::Unsigned3 grid_size,
        Math::Float3 point_offset,
        Math::Float3 cube_scale,
        Get3DSampleFunctionType const & get_sample,
        Range<DensityAndGradient*> densities_and_gradients);
}