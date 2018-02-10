#pragma once

#include "SSETypes.h"

namespace Math
{
    namespace SSE
    {
        // add/subtract
        IntegerVector Add8Bit(IntegerVector a, IntegerVector b);
        IntegerVector Add16Bit(IntegerVector a, IntegerVector b);
        IntegerVector Add32Bit( IntegerVector a, IntegerVector b );
        IntegerVector Subtract8Bit(IntegerVector a, IntegerVector b);
        IntegerVector Subtract16Bit(IntegerVector a, IntegerVector b);
        IntegerVector Subtract32Bit( IntegerVector a, IntegerVector b );

        // multiply/divide

        IntegerVector Multiply32Bit( IntegerVector a, IntegerVector b );

        // bitwise
        IntegerVector ShiftRight16BitUnsigned(IntegerVector val, const int shift);
        IntegerVector ShiftRight16BitSigned(IntegerVector val, const int shift);
        IntegerVector ShiftRight32BitSigned( IntegerVector val, const int shift );
        IntegerVector ShiftRight32BitUnsigned(IntegerVector val, const int shift);
        IntegerVector ShiftLeft32Bit( IntegerVector val, const int shift );
        IntegerVector ShiftRight64BitUnsigned( IntegerVector val, const int shift );
        IntegerVector ShiftLeft64Bit(IntegerVector val, const int shift);
        template<int shift_bytes> IntegerVector ShiftRight128BitUnsigned(IntegerVector val);
        template<int shift_bytes> IntegerVector ShiftLeft128Bit(IntegerVector val);
        IntegerVector ExclusiveOr( IntegerVector a, IntegerVector b );
        IntegerVector Or( IntegerVector a, IntegerVector b );
        IntegerVector And( IntegerVector a, IntegerVector b );
        // a & !b
        IntegerVector AndNot( IntegerVector a, IntegerVector b );

        IntegerVector Abs( IntegerVector a );
        // comparison

        // returns a mask (either all bits set or not per component)
        IntegerVector Equal32Bit( IntegerVector a, IntegerVector b );



        // 1 if all the bits set in b are set in a; otherwise 0.
        int TestBits( IntegerVector a, IntegerVector b );

        // tests only the bits in mask
        int TestAllZero( IntegerVector value, IntegerVector mask );

        // tests all bits
        int TestAllZero( IntegerVector value );

        int TestAllBitsSet( IntegerVector value );

        // value move

        template<unsigned Index0, unsigned Index1 = 1, unsigned Index2 = 2, unsigned Index3 = 3> IntegerVector Swizzle32Bit( IntegerVector val );
        // Selects and interleaves the lower values from a and b.
        // r.x = a.x
        // r.y = b.x
        // r.z = a.y
        // r.w = b.y
        IntegerVector InterleaveLow( IntegerVector a, IntegerVector b );



        // Selects and interleaves the higher two single-precision, floating-point values from a and b.
        // r.x = a.z
        // r.y = b.z
        // r.z = a.w
        // r.w = b.w
        IntegerVector InterleaveHigh( IntegerVector a, IntegerVector b );


        // Selects and interleaves the lower 64 bit integers from a and b.
        // r.x = a.x
        // r.y = b.x
        IntegerVector InterleaveLow64( IntegerVector a, IntegerVector b );


        // Selects and interleaves the higher 64 bit integers from a and b
        // r.x = a.y
        // r.y = b.y
        IntegerVector InterleaveHigh64( IntegerVector a, IntegerVector b );
    }
}


// implementations
namespace Math
{
    namespace SSE
    {
        inline IntegerVector ShiftRight16BitUnsigned(IntegerVector val, const int shift)
        {
            return _mm_srli_epi16(val, shift);
        }


        inline IntegerVector ShiftRight16BitSigned(IntegerVector val, const int shift)
        {
            return _mm_srai_epi16(val, shift);
        }


        inline IntegerVector ShiftRight32BitSigned( IntegerVector val, const int shift )
        {
            return _mm_srai_epi32(val, shift);
        }


        inline IntegerVector ShiftRight32BitUnsigned(IntegerVector val, const int shift)
        {
            return _mm_srli_epi32( val, shift );
        }


        template<int shift_bytes>
        inline IntegerVector ShiftRight128BitUnsigned(IntegerVector val)
        {
            return _mm_srli_si128(val, shift_bytes);
        }


        inline IntegerVector ShiftLeft32Bit( IntegerVector val, const int shift )
        {
            return _mm_slli_epi32( val, shift );
        }


        inline IntegerVector ShiftRight64BitUnsigned( IntegerVector val, const int shift )
        {
            return _mm_srli_epi64( val, shift );
        }


        inline IntegerVector ShiftLeft64Bit( IntegerVector val, const int shift )
        {
            return _mm_slli_epi64( val, shift );
        }


        template<int shift_bytes>
        inline IntegerVector ShiftLeft128Bit(IntegerVector val)
        {
            return _mm_slli_si128(val, shift_bytes);
        }


        inline IntegerVector ExclusiveOr( IntegerVector a, IntegerVector b )
        {
            return _mm_xor_si128( a, b );
        }


        inline IntegerVector Or( IntegerVector a, IntegerVector b )
        {
            return _mm_or_si128( a, b );
        }


        inline IntegerVector And( IntegerVector a, IntegerVector b )
        {
            return _mm_and_si128( a, b );
        }


        inline IntegerVector AndNot( IntegerVector a, IntegerVector b )
        {
            return _mm_andnot_si128( a, b );
        }


        inline IntegerVector Abs( IntegerVector a )
        {
            return _mm_abs_epi32( a );
        }


        inline IntegerVector Equal32Bit( IntegerVector a, IntegerVector b )
        {
            return _mm_cmpeq_epi32( a, b );
        }


        template<unsigned Index0, unsigned Index1, unsigned Index2, unsigned Index3>
        inline IntegerVector Swizzle32Bit( IntegerVector val )
        {
            static_assert( Index0 < 4, "Index0 exceeds dimensions." );
            static_assert( Index1 < 4, "Index1 exceeds dimensions." );
            static_assert( Index2 < 4, "Index2 exceeds dimensions." );
            static_assert( Index3 < 4, "Index3 exceeds dimensions." );
            return _mm_shuffle_epi32( val, _MM_SHUFFLE( Index3, Index2, Index1, Index0 ) );
        }



        template<>
        inline IntegerVector Swizzle32Bit<0, 0, 1, 1>( IntegerVector val )
        {
            return InterleaveLow( val, val );
        }


        template<>
        inline IntegerVector Swizzle32Bit<2, 2, 3, 3>( IntegerVector val )
        {
            return InterleaveHigh( val, val );
        }


        template<>
        inline IntegerVector Swizzle32Bit<0, 1, 0, 1>( IntegerVector val )
        {
            return InterleaveLow64( val, val );
        }


        template<>
        inline IntegerVector Swizzle32Bit<2, 3, 2, 3>( IntegerVector val )
        {
            return InterleaveHigh64( val, val );
        }


        inline IntegerVector Multiply32Bit( IntegerVector a, IntegerVector b )
        {
            return _mm_mullo_epi32( a, b );
        }


        inline IntegerVector Add16Bit(IntegerVector a, IntegerVector b)
        {
            return _mm_add_epi16(a, b);
        }


        inline IntegerVector Add8Bit(IntegerVector a, IntegerVector b)
        {
            return _mm_add_epi8(a, b);
        }


        inline IntegerVector Add32Bit( IntegerVector a, IntegerVector b )
        {
            return _mm_add_epi32( a, b );
        }


        inline IntegerVector Subtract8Bit(IntegerVector a, IntegerVector b)
        {
            return _mm_sub_epi8(a, b);
        }


        inline IntegerVector Subtract16Bit(IntegerVector a, IntegerVector b)
        {
            return _mm_sub_epi16(a, b);
        }


        inline IntegerVector Subtract32Bit( IntegerVector a, IntegerVector b )
        {
            return _mm_sub_epi32( a, b );
        }


        // 1 if all the bits set in b are set in a; otherwise 0.
        inline int TestBits( IntegerVector a, IntegerVector b )
        {
            return _mm_testc_si128( a, b );
        }


        inline int TestAllZero( IntegerVector value, IntegerVector mask )
        {
            return _mm_testz_si128( mask, value );
        }


        inline int TestAllZero( IntegerVector value )
        {
            return TestAllZero( value, Equal32Bit( value, value ) );
        }


        inline int TestAllBitsSet( IntegerVector value )
        {
            return TestBits( value, Equal32Bit( value, value ) );
        }

        // Selects and interleaves the lower values from a and b.
        // r.x = a.x
        // r.y = b.x
        // r.z = a.y
        // r.w = b.y
        inline IntegerVector InterleaveLow( IntegerVector a, IntegerVector b )
        {
            return _mm_unpacklo_epi32( a, b );
        }


        // Selects and interleaves the higher 32 bit values from a and b.
        // r.x = a.z
        // r.y = b.z
        // r.z = a.w
        // r.w = b.w
        inline IntegerVector InterleaveHigh( IntegerVector a, IntegerVector b )
        {
            return _mm_unpackhi_epi32( a, b );
        }


        // Selects and interleaves the lower 64 bit integers from a and b.
        // r.x = a.x
        // r.y = b.x
        inline IntegerVector InterleaveLow64( IntegerVector a, IntegerVector b )
        {
            return _mm_unpacklo_epi64( a, b );
        }


        // Selects and interleaves the higher 64 bit integers from a and b
        // r.x = a.y
        // r.y = b.y
        inline IntegerVector InterleaveHigh64( IntegerVector a, IntegerVector b )
        {
            return _mm_unpackhi_epi64( a, b );
        }
    }
}
