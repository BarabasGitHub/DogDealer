#pragma once
#include "IntegerTypes.h"

namespace Math
{
    // move the elements around, output will be (input[Index0], input[Index1], input[Index2])
    template<unsigned Index0, unsigned Index1>
    inline Int2 Swizzle( Int2 const & input )
    {
        static_assert( Index0 < 2, "Index0 exceeds dimensions." );
        static_assert( Index1 < 2, "Index1 exceeds dimensions." );
        return{ input[Index0], input[Index1] };
    }

    // move the elements around, output will be (input[Index0], input[Index1], input[Index2])
    template<unsigned Index0, unsigned Index1, unsigned Index2>
    inline Int3 Swizzle( Int3 const & input )
    {
        static_assert( Index0 < 3, "Index0 exceeds dimensions." );
        static_assert( Index1 < 3, "Index1 exceeds dimensions." );
        static_assert( Index2 < 3, "Index2 exceeds dimensions." );
        return{ input[Index0], input[Index1], input[Index2] };
    }

    // move the elements around, output will be (input[Index0], input[Index1], input[Index2])
    template<unsigned Index0, unsigned Index1, unsigned Index2, unsigned Index3>
    inline Int4 Swizzle( Int4 const & input )
    {
        static_assert( Index0 < 4, "Index0 exceeds dimensions." );
        static_assert( Index1 < 4, "Index1 exceeds dimensions." );
        static_assert( Index2 < 4, "Index2 exceeds dimensions." );
        static_assert( Index3 < 4, "Index3 exceeds dimensions." );
        return{ input[Index0], input[Index1], input[Index2], input[Index3] };
    }
 

    // move the elements around, output will be (input[Index0], input[Index1], input[Index2])
    template<unsigned Index0, unsigned Index1>
    inline Unsigned2 Swizzle( Unsigned2 const & input )
    {
        static_assert( Index0 < 2, "Index0 exceeds dimensions." );
        static_assert( Index1 < 2, "Index1 exceeds dimensions." );
        return{ input[Index0], input[Index1] };
    }


    // move the elements around, output will be (input[Index0], input[Index1], input[Index2])
    template<unsigned Index0, unsigned Index1, unsigned Index2>
    inline Unsigned3 Swizzle( Unsigned3 const & input )
    {
        static_assert( Index0 < 3, "Index0 exceeds dimensions." );
        static_assert( Index1 < 3, "Index1 exceeds dimensions." );
        static_assert( Index2 < 3, "Index2 exceeds dimensions." );
        return{ input[Index0], input[Index1], input[Index2] };
    }


    // move the elements around, output will be (input[Index0], input[Index1], input[Index2])
    template<unsigned Index0, unsigned Index1, unsigned Index2, unsigned Index3>
    inline Unsigned4 Swizzle( Unsigned4 const & input )
    {
        static_assert( Index0 < 4, "Index0 exceeds dimensions." );
        static_assert( Index1 < 4, "Index1 exceeds dimensions." );
        static_assert( Index2 < 4, "Index2 exceeds dimensions." );
        static_assert( Index3 < 4, "Index3 exceeds dimensions." );
        return{ input[Index0], input[Index1], input[Index2], input[Index3] };
    }
}