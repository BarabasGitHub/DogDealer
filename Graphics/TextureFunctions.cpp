#include "TextureFunctions.h"
#include "TextureContainer.h"
#include "Texture.h"
#include "ResourceViewContainer.h"

using namespace Graphics;

void Graphics::UnloadTexture(TextureTypeAndID const & id, TextureContainer & texture_container, ResourceViewContainer & resource_view_container)
{
    texture_container.RemoveTexture2D(id.data_id);
    resource_view_container.RemoveShaderResourceView(id.view_id);
}


void Graphics::UnloadTextures(Range<TextureTypeAndID const*> ids, TextureContainer & texture_container, ResourceViewContainer & resource_view_container)
{
    for( auto & id : ids)
    {
        UnloadTexture(id, texture_container, resource_view_container);
    }
}
