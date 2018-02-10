#pragma once
#include "HLSLTypes.h"

#pragma warning( push )
// disable padding due to __declspec(align()) warnings.
#pragma warning( disable : 4324 )

namespace Graphics
{
    typedef uint32_t uint;
    #include "Shaders\LightConstants.hlsli"

    namespace ConstantBuffers
    {
        struct Object
        {
            HLSL::float3x4 transform_matrix;
            HLSL::float4 base_color;
        };

        struct Projection
        {
            HLSL::float4x4 matrix;
        };

        struct Camera
        {
            HLSL::float4x4 inverse_camera_projection_matrix;;
            HLSL::float3x3 matrix;
            HLSL::float3 camera_position;
        };

        struct Bones
        {
            HLSL::float3x4 matrices[64];
        };


        struct Sky
        {
            HLSL::float3 color;
        };


        struct Light
        {
            static auto const c_max_number_of_transforms = c_max_number_of_light_transforms;
            // normal in world space
            HLSL::float3 normal;
            HLSL::float3 color;
            // world -> light -> projection
            HLSL::float4x4 transform[c_max_number_of_transforms];
            HLSL::arrayfloat far_planes[c_max_number_of_transforms];
        };

        struct LightSingleMap // old
        {
            // normal in world space
            HLSL::float3 normal;
            // world -> light -> projection
            HLSL::float4x4 transform;
        };

        struct SingleLightTransform
        {
            // world -> light -> projection
            HLSL::float4x4 transform;
        };

        struct Terrain2D
        {
            Math::Unsigned2 patch_dimensions;
            Math::Float2 patch_size;
        };

        struct Terrain2DPatch
        {
            Math::Float2 start;
            Math::Unsigned2 texture_offset;
            Math::Unsigned2 edges_to_double;
            uint32_t lod_level;
        };
    }
}

#pragma warning(pop)