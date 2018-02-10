#include "RenderComponentFunctions.h"

#include "RenderComponent.h"
#include "ConstantBufferTypeAndIDFunctions.h"
#include "MeshFunctions.h"
#include "TextureFunctions.h"

using namespace Graphics;

void Graphics::ReleaseAllResources(
        RenderComponent const & render_component,
        VertexBufferContainer & /*vertex_buffer_container*/,
        IndexBufferContainer & /*index_buffer_container*/,
        TextureContainer & /*texture_container*/,
        ResourceViewContainer & /*resource_view_container*/,
        ConstantBufferContainer & constant_buffer_container
        )
{
    //UnloadMesh(render_component.mesh, index_buffer_container, vertex_buffer_container);
    //UnloadTextures(render_component.textures, TextureContainer & texture_container, ResourceViewContainer & resource_view_container);
    ReleaseBuffers(render_component.buffers, constant_buffer_container);
}


void Graphics::ReleaseAllResources(Range<RenderComponent const*> render_components, VertexBufferContainer & vertex_buffer_container, IndexBufferContainer & index_buffer_container, TextureContainer & texture_container, ResourceViewContainer & resource_view_container, ConstantBufferContainer & constant_buffer_container)
{
    for(auto & rc : render_components)
    {
        ReleaseAllResources(rc, vertex_buffer_container, index_buffer_container, texture_container, resource_view_container, constant_buffer_container);
    }
}
