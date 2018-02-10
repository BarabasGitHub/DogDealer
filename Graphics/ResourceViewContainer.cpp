#include "ResourceViewContainer.h" 

#include "Device.h"

#include <Utilities\ContainerHelpers.h>

using namespace Graphics;


RenderTargetViewID ResourceViewContainer::AddRenderTargetView( ComPtr<ID3D11RenderTargetView> view )
{    
    RenderTargetViewID id;
    Add( id, std::move( view ), m_render_target_views, m_render_target_view_generations, m_free_render_target_views );
    return id;
}

DepthStencilViewID ResourceViewContainer::AddDepthStencilView( ComPtr<ID3D11DepthStencilView> view )
{
    DepthStencilViewID id;
    Add( id, std::move( view ), m_depth_stencil_views, m_depth_stencil_view_generations, m_free_depth_stencil_views );
    return id;
}


ShaderRersourceViewID ResourceViewContainer::AddShaderResourceView( ComPtr<ID3D11ShaderResourceView> view )
{
    ShaderRersourceViewID id;
    Add( id, std::move( view ), m_shader_resource_views, m_shader_resource_view_generations, m_free_shader_resouce_views );
    return id;
}