#pragma once
#include "ConstantBufferTypes.h"
#include "IDs.h"

#include <tuple> // for std::tie

namespace Graphics
{
    struct ConstantBufferTypeAndID
    {
        ConstantBufferType type;
        ConstantBufferID id;
        ConstantBufferTypeAndID() = default;
        ConstantBufferTypeAndID( ConstantBufferType type, ConstantBufferID id) : type( type ), id(id) {}
    };

    inline bool operator<( ConstantBufferTypeAndID const & first, ConstantBufferTypeAndID const & second )
    {
        return std::tie( first.id, first.type ) < std::tie( second.id, second.type );
    }

    inline bool operator==( ConstantBufferTypeAndID const & first, ConstantBufferTypeAndID const & second )
    {
        return std::tie( first.id, first.type ) == std::tie( second.id, second.type );
    }
}