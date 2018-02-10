#pragma once

#include <string>

namespace Graphics
{
    enum struct TextureType
    {
        Diffuse,
        Normal,
        Specular,
        Height,
        Shadow,
        Detail
    };

    TextureType ConvertToTextureType( std::string name );
}