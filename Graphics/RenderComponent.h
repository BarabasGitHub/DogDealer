#pragma once

#include "Mesh.h"
#include "Texture.h"
#include "ConstantBufferTypeAndID.h"

#include <vector>

namespace Graphics
{
    struct RenderComponent
    {
        Mesh mesh;
        std::vector<TextureTypeAndID> textures;
        std::vector<ConstantBufferTypeAndID> buffers;
    };

}