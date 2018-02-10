#pragma once
#include "3DTerrainSystem.h"

#include "MarchingCubes.h"
#include "3DTerrainSystemSkirting.h"

#include "Device.h"

#include "VertexBufferContainer.h"
#include "IndexBufferContainer.h"

#include "FillConstantBuffer.h" // for grass buffers
#include "HLSLTypes.h"          // for grass buffers

#include <BoundingShapes\Triangle.h>
#include <BoundingShapes\TriangleFunctions.h>
#include <BoundingShapes\AxisAlignedBoxFunctions.h>

#include <Math\FloatOperators.h>
#include <Math\TransformFunctions.h>
#include <Math\IntegerOperators.h>

#include <Utilities\IndexUtilities.h>

#include <cassert>

#include <Utilities\HRTimer.h>
#include <Utilities\Logger.h>
#include <Math\Conversions.h>

#include "MeshFunctions.h"

//#include <sstream>
#include <random>


namespace
{
    using namespace Graphics;


    // Use the input transforms to create an array of VertexBufferIDs on the VertexBufferContainer
    // First buffer handle to stored transforms
    // Second buffer handle to colors
    void CreateInstanceBuffer(std::vector<HLSL::float3x4> const & transforms,
                            std::array<VertexBufferID, 2>& buffer_handles,
                            VertexBufferContainer& vertex_container,
                            Device& device)
    {
        // CREATE WORLD MATRIX BUFFER
        {
            auto world_buffer = device.CreateVertexBuffer(transforms);

            // Store on VertexBufferContainer
            VertexBufferInfo buffer_info;
            buffer_info.type = VertexBufferType::WorldMatrix;
            buffer_info.vertex_count = uint32_t(transforms.size());
            buffer_info.offset = 0;

            buffer_handles[0] = vertex_container.Add(buffer_info, world_buffer);
        }

        buffer_handles[1] = c_invalid_vertex_buffer_id;
    }


    Math::Float3 CalculateTerrainShift(Math::Float3 const offset,
                                       Math::Float3 const block_edge_length)
    {
        float min_distance = std::numeric_limits<float>::max();
        auto closest_center = Math::Float3(0.0f, 0.0f, 0.0f);

        // Find closest relative block center
        for (auto x = -1; x <= 1; x++)
        {
            for (auto y = -1; y <= 1; y++)
            {
                for (auto z = -1; z <= 1; z++)
                {
                    // Get relative block center
                    auto block_center = Math::Float3(float(x), float(y), float(z));
                    block_center *= block_edge_length;

                    // Get distance between player position and block center
                    auto distance = Norm(offset - block_center);

                    // Compare to known minimum
                    if (distance < min_distance)
                    {
                        min_distance = distance;
                        closest_center = block_center;
                    }
                }
            }
        }

        // Return relative coordinates of closest block center
        return closest_center;
    }


    uint32_t CalculateLODLevel(Math::Float3 const offset, std::vector<float> degradation_thresholds)
    {
        uint32_t lod_level = 0;

        // Keep increasing the lod level until below distance threshold
        for (auto l = 0u; l < degradation_thresholds.size(); l++)
        {
            auto level_threshold = degradation_thresholds[l];

            auto norm = Norm(offset);
            bool increase_level = norm > level_threshold;

            if (!increase_level) break;

            ++lod_level;
        }

        return lod_level;
    }

    // For all lod meshes of a given block
    //  -clear the empty flags
    //  -set meshes to invalid
    void ResetTerrainBlockMeshes(TerrainBlockMeshes & meshes)
    {
        // Clear all existing meshes for block
        for (auto j = 0u; j < meshes.lods.size(); j++)
        {
            meshes.empty[j] = false;
            meshes.lods[j].mesh.index_id.index = c_invalid_vertex_buffer_id.index;
        }
    }

    // As none of the meshes are used anymore, add their MeshDatas to cleared_meshes
    void PruneBlockMesh(TerrainBlockMeshes const & terrain_block_meshes,
                        std::vector<MeshData> & unload_meshes)
    {
        // Loop over all lod meshes
        for (auto& mesh_data : terrain_block_meshes.lods)
        {
            // Skip if mesh for lod was never used
            if (mesh_data.mesh.index_id.index != c_invalid_vertex_buffer_id.index)
            {
                unload_meshes.push_back(mesh_data);
            }
        }
    }

    uint32_t CalculateShiftedIndex(uint32_t const index, Math::Int3 const step, Math::Unsigned3 const block_count)
    {
        // Convert original index to 3d
        Math::Unsigned3 index_3d = Calculate3DindexFrom1D(index, block_count.x, block_count.y);

        // Check for new index being out of scope
        for (auto i = 0u; i < 3; i++)
        {
            index_3d[i] += step[i];

            // Overflow on negative or positive out of scope
            if (index_3d[i] >= block_count[i]) return uint32_t(-1);
        }

        // Convert result to 1d
        uint32_t result_index = Calculate1DindexFrom3D(index_3d, block_count.x, block_count.y);

        return result_index;
    }

    // Map existing terrain blocks to shifted terrain block storage
    void CreateShiftedMapping(
        Math::Float3 const step,
        Math::Float3 block_dimensions,
        Math::Unsigned3 block_count,
        Range<uint32_t*> mapping)
    {
        // Calculate offsets in block dimensions (getting -1, 0 or 1)
        Math::Int3 shift = Int3FromFloat3(-step / block_dimensions);

        // Update all blocks
        auto size = Size(mapping);
        for (auto i = 0u; i < size; i++)
        {
            mapping[i] = CalculateShiftedIndex(i, shift, block_count);
        }
    }

    // Execute the block shift by assiging the meshes of the current block to the target block
    // If the mesh at the target lod level does not exist yet and is not empty, mark it to be loaded
    void ShiftBlockMesh(uint32_t const & current_index,
                        uint32_t const & target_index,
                        std::vector<uint32_t> const & previous_block_data_indices,
                        std::vector<uint32_t> const & lod_levels,
                        std::vector<uint32_t> & new_block_data_indices,
                        std::vector<TerrainBlockMeshes> const & terrain_block_meshes,
                        std::vector<uint32_t> & load_blocks,
                        std::vector<bool> & block_was_updated)
    {
        // Assign the mesh used by the current block to the target block
        auto current_block_data_index = previous_block_data_indices[current_index];
        new_block_data_indices[target_index] = current_block_data_index;

        // Get the lod level that will be shown at the target position
        auto target_lod_level = lod_levels[target_index];

        // Get the shifted meshes
        auto& new_mesh_lods = terrain_block_meshes[current_block_data_index];

        // Get 'empty' flag and mesh_data index for shifted mesh at target lod
        auto lod_mesh_is_empty = new_mesh_lods.empty[target_lod_level];
        auto lod_mesh_data = new_mesh_lods.lods[target_lod_level];

        // Mark mesh for loading if none exists for given LOD yet
        if (lod_mesh_data.mesh.index_id.index == c_invalid_vertex_buffer_id.index
            && lod_mesh_is_empty == false)
        {
            load_blocks.push_back(target_index);
        }

        // Mark mesh as having been updated
        block_was_updated[target_index] = true;
    }



    // Use the mapping to shift which terrain block mesh is used by which relative block
    // For each block with a valid mapped target index:
    //  -execute a shift by assigning the current blocks meshes to the target block
    //  -mark new meshes that are needed for loading (load_blocks)
    //
    // For each block with no target index:
    //  -meshes are not used anymore, mark for deletion (unload_meshes)
    void RemapTerrainBlocks(std::vector<uint32_t> const & mapping,
                            std::vector<uint32_t> const & previous_block_data_indices,
                            std::vector<uint32_t> const & lod_levels,
                            std::vector<uint32_t> & new_block_data_indices,
                            std::vector<TerrainBlockMeshes> const & terrain_block_meshes,
                            std::vector<uint32_t> & load_blocks,
                            std::vector<bool> & block_was_updated,
                            std::vector<uint32_t> & cleared_block_data_indices,
                            std::vector<MeshData> & unload_meshes,
                            std::vector<GrassBufferVector> & grass_instance_buffers,
                            std::vector<VertexBufferID>& unload_instance_buffers)
    {
        // Apply the mapping from the copy to current storage
        for (auto i = 0u; i < mapping.size(); i++)
        {
            // Get current and target index of block
            auto current_index = i;
            auto target_index = mapping[i];

            // Check if a mapping exists
            if (target_index != uint32_t(-1))
            {
                // Use the mapping to shift the block mesh to a new block
                ShiftBlockMesh(current_index,
                                target_index,
                                previous_block_data_indices,
                                lod_levels,
                                new_block_data_indices,
                                terrain_block_meshes,
                                load_blocks,
                                block_was_updated);
            }
            else
            {
                // Without a mapping the block has no place in the new terrain state
                // Mark all its used meshes to be unloaded
                auto block_data_index = previous_block_data_indices[i];
                PruneBlockMesh(terrain_block_meshes[block_data_index], unload_meshes);

                // Mark all its grass instance buffers to be unloaded
                for (auto& buffers : grass_instance_buffers[block_data_index])
                {
                    for (auto& buffer : buffers)
                    {
                        // If buffer is valid, mark for unloading and set invalid
                        if (buffer.index != c_invalid_vertex_buffer_id.index)
                        {
                            unload_instance_buffers.push_back(buffer);
                            buffer.index = c_invalid_vertex_buffer_id.index;
                        }
                    }
                }

                // Store its slot in the terrain meshes vector as cleared
                cleared_block_data_indices.push_back(block_data_index);
            }
        }
    }

    // Terrain blocks that were not updated can be assumed to having just entered scope
    // Assign a free mesh slot to each block, re-initialize the mesh and mark it as to be loaded
    void InitializeIncidentTerrainBlocks(std::vector<bool> const & block_was_updated,
                                        std::vector<uint32_t> & vacant_block_data_indices,
                                        std::vector<uint32_t> & new_block_data_indices,
                                        std::vector<uint32_t> & load_blocks,
                                        std::vector<TerrainBlockMeshes> & terrain_block_meshes)
    {
        // Initialize non-mapped and thus to be newly added blocks
        for (auto i = 0u; i < block_was_updated.size(); i++)
        {
            // Skip if block was updated
            if (block_was_updated[i]) continue;

            // Otherwise get a free slot in the terrain meshes vector
            auto block_data_index = vacant_block_data_indices.back();
            vacant_block_data_indices.pop_back();

            // Assign freed mesh slot to new block
            new_block_data_indices[i] = block_data_index;

            // Mark block mesh as to be loaded
            load_blocks.push_back(i);

            // Clear all existing meshes for block
            ResetTerrainBlockMeshes(terrain_block_meshes[block_data_index]);
        }
    }

    // Use the mapping to:
    //  -assign existing meshes to other blocks
    //  -store indices of blocks that require a mesh to be generated (load_blocks)
    //  -store mesh datas of blocks that are not used anymore (unload_meshes)
    void ApplyBlockMapping(std::vector<uint32_t> const & mapping,
                        Terrain3DData& terrain,
                        std::vector<MeshData>& unload_meshes,
                        std::vector<VertexBufferID>& unload_instance_buffers,
                        std::vector<uint32_t>& load_blocks)
    {
        auto block_count = terrain.block_data_indices.size();

        // Keep track of which blocks were updated with the mapping
        auto block_was_updated = std::vector<bool>(block_count, false);

        // Keep a copy of the mesh indices
        auto previous_block_data_indices = terrain.block_data_indices;

        // Keep track of the mesh slots of the terrain.meshes vector
        // that are freed up by unloading
        std::vector<uint32_t> cleared_block_data_indices;

        // Apply the mapping to all terrain blocks
        // Mark new block meshes which need to be loaded
        RemapTerrainBlocks(mapping,
                            previous_block_data_indices,
                           terrain.lod_levels,
                           terrain.block_data_indices,
                           terrain.meshes,
                           load_blocks,
                           block_was_updated,
                           cleared_block_data_indices,
                           unload_meshes,
                           terrain.grass_instance_buffers,
                           unload_instance_buffers);


        // Blocks that were not updated are just entering scope
        //  -Re-initialize all meshes
        //  -Mark visible lod as to be loaded
        InitializeIncidentTerrainBlocks(block_was_updated,
                                        cleared_block_data_indices,
                                        terrain.block_data_indices,
                                        load_blocks,
                                        terrain.meshes);
    }

    Math::Float3 GetBlockSampleStartPosition(uint32_t const block_index,
                                            Math::Unsigned3 const & block_counts,
                                            Math::Float3 const & terrain_center,
                                            Math::Float3 const block_dimensions)
    {
        // Get relative position of block center in block dimensions
        auto block_index_3d = Calculate3DindexFrom1D(block_index, block_counts.x, block_counts.y);
        auto offset = Math::Float3FromUnsigned3(block_index_3d);

        // Scale by block dimensions to get coordinates
        offset *= block_dimensions;

        // Center block around terrain center
        offset -= Math::Float3FromUnsigned3(block_counts / 2) * block_dimensions;

        // Make position absolute and determine -x,-y,-z corner for point_offset
        auto block_center = terrain_center + offset;
        auto point_offset = block_center - 0.5 * Math::Float3(block_dimensions);

        return point_offset;
    }

    // Generate a random position on the face formed by the input vertices
    // Create output orientation from position and respective face normal
    Orientation GetRandomOrientationOnFace(BoundingShapes::Triangle const & triangle, std::default_random_engine& random_engine)
    {
        Orientation result;
        result.position = Math::Float3(0.0f, 0.0f, 0.0f);

        // Assign an influence to each face vertex
        std::array<float, 3> influences;
        float influence_sum = 0.0f;

        auto minimal_start_influence = 0.1f;
        std::uniform_real_distribution<float> influence_distribution( minimal_start_influence, 1);
        // Calculate random influences for the vertices
        for (auto i = 0u; i < 3; i++)
        {
            // Generate random percentage
            influences[i] = influence_distribution( random_engine );
            influence_sum += influences[i];
        }

        // Calculate the position and normalize
        for (auto i = 0u; i < 3; i++)
        {
            result.position += (influences[i] / influence_sum) * triangle.corners[i];
        }

        // Calculate orientation, to be orthogonal to face
        auto normal = GetNormal(triangle);
        result.rotation = RotationBetweenNormals(Math::Float3(0, 0, 1), normal);

        // Add random rotation around own z axis
        std::uniform_real_distribution<float> rotation_distribution(0, Math::c_2PI);
        result.rotation *= Math::ZAngleToQuaternion(rotation_distribution(random_engine));

        return result;
    }

    // Generate a number (#grass_count) of random orientations on the given vertex data
    void GenerateGrassPlacements(VertexData const & vertex_data,
                                std::vector<Orientation>& result_orientations,
                                uint32_t const grass_count,
                                uint32_t const seed_offset)
    {
        if (vertex_data.positions.empty()) return;

        // Seed random generator with position and seed_offset
        auto first_position = vertex_data.positions.front();
        auto seed = reinterpret_cast<uint32_t&>(first_position.x) ^ reinterpret_cast<uint32_t&>(first_position.y) ^ reinterpret_cast<uint32_t&>(first_position.z);
        seed += seed_offset;

        std::default_random_engine random_engine(seed);
        auto face_count = uint32_t(vertex_data.indices.size() / 3);

        std::uniform_int_distribution<uint32_t> distrubution(0, face_count - 1);

        for (auto i = 0u; i < grass_count; i++)
        {
            // Get random face index
            auto face_index = 3 * distrubution(random_engine);
            BoundingShapes::Triangle triangle;

            for (auto j = 0; j < 3; j++)
            {
                // Get and store j-th vertex of face
                auto vertex_index = vertex_data.indices[face_index + j];
                // no complaining about uninitialized triangle, cause we're initializing it here...
#pragma warning(suppress: 6001)
                triangle.corners[j] = vertex_data.positions[vertex_index];
            }

            auto orientation = GetRandomOrientationOnFace(triangle, random_engine);
            result_orientations.push_back(orientation);
        }
    }

    // Use the input grass orientations to create a vertex buffer containing transforms
    void GenerateGrassBuffers(std::vector<Orientation> const & orientations,
                            std::array<VertexBufferID, 2> & buffer_handles,
                            VertexBufferContainer& vertex_container,
                            Device& device)
    {
        // Convert orientations to transforms
        auto transforms = std::vector<HLSL::float3x4>(orientations.size());
        std::transform(orientations.begin(), orientations.end(), transforms.begin(), [](Orientation const & orientation)
        {
            return AffineTransform( orientation.position, orientation.rotation );
        });

        // Use the transforms to generate world matrix and color buffers for the grass
        CreateInstanceBuffer(transforms, buffer_handles, vertex_container, device);
    }

    // Use the Grass parameters to generate grass on random faces of the vertex data
    // consecutively push back #count_per_type[i] orientations for grass type i to the result_orientations
    void GenerateGrassForBlock(
        VertexData const & vertex_data,
        GrassParameters const & grass_parameters,
        Math::Float2 block_size,
        uint32_t lod_level,
        GrassBufferVector & block_grass_buffers,
        GrassBoundsVector & block_grass_bounds,
        VertexBufferContainer& vertex_container,
        Device& device)
    {
        // Get number of different grass types
        auto grass_type_count = grass_parameters.densities.size();

        // Ensure that vector space is allocated for the following grass buffers
        block_grass_buffers.resize(grass_type_count);

        std::vector<Orientation> grass_orientations;

        // Iterate over existing grass types
        for (auto i = 0u; i < grass_type_count; i++)
        {
            // Get desired number of instaces for current type
            auto density_of_type = grass_parameters.densities[i][lod_level];
            auto count_of_type = uint32_t(Math::Round(Product(block_size) * density_of_type));

            // Create offset for random generator seed
            auto seed_offset = i;

            // Generate random orientations for the given grass type
            grass_orientations.clear();
            GenerateGrassPlacements(vertex_data, grass_orientations, count_of_type, seed_offset);

            // Get buffers for current type of grass
            auto & buffers = block_grass_buffers[i];

            // Only create proper buffers if any orientations were created
            if (!grass_orientations.empty())
            {
                // Create vertex buffer containing the transforms for later instanced rendering
                GenerateGrassBuffers(grass_orientations, buffers, vertex_container, device);

                // create one bounding box from multiple transformed instances of the original
                block_grass_bounds[i] = Merge( grass_parameters.bounds[i], grass_orientations );
            }
            else
            {
                // Otherwise store invalid transform and color buffers
                buffers[0].index = c_invalid_vertex_buffer_id.index;
                buffers[1].index = c_invalid_vertex_buffer_id.index;

                // Since no component will be created for these buffers
                // the bounds actually do not matter either
            }
        }
    }

    // Based on the player movement, calculate an updated block mapping
    // For each index to the mapping a new position in the block vector is stored
    // e.g. moving towards a terrain block will map the block to a closer relative slot
    void CalculateTerrainShiftMapping(
        Math::Float3 const position,
        Math::Float3 block_dimensions,
        Math::Unsigned3 block_count,
        Math::Float3 & terrain_center,
        Math::Float3 & sample_offset,
        Range<uint32_t*> mapping)
    {
        // Get distance of player from terrain center with highest lod quality
        auto offset = position - terrain_center;

        // Calculate absolute resulting shift of terrain center
        auto terrain_shift = CalculateTerrainShift(offset, block_dimensions);
        terrain_center += terrain_shift;
        sample_offset += terrain_shift;

        // Map old block properties to shifted setup
        CreateShiftedMapping(terrain_shift, block_dimensions, block_count, mapping);
    }

    // Create a parameter struct from the given input data to be used by Marching Cubes
    void CalculateMarchingCubesParameters(
        uint32_t const index,
        uint32_t const lod_level,
        Terrain3DData const & terrain,
        MarchingCubesParameters & parameters)
    {
        // Scale of terrain block and number of cubes per block
        parameters.cube_scale = terrain.block_parameters.block_cube_scales[lod_level];
        parameters.cube_counts = terrain.block_parameters.block_cube_counts[lod_level];

        // Start position at -x, -y, -z for the sample grid
        parameters.point_offset = GetBlockSampleStartPosition(index, terrain.block_count, terrain.sample_offset, terrain.block_dimensions);

        // Sample counts of the grid
        auto grid_size = parameters.cube_counts + 1;
        parameters.grid_size = grid_size;

        // Set density storage size
        parameters.densities_and_gradients.clear();
        parameters.densities_and_gradients.resize(grid_size.x * grid_size.y * grid_size.z);
    }

    // For empty vertex data, flag the blocks mesh at the current lod as empty
    // Otherwise create a MeshData on the Device and set it to be used by the block
    void UploadAndSetMesh(uint32_t const block_index,
                        Terrain3DData & terrain,
                        VertexData & vertex_data,
                        IndexBufferContainer& index_container,
                        VertexBufferContainer& vertex_container,
                        Device& device)
    {
        // Get index of using block and position of mesh in vector
        auto block_data_index = terrain.block_data_indices[block_index];
        auto& meshes = terrain.meshes[block_data_index];

        auto lod_level = terrain.lod_levels[block_index];

        // Set mesh to empty if no vertices were generated for given LOD
        if (vertex_data.positions.size() == 0)
        {
            meshes.empty[lod_level] = true;
            return;
        }

        auto mesh_data = Graphics::GenerateMesh(device,
                                            index_container,
                                            vertex_container,
                                            vertex_data.indices,
                                            vertex_data.positions,
                                            vertex_data.normals);

        // Set mesh on meshes vector
        meshes.lods[lod_level] = mesh_data;
    }

}

namespace Graphics{



    void InitializeTerrain3DData(Math::Float3 const & terrain_center,
                            float const update_distance,
                            Math::Unsigned3 const & terrain_block_count,
                            Math::Float3 const & terrain_block_dimensions,
                            Math::Unsigned3 const & terrain_block_cube_count,
                            std::vector<float> const & degradation_thresholds,
                            Get3DSampleFunctionType get_sample,
                            RenderComponentDescription const & render_component_description,
                            EntityID const & terrain_entity_id,
                            Terrain3DData& terrain)
    {

        // Set the density and gradient sample function on the terrain
        terrain.get_sample = get_sample;

        // Store render component description and entity id for terrain block meshes
        terrain.block_render_description = render_component_description;
        terrain.terrain_entity_id = terrain_entity_id;

        // Define absolute center of middle terrain block
        terrain.real_center = terrain_center;
        terrain.sample_offset = 0;

        // Define minimal distance of player entity to terrain center
        // that leads to the relative terrain being updated
        terrain.update_distance = update_distance;

        // Set number and extents of terrain blocks
        terrain.block_count = terrain_block_count;
        terrain.block_dimensions = terrain_block_dimensions;

        // Ensure that theres enough lods for all the given
        // LOD distance thresholds
        assert(c_lod_count > degradation_thresholds.size());

        // BLOCK DATA:
        auto block_count_1d = terrain.block_count.x * terrain.block_count.y * terrain.block_count.z;

        // Allocate memory for the individual terrain blocks
        terrain.meshes = std::vector<TerrainBlockMeshes>(block_count_1d);
        terrain.grass_instance_buffers = std::vector<GrassBufferVector>(block_count_1d);
        terrain.grass_buffer_bounds = std::vector<GrassBoundsVector>(block_count_1d);
        terrain.lod_levels = std::vector<uint32_t>(block_count_1d);

        //terrain.displayed_meshes = std::vector<MeshData>();

        // Initialize MeshDatas
        for (auto i = 0u; i < block_count_1d; i++)
        {
            // Block i initially using mesh i
            terrain.block_data_indices.push_back(i);
            auto& lod_entry = terrain.meshes[i];

            // Initialize mesh as invalid
            for (auto& mesh_data : lod_entry.lods)
            {
                mesh_data.mesh.index_id.index = c_invalid_vertex_buffer_id.index;
            }

            // Mark mesh as non-empty
            lod_entry.empty.fill(false);
        }

        auto grass_type_count = terrain.grass_parameters.densities.size();
        // Initialize grass instance buffers
        for (auto i = 0u; i < block_count_1d; i++)
        {
            // Reserve buffers for all grass types for current block
            auto& block_grass_buffers = terrain.grass_instance_buffers[i];
            block_grass_buffers.resize(grass_type_count);

            auto& block_grass_bounds = terrain.grass_buffer_bounds[i];
            block_grass_bounds.resize(grass_type_count);

            // Iterate over all buffer types (world matrix and color)
            for (auto& type_buffers : block_grass_buffers)
            {
                for (auto& buffer : type_buffers)
                {
                    buffer.index = c_invalid_vertex_buffer_id.index;
                }
            }
        }

        // LOD LEVELS:
        for (auto i = 0u; i < block_count_1d; i++)
        {
            // Calculate 3d index of block
            auto block_index3d = Calculate3DindexFrom1D(i, terrain.block_count.x, terrain.block_count.y);

            // Get absolute position of block
            auto offset = Math::Float3FromUnsigned3(block_index3d);
            offset *= terrain.block_dimensions;
            offset -= Math::Float3FromUnsigned3(terrain.block_count / 2) * terrain.block_dimensions;

            // Calculate LOD level for block
            terrain.lod_levels[i] = CalculateLODLevel(offset, degradation_thresholds);
        }

        // BLOCK PARAMETERS:
        terrain.block_parameters = TerrainBlockParameters();

        // Store Marching Cubes parameters for each block by lod index
        for (auto i = 0u; i <= degradation_thresholds.size(); i++)
        {
            auto lod_level = i;

            // Adjust sample resolution with lod level
            auto block_cube_count = terrain_block_cube_count / (lod_level + 1);
            auto block_cube_scale = terrain.block_dimensions / Float3FromUnsigned3(block_cube_count);

            // Ensure there is at least one cube per terrain block lod
            for(auto j = 0; j < 3; j++) assert(block_cube_count[j] > 0 && "Too many lod levels!");

            // Store parameters in lookup
            terrain.block_parameters.block_cube_counts.push_back(block_cube_count);
            terrain.block_parameters.block_cube_scales.push_back(block_cube_scale);
        }
    }

    // For each block_indices entry:
    //  -Generate a mesh at the displayed lod level
    //  -Generate grass orientations and store in new_grass_orientations
    void GenerateNewLODMeshes(Terrain3DData & terrain,
                            std::vector<uint32_t> const & block_indices,
                            IndexBufferContainer& index_container,
                            VertexBufferContainer& vertex_container,
                            Device& device)
    {
        MarchingCubesParameters parameters;
        VertexData vertex_data;
        // Loop over blocks to be loaded
        for (auto i = 0u; i < block_indices.size(); i++)
        {
            // Get index and lod level of block
            auto block_index = block_indices[i];
            auto lod_level = terrain.lod_levels[block_index];
            vertex_data.Clear();

            // Get parameters for Marching Cubes
            CalculateMarchingCubesParameters(block_index, lod_level, terrain, parameters);

            // Sample density function at cube corners
            SampleBlockGrid(parameters.grid_size, parameters.point_offset, parameters.cube_scale, terrain.get_sample, parameters.densities_and_gradients);

            // Generate vertex data for cubes
            GenerateBlock(parameters, vertex_data);

            // Scampily Generate grass placements
            // Get Grass transform buffers for current block
            auto block_data_index = terrain.block_data_indices[block_index];
            auto& block_grass_buffers = terrain.grass_instance_buffers[block_data_index];
            auto& block_grass_bounds = terrain.grass_buffer_bounds[block_data_index];

            GenerateGrassForBlock(vertex_data,
                                terrain.grass_parameters,
                                Math::Float2(terrain.block_dimensions.x, terrain.block_dimensions.y),
                                lod_level,
                                block_grass_buffers,
                                block_grass_bounds,
                                vertex_container,
                                device);

            // Create skirt
            ExtractBorderVertices(parameters.densities_and_gradients, parameters.grid_size, parameters.cube_counts, parameters.cube_scale, parameters.point_offset, vertex_data);

            // Upload mesh to GPU and set it for the current block
            UploadAndSetMesh(block_index,
                             terrain,
                             vertex_data,
                             index_container,
                             vertex_container,
                             device);


        }
    }

    void ExtractDisplayMeshes(std::vector<TerrainBlockMeshes> const & meshes,
                            std::vector<GrassBufferVector> const & grass_buffers,
                            std::vector<GrassBoundsVector> const & grass_buffer_bounds,
                            std::vector<uint32_t> const & block_data_indices,
                            std::vector<uint32_t> const & lod_levels,
                            std::vector<MeshData> & output_meshes,
                            std::vector<GrassBufferVector> & output_grass_buffers,
                            std::vector<GrassBoundsVector>& output_grass_buffer_bounds)
    {
        // Get existing mesh datas
        for (auto i = 0u; i < meshes.size(); i++)
        {
            // Get active lod level and mesh index
            auto lod_level = lod_levels[i];
            auto block_data_index = block_data_indices[i];

            // Get mesh with lods for block
            auto& lod_meshes = meshes[block_data_index].lods;

            // Store mesh at given lod level to output if not empty
            // Only non-empty meshes contain any grass
            if (lod_meshes[lod_level].mesh.index_id.index != c_invalid_vertex_buffer_id.index)
            {
                // Store current lod level of terrain block as displayed mesh
                output_meshes.push_back(lod_meshes[lod_level]);

                // Only extract grass for lod 0
                // if (lod_level == 0)
                {
                    // Get grass instance buffers for block
                    auto& block_grass_buffers = grass_buffers[block_data_index];
                    auto& blocK_grass_buffer_bounds = grass_buffer_bounds[block_data_index];

                    // Check if buffers are valid (containing anything)
                    if ( std::any_of(begin(block_grass_buffers), end(block_grass_buffers), [](std::array<VertexBufferID, 2> const & buffer)
                    {
                        return buffer[0].index != c_invalid_vertex_buffer_id.index;
                    }))
                    {
                        output_grass_buffers.push_back(block_grass_buffers);
                        output_grass_buffer_bounds.push_back(blocK_grass_buffer_bounds);
                    }
                }
            }
        }
    }

    // Use the input position to update the terrain relative to the player.
    // If the position is further than terrain.update_distance from terrain.center,
    // its scope gets updated to be centered around the closest terrain block to the position
    //
    // Parameters:
    // position                     - player position (or whatever the terrain is relative to)
    // terrain                      - terrain container
    // display_meshes               - output of terrain block meshes visible in the scope of current terrain
    // display_grass_buffers        - output of instance buffers for grass, in snyc to meshes
    // display_grass_buffer_bounds  - output of AABBs for above grass buffers
    // unload_meshes                - output of meshes of pruned blocks
    // unload_instance_buffers      - output of grass instance buffers for pruned blocks
    // index_container              - input for resource creation
    // vertex_container             - - " -
    // device                       - - " -
    void UpdateTerrainBlocks(Math::Float3 const & position,
                            Terrain3DData& terrain,
                            std::vector<MeshData>& display_meshes,
                            std::vector<GrassBufferVector>& display_grass_buffers,
                            std::vector<GrassBoundsVector>& display_grass_buffer_bounds,
                            std::vector<MeshData>& unload_meshes,
                            std::vector<VertexBufferID>& unload_instance_buffers,
                            IndexBufferContainer& index_container,
                            VertexBufferContainer& vertex_container,
                            Device& device)
    {
        // Create a mapping to assign new relative positions to all terrain blocks
        std::vector<uint32_t> mapping(terrain.block_data_indices.size());
        CalculateTerrainShiftMapping(
            position,
            terrain.block_dimensions,
            terrain.block_count,
            terrain.real_center,
            terrain.sample_offset,
            mapping);

        // Use mapping to shift terrain block meshes to other relative blocks
        // Store indices of blocks with meshes that need yet to be generated (load_blocks)
        std::vector<uint32_t> load_blocks;
        ApplyBlockMapping(mapping, terrain, unload_meshes, unload_instance_buffers, load_blocks);

        // Generate new meshes for each load_blocks entry at their visible lod level
        GenerateNewLODMeshes(terrain,
                            load_blocks,
                            index_container,
                            vertex_container,
                            device);

        // Store non-empty meshes at correct LOD level to displayed_meshes
        ExtractDisplayMeshes(terrain.meshes, terrain.grass_instance_buffers, terrain.grass_buffer_bounds, terrain.block_data_indices, terrain.lod_levels, display_meshes, display_grass_buffers, display_grass_buffer_bounds);
    }


	void SampleBlockGrid(
        Math::Unsigned3 grid_size,
        Math::Float3 point_offset,
        Math::Float3 cube_scale,
        Get3DSampleFunctionType const & get_sample,
        Range<DensityAndGradient*> densities_and_gradients)
	{
        auto total_size = grid_size.x * grid_size.y * grid_size.z;
        for( auto i = 0u; i < total_size; i++ )
        {
		    // Relative position in block
            auto index3d = Calculate3DindexFrom1D(i, grid_size.x, grid_size.y);
		    auto grid_position = Math::Float3FromUnsigned3(index3d);
            auto position = point_offset + (grid_position * cube_scale);

            get_sample(position, densities_and_gradients[i].density, densities_and_gradients[i].gradient);
        }
	}
}
