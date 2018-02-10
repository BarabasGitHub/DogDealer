#pragma once
#include "TextureType.h"
#include "IDs.h"

#include <tuple> // for std::tie

namespace Graphics
{
    struct TextureTypeAndID
    {
        TextureType type;
        Texture2DID data_id;
        ShaderRersourceViewID view_id;
        TextureTypeAndID() = default;
        TextureTypeAndID( TextureType type, Texture2DID data_id, ShaderRersourceViewID view_id ) : type( type ), data_id(data_id), view_id( view_id ) {}
    };

    inline bool operator<( TextureTypeAndID const & first, TextureTypeAndID const & second )
    {
        return std::tie( first.data_id, first.view_id, first.type ) < std::tie( second.data_id, second.view_id, second.type );
    }

    inline bool operator==( TextureTypeAndID const & first, TextureTypeAndID const & second )
    {
        return std::tie( first.data_id, first.view_id, first.type ) == std::tie( second.data_id, second.view_id, second.type );
    }
}