#pragma once

#include "DirectX/Direct3D11.h"
#include "IDs.h"

#include <vector>
#include <deque>
#include <cassert>
#include <cstdint>

#include <Utilities\Handle.h>

namespace Graphics
{
    struct Device;


    struct ResourceViewContainer
    {
    public:

        std::vector<ComPtr<ID3D11RenderTargetView>> m_render_target_views;
        std::vector<uint8_t> m_render_target_view_generations;
        std::deque<unsigned> m_free_render_target_views;

        std::vector<ComPtr<ID3D11DepthStencilView>> m_depth_stencil_views;
        std::vector<uint8_t> m_depth_stencil_view_generations;
        std::deque<unsigned> m_free_depth_stencil_views;

        std::vector<ComPtr<ID3D11ShaderResourceView>> m_shader_resource_views;
        std::vector<uint8_t> m_shader_resource_view_generations;
        std::deque<unsigned> m_free_shader_resouce_views;

        bool IsValid( ShaderRersourceViewID id ) const
        {
            assert(m_shader_resource_views.size() == m_shader_resource_view_generations.size() );
            return id.index < m_shader_resource_views.size() && id.generation == m_shader_resource_view_generations[id.index];
        }

        bool IsValid( RenderTargetViewID id ) const
        {
            assert(m_render_target_views.size() == m_render_target_view_generations.size() );
            return id.index < m_render_target_views.size() && id.generation == m_render_target_view_generations[id.index];
        }

        bool IsValid( DepthStencilViewID id ) const
        {
            assert(m_depth_stencil_views.size() == m_depth_stencil_view_generations.size() );
            return id.index < m_depth_stencil_views.size() && id.generation == m_depth_stencil_view_generations[id.index];
        }

        ID3D11RenderTargetView* GetRenderTargetView( RenderTargetViewID id )
        {
            assert(IsValid(id));
            return m_render_target_views[id.index];
        }

        ID3D11DepthStencilView* GetDepthStencilView( DepthStencilViewID id )
        {
            assert(IsValid(id));
            return m_depth_stencil_views[id.index];
        }

        ID3D11ShaderResourceView* GetShaderResourceView( ShaderRersourceViewID id )
        {
            assert(IsValid(id));
            return m_shader_resource_views[id.index];
        }

        RenderTargetViewID AddRenderTargetView( ComPtr<ID3D11RenderTargetView> view );
        DepthStencilViewID AddDepthStencilView( ComPtr<ID3D11DepthStencilView> view );
        ShaderRersourceViewID AddShaderResourceView( ComPtr<ID3D11ShaderResourceView> view );

        void RemoveRenderTargetView( RenderTargetViewID id )
        {
            if( IsValid(id) )
            {
                auto users_left = m_render_target_views[id.index].Reset();
                (void) users_left;
                assert( users_left == 0 );
                m_free_render_target_views.push_back(id.index);
                ++m_render_target_view_generations[id.index];
            }
        }

        void RemoveDepthStencilView( DepthStencilViewID id )
        {
            if( IsValid(id) )
            {
                auto users_left = m_depth_stencil_views[id.index].Reset();
                (void) users_left;
                assert(users_left == 0);

                m_free_depth_stencil_views.push_back(id.index);
                ++m_depth_stencil_view_generations[id.index];
            }
        }

        void RemoveShaderResourceView( ShaderRersourceViewID id )
        {
            if( IsValid(id) )
            {
                auto users_left = m_shader_resource_views[id.index].Reset();
                (void) users_left;
                assert(users_left == 0);

                m_free_shader_resouce_views.push_back(id.index);
                ++m_shader_resource_view_generations[id.index];
            }
        }
    };
}
