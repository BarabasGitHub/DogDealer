#pragma once
#include <cstdint>
namespace Graphics
{
    enum struct VertexBufferType : uint8_t
    {
        None,
        Position,
        Normal,
        Tangent,
        Texture,
        Color,
        Bone,
        WorldMatrix,
        SystemGenerated,
        Undefined,
    };

    unsigned GetStride(VertexBufferType in);

}