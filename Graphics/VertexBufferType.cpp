#include "VertexBufferType.h"
#include "HLSLTypes.h"
#include <FileLayout\VertexDataType.h>
#include <limits>

namespace Graphics
{

    unsigned GetStride(VertexBufferType in)
    {
        switch (in)
        {
        case VertexBufferType::Position:
            return sizeof(PositionType);
        case VertexBufferType::Normal:
            return sizeof(NormalType);
        case VertexBufferType::Tangent:
            return sizeof(TangentDirections);
        case VertexBufferType::Texture:
            return sizeof(UVType);
        case VertexBufferType::Color:
            return sizeof(ColorType);
        case VertexBufferType::Bone:
            return sizeof(BoneWeightsAndIndices);
        case VertexBufferType::WorldMatrix:
            return sizeof(HLSL::float3x4);
        case VertexBufferType::None:
        case VertexBufferType::SystemGenerated:
            return 0;
        default:
            assert(false);
            return std::numeric_limits<unsigned>::max();
        }
    }



}
