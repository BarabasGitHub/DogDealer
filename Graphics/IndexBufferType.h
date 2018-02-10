#pragma once

#include <dxgiformat.h>
#include <cassert>
#include <limits>
#include <cstdint>

namespace Graphics
{
    enum struct IndexBufferType : uint8_t
    {
        Short = DXGI_FORMAT_R16_UINT,
        Normal = DXGI_FORMAT_R32_UINT,
    };

    template<typename IndexType>
    inline IndexBufferType constexpr IndexBufferTypeFromIntegerType(IndexType = 0)
    {
        static_assert(false, "Not a valid integer type to use for an index buffer.")
    }

    template<>
    inline IndexBufferType constexpr IndexBufferTypeFromIntegerType<uint16_t>(uint16_t)
    {
        return IndexBufferType::Short;
    }


    template<>
    inline IndexBufferType constexpr IndexBufferTypeFromIntegerType<uint32_t>(uint32_t)
    {
        return IndexBufferType::Normal;
    }

    inline unsigned GetSize(IndexBufferType in)
    {
        switch (in)
        {
        case Graphics::IndexBufferType::Short:
            return sizeof(unsigned short);
        case Graphics::IndexBufferType::Normal:
            return sizeof(unsigned);
        default:
            assert(false);
            return std::numeric_limits<unsigned>::max();
        }
    }

    inline DXGI_FORMAT constexpr ToDXGIFormat(IndexBufferType in)
    {
        return static_cast<DXGI_FORMAT>(in);
    }
}
