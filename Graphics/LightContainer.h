#pragma once
#include "IDs.h"
#include "Structures.h"

#include <Math\FloatTypes.h>

#include <Conventions\EntityID.h>

#include <vector>

namespace Graphics
{
    struct LightContainer
    {
        // just for administrative purposes?
        std::vector<Texture2DID> textures;

        // needed for rendering the depth texture
        std::vector<D3D11_VIEWPORT> view_ports;

        // depth stencil views for rendering the shadow map
        std::vector<DepthStencilViewID> depth_stencil_views;

        // for shadow lookup in shaders
        std::vector<ShaderRersourceViewID> shader_resource_views;

        // constant buffers with light data
        std::vector<ConstantBufferID> constant_buffers;
        // constant buffers with light transforms, has shadow_map_count buffers per light
        std::vector<ConstantBufferID> transform_constant_buffers;

        // colors
        std::vector<Math::Float3> colors;

        // either an entity id or a render component id I think
        std::vector<EntityID> entity_ids;

        // 'global' shadow states
        ComPtr<ID3D11DepthStencilState> depth_stencil_state;
        ComPtr<ID3D11RasterizerState> rasterizer_state;
        uint32_t shadow_map_count; // per light
    };
}