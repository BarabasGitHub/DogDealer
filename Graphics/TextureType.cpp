#include "TextureType.h"
#include "ShaderKeyWords.h"
#include <Utilities\DogDealerException.h>
#include <string>

namespace Graphics
{
    TextureType ConvertToTextureType( std::string name )
    {
        using namespace KeyWord;
        if( name == diffusemap ) return TextureType::Diffuse;
        else if( name == normalmap ) return TextureType::Normal;
        else if( name == specularmap ) return TextureType::Specular;
        else if( name == specmap ) return TextureType::Specular;
        else if( name == shadowmap ) return TextureType::Shadow;
        else if( name == heightmap ) return TextureType::Height;
        else if( name == detailmap ) return TextureType::Detail;
        else throw DogDealerException(("Unknown Texture object name: " + name).c_str(), true);
    }
}