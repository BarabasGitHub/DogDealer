#pragma once

#include <Utilities\Range.h>

namespace Graphics
{
    struct TextureTypeAndID;
    struct TextureContainer;
    struct ResourceViewContainer;

    void UnloadTexture(TextureTypeAndID const & id, TextureContainer & texture_container, ResourceViewContainer & resource_view_container);
    void UnloadTextures(Range<TextureTypeAndID const*> ids, TextureContainer & texture_container, ResourceViewContainer & resource_view_container);
}
