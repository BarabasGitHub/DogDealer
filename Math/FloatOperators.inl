#pragma once
#include "FloatTypes.h"

namespace Math
{
     // move the elements around, output will be (input[Index0], input[Index1], input[Index2])
    template<unsigned Index0, unsigned Index1>
    inline Float2 Swizzle( Float2 const & input )
    {
        static_assert( Index0 < 2, "Index0 exceeds dimensions." );
        static_assert( Index1 < 2, "Index1 exceeds dimensions." );
        return{ input[Index0], input[Index1] };
    }

    // move the elements around, output will be (input[Index0], input[Index1], input[Index2])
    template<unsigned Index0, unsigned Index1, unsigned Index2>
    inline Float3 Swizzle( Float3 const & input )
    {
        static_assert( Index0 < 3, "Index0 exceeds dimensions." );
        static_assert( Index1 < 3, "Index1 exceeds dimensions." );
        static_assert( Index2 < 3, "Index2 exceeds dimensions." );
        return{ input[Index0], input[Index1], input[Index2] };
    }

    // move the elements around, output will be (input[Index0], input[Index1], input[Index2])
    template<unsigned Index0, unsigned Index1, unsigned Index2, unsigned Index3>
    inline Float4 Swizzle( Float4 const & input )
    {
        static_assert( Index0 < 4, "Index0 exceeds dimensions." );
        static_assert( Index1 < 4, "Index1 exceeds dimensions." );
        static_assert( Index2 < 4, "Index2 exceeds dimensions." );
        static_assert( Index3 < 4, "Index3 exceeds dimensions." );
        return{ input[Index0], input[Index1], input[Index2], input[Index3] };
    }
}