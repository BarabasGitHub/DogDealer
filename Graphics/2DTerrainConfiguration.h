#pragma once

#include "IDs.h"
#include "RenderComponent.h"
#include "DisplayTechnique.h"

#include <Math\FloatTypes.h>
#include <Math\IntegerTypes.h>

#include <Conventions\EntityID.h>

#include <functional>
#include <vector>

namespace Graphics
{
    typedef std::function<void(Math::Float2 const position, float & density, Math::Float2 & gradient)> Get2DSampleFunctionType;

    struct Terrain2DData
    {
        struct PatchData;
        struct GraphicsData
        {
            ConstantBufferID persistent_terrain_data_buffer;
            IndexBufferID universal_patch_index_buffer;
            std::vector<TextureTypeAndID> global_textures;
            DisplayTechnique display_technique, shadow_technique;
            bool alpha_to_coverage;
        };

        struct GrassParameters
        {
            struct GraphicsData
            {
                Mesh mesh;
                ConstantBufferID object_buffer;
                // for the orientation/affine transformation for each batch
                std::vector<VertexBufferID> instance_buffers;
                std::vector<TextureTypeAndID> textures;
                DisplayTechnique display_technique, shadow_technique;
                bool alpha_to_coverage;
            };
            GraphicsData graphics;
            BoundingShapes::AxisAlignedBox original_bounds;
            std::vector<BoundingShapes::AxisAlignedBox> batch_bounds;
            std::vector<uint32_t> lod_levels;
        };


        EntityID entity_id = c_invalid_entity_id;
        Get2DSampleFunctionType sample_function;
        Math::Unsigned2 patch_dimensions = 0;
        Math::Float2 patch_size = 0;
        Math::Float3 center = 0;
        Math::Unsigned2 total_size = 0;
        std::vector<float> lod_distances;
        // LOD levels for each highest resolution patch
        // So if we have one at a lower level it will fill it for all highest resolution patches that it covers
        // in 2D Morton layout.
        std::vector<uint32_t> lod_levels;
        // The data for each patch, organized the same way as the lod_levels.
        std::vector<PatchData> patch_data;
        GraphicsData graphics;
        std::vector<GrassParameters> grass;
        std::vector<uint32_t> grass_lod_data_offsets;
        std::vector<float> grass_lod_densities;
        std::vector<float> grass_lod_distances;

        struct PatchData
        {
            TextureTypeAndID texture_id;
            ConstantBufferID constant_buffer_id;
            BoundingShapes::AxisAlignedBox bounding_box;
        };
    };
}
