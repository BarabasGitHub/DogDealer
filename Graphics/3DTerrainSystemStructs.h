#pragma once

#include "Structures.h" // For mesh data in TerrainBlockMeshes
#include "ResourceDescriptions.h"

#include <Conventions\EntityID.h>
#include <FileLayout\VertexDataType.h>

#include <vector>
#include <functional>

namespace Graphics{

    float const c_surface_threshold = 0.0f;

    unsigned const c_lod_count = 5;

	typedef std::function<void(Math::Float3 const position, float & density, Math::Float3 & gradient)> Get3DSampleFunctionType;

	struct DensityAndGradient
	{
		Math::Float3 gradient;
		float density;
	};

	struct VertexData{
		std::vector<PositionType>	positions;
		std::vector<NormalType>		normals;
		std::vector<unsigned>		indices;

        void Clear()
        {
            positions.clear();
            normals.clear();
            indices.clear();
        }
	};

    // Meshes for each lod level
    struct TerrainBlockMeshes{

        std::array<MeshData, c_lod_count> lods;
        std::array<bool, c_lod_count> empty;
    };

    // Store cube count and sizes for each lod level
    struct TerrainBlockParameters
    {
        std::vector<Math::Unsigned3> block_cube_counts;
        std::vector<Math::Float3> block_cube_scales;
    };

    // Store description and count of grass as spawned on all terrain blocks
    // (single lookup used for all blocks, not one entry per block)
    struct GrassParameters
    {
        std::vector<Graphics::RenderComponentDescription> descriptions;
        std::vector<std::array<float, c_lod_count>> densities;

        std::vector<BoundingShapes::AxisAlignedBox> bounds;
    };

    typedef std::vector<std::array<VertexBufferID,2>> GrassBufferVector;
    typedef std::vector<BoundingShapes::AxisAlignedBox> GrassBoundsVector;

    struct Terrain3DData
    {

        // Density function
        Get3DSampleFunctionType get_sample;

        // Render component description of terrain block meshes
        RenderComponentDescription block_render_description;
        EntityID terrain_entity_id = c_invalid_entity_id;

        // Center of terrain
        Math::Float3 sample_offset;
        Math::Float3 real_center;

        // Terrain Block layout
        Math::Unsigned3 block_count;
        Math::Float3 block_dimensions;
        TerrainBlockParameters block_parameters;


        // Required distance of player to center
        // to check for terrain shift
        float update_distance;


        // Per-block data:
        // Data indices for i-th block, used for meshes and grass_instance_buffers
        std::vector<unsigned> block_data_indices;

        std::vector<unsigned> lod_levels;

        // Meshes used by block i: meshes[mesh_indices[i]]
        std::vector<TerrainBlockMeshes> meshes;

        // Instanced grass transforms by block i: grass_buffers[mesh_indices[i]]
        // parallel to meshes vector
        // [0] world matrix
        // [1] color
        std::vector<GrassBufferVector> grass_instance_buffers;
        std::vector<GrassBoundsVector> grass_buffer_bounds;

        // Meshes being rendered
        //std::vector<MeshData> displayed_meshes;

        // Vegetation generated on tiles
        GrassParameters grass_parameters;
    };


    // Collection of input parameters used by Marching Cubes for:
    //  -Mesh generation
    //  -Skirt mesh generation
    struct MarchingCubesParameters{

        std::vector<DensityAndGradient> densities_and_gradients;

        Math::Float3 point_offset;
        Math::Unsigned3 grid_size;

        Math::Float3 cube_scale;
        Math::Unsigned3 cube_counts;
    };

}
