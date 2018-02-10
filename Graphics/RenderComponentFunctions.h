#pragma once

#include <Utilities\Range.h>

namespace Graphics
{
    struct RenderComponent;
    struct VertexBufferContainer;
    struct IndexBufferContainer;
    struct TextureContainer;
    struct ResourceViewContainer;
    struct ConstantBufferContainer;

    void ReleaseAllResources(
        RenderComponent const & render_component,
        VertexBufferContainer & vertex_buffer_container,
        IndexBufferContainer & index_buffer_container,
        TextureContainer & texture_container,
        ResourceViewContainer & resource_view_container,
        ConstantBufferContainer & constant_buffer_container
        );


    void ReleaseAllResources(
        Range<RenderComponent const *> render_components,
        VertexBufferContainer & vertex_buffer_container,
        IndexBufferContainer & index_buffer_container,
        TextureContainer & texture_container,
        ResourceViewContainer & resource_view_container,
        ConstantBufferContainer & constant_buffer_container
        );
}
