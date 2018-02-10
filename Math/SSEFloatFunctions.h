#pragma once

#include "SSETypes.h"
#include "SSESetFunctions.h"

namespace Math
{
    namespace SSE
    {
        // add subtract

        Float32Vector Subtract( Float32Vector a, Float32Vector b );
        Float32Vector SubtractSingle( Float32Vector a, Float32Vector b );
        Float32Vector Add( Float32Vector a, Float32Vector b );
        Float32Vector AddSingle( Float32Vector a, Float32Vector b );
        // Adds elements 1 and 3 and subtracts 0 and 2, thus the result is: (ax-bx, ay+by, az-bz, aw+bw).
        Float32Vector InterleavedAddAndSubtract( Float32Vector a, Float32Vector b );
        // result is ax + ay, az + aw, bx + by, bz + bw
        Float32Vector HorizontalAdd( Float32Vector a, Float32Vector b );
        // result is ax - ay, az - aw, bx - by, bz - bw
        Float32Vector HorizontalSubtract( Float32Vector a, Float32Vector b );
        // result is the equivalent of |a - b|
        Float32Vector AbsoluteDifference( Float32Vector a, Float32Vector b);

        // multiply divide

        Float32Vector Multiply( Float32Vector a, Float32Vector b );
        // Multiplies only the first value of a and b. The other three values are passed through from a.
        Float32Vector MultiplySingle( Float32Vector a, Float32Vector b );
        // a * b + c
        Float32Vector MultiplyAdd( Float32Vector a, Float32Vector b, Float32Vector c );
        Float32Vector Divide( Float32Vector a, Float32Vector b );
        Float32Vector Reciproce(Float32Vector a);
        // reciproce of ax, rest gets passed through
        Float32Vector ReciproceSingle(Float32Vector a);

        // square roots
        Float32Vector SquareRoot(Float32Vector f);
        // square root for the first value, the rest gets passed through
        Float32Vector SquareRootSingle( Float32Vector f );
        Float32Vector ReciprocalSquareRoot(Float32Vector f);
        // reciproce square root for the first value, the rest gets passed through
        Float32Vector ReciprocalSquareRootSingle( Float32Vector f );

        // dot product

        template<uint8_t Mask> Float32Vector Dot( Float32Vector a, Float32Vector b );
        template<bool MultiplyX, bool MultiplyY, bool MultiplyZ, bool MultiplyW, bool WriteX, bool WriteY, bool WriteZ, bool WriteW> Float32Vector Dot( Float32Vector a, Float32Vector b );
        template<bool MultiplyX, bool MultiplyY, bool MultiplyZ, bool MultiplyW> Float32Vector Dot( Float32Vector a, Float32Vector b );
        // Dot product for the x and y components, writes to all
        Float32Vector Dot2( Float32Vector a, Float32Vector b );
        // Dot product for the x, y and z components, both multiplied and writes to all
        Float32Vector Dot3( Float32Vector a, Float32Vector b );
        // Dot product for all components (x, y, z, w) writes to all
        Float32Vector Dot4( Float32Vector a, Float32Vector b );
        // Dot product for the x and y components, writes to first component only
        Float32Vector Dot2Single( Float32Vector a, Float32Vector b );
        // Dot product for the x, y and z components, both multiplied and writes to first component only
        Float32Vector Dot3Single( Float32Vector a, Float32Vector b );
        // Dot product for all components (x, y, z, w) writes to first component only
        Float32Vector Dot4Single( Float32Vector a, Float32Vector b );

        // other products

        // 3d cross product
        // Produce a result with the last element zero regardless of the input
        Float32Vector VECTOR_CALL Cross( Float32Vector a, Float32Vector b );

        // comparison

        // compares a < b per component, if it evaluates to true all bits will be set for that component
        Float32Vector LessThan( Float32Vector a, Float32Vector b );
        // compares a < b for the first component, if it evaluates to true all bits will be set for that component, the rest will be passed on from a
        Float32Vector LessThanSingle( Float32Vector a, Float32Vector b );
        // compares a <= b per component, if it evaluates to true all bits will be set for that component
        Float32Vector LessThanOrEqual( Float32Vector a, Float32Vector b );
        // compares a > b per component, if it evaluates to true all bits will be set for that component
        Float32Vector GreaterThan( Float32Vector a, Float32Vector b );
        // compares a > b for the first component, if it evaluates to true all bits will be set for that component, the rest will be passed on from a
        Float32Vector GreaterThanSingle( Float32Vector a, Float32Vector b );
        // compares a >= b per component, if it evaluates to true all bits will be set for that component
        Float32Vector GreaterThanOrEqual( Float32Vector a, Float32Vector b );

        // makes a mask of the sign bits
        unsigned MaskSignBits( Float32Vector a );
        bool AllSignBitsSet( Float32Vector a );
        bool AnySignBitsSet( Float32Vector a );

        // value stuff

        Float32Vector Min( Float32Vector a, Float32Vector b );
        Float32Vector Max( Float32Vector a, Float32Vector b );
        Float32Vector MinSingle( Float32Vector a, Float32Vector b );
        Float32Vector MaxSingle( Float32Vector a, Float32Vector b );
        Float32Vector VECTOR_CALL Clamp( Float32Vector min, Float32Vector max, Float32Vector input );
        Float32Vector VECTOR_CALL ClampSingle( Float32Vector min, Float32Vector max, Float32Vector input );
        // inverts the sign
        Float32Vector Negate( Float32Vector a );
        Float32Vector NegateSingle( Float32Vector a );
        Float32Vector Abs( Float32Vector a );
        Float32Vector AbsSingle( Float32Vector a );
        // same as Abs, but produces -|a|
        Float32Vector Negative( Float32Vector a );
        Float32Vector VECTOR_CALL CopySign( Float32Vector magnitude, Float32Vector sign );
        Float32Vector Floor( Float32Vector val );

        // bitwise

        // Computes the bitwise XOR of the four single-precision, floating-point values of a and b.
        Float32Vector ExclusiveOr( Float32Vector a, Float32Vector b );
        // Computes the bitwise AND of the four single-precision, floating-point values of a and b.
        Float32Vector And( Float32Vector a, Float32Vector b );
        // Computes the bitwise OR of the four single-precision, floating-point values of a and b.
        Float32Vector Or( Float32Vector a, Float32Vector b );
        // Computes the bitwise AND of the four single-precision, floating-point values of a and a negated b.
        Float32Vector AndNot( Float32Vector a, Float32Vector b );

        // value moves

        // select values from a or b, when true it selects the element from b, otherwise a
        template<bool X, bool Y, bool Z, bool W> Float32Vector Blend( Float32Vector a, Float32Vector b );
        // select values from a or b, select from b when the sign bit of mask is active, thus SignBit(mask) ? b : a
        Float32Vector Blend( Float32Vector a, Float32Vector b, Float32Vector mask );
        // select from a and b, Index0 and Index1 select an element from a and Index2 and Index3 from b
        template<unsigned Index0 = 0, unsigned Index1 = 1, unsigned Index2 = 2, unsigned Index3 = 3> Float32Vector Shuffle( Float32Vector a, Float32Vector b );
        // change around the values in a vector
        template<unsigned Index0, unsigned Index1 = 1, unsigned Index2 = 2, unsigned Index3 = 3> Float32Vector Swizzle( Float32Vector val );
        // Selects and interleaves the lower two single-precision, floating-point values from a and b.
        // r.x = a.x
        // r.y = b.x
        // r.z = a.y
        // r.w = b.y
        Float32Vector InterleaveLow( Float32Vector a, Float32Vector b );
        // Selects and interleaves the higher two single-precision, floating-point values from a and b.
        // r.x = a.z
        // r.y = b.z
        // r.z = a.w
        // r.w = b.w
        Float32Vector InterleaveHigh( Float32Vector a, Float32Vector b );


        // transposes four rows as if they were a 4x4 matrix
        void VECTOR_CALL Transpose( Float32Vector & row0, Float32Vector & row1, Float32Vector & row2, Float32Vector & row3 );


        // linear interpolation: r = a * blend_factor + b * (1 - blend_factor)
        Float32Vector VECTOR_CALL Lerp( Float32Vector a, Float32Vector b, Float32Vector blend_factor );
        // linear interpolation: r = a * blend_factor + b * (1 - blend_factor)
        FloatMatrix VECTOR_CALL Lerp( FloatMatrix a, FloatMatrix const & b, Float32Vector blend_factor );
        // normalized linear interpolation on all four elements
        Float32Vector VECTOR_CALL Nlerp4( Float32Vector a, Float32Vector b, Float32Vector blend_factor );

        // normalize, taking into account all four elements
        Float32Vector VECTOR_CALL Normalize4( Float32Vector f);
    }
}

// implemtnations
namespace Math
{
    namespace SSE
    {

        template<uint8_t Mask>
        inline Float32Vector Dot( Float32Vector a, Float32Vector b )
        {
            return _mm_dp_ps( a, b, Mask );
        }


        template<bool MultiplyX, bool MultiplyY, bool MultiplyZ, bool MultiplyW, bool WriteX, bool WriteY, bool WriteZ, bool WriteW>
        inline Float32Vector Dot( Float32Vector a, Float32Vector b )
        {
            auto const mask = ( MultiplyX * ( 1 << 4 ) ) + ( MultiplyY * ( 1 << 5 ) ) |
                ( MultiplyZ * ( 1 << 6 ) ) | ( MultiplyW * ( 1 << 7 ) ) |
                ( WriteX * ( 1 << 0 ) ) | ( WriteY * ( 1 << 1 ) ) |
                ( WriteZ * ( 1 << 2 ) ) | ( WriteW * ( 1 << 3 ) );
            return Dot<mask>( a, b );
        }


        template<bool MultiplyX, bool MultiplyY, bool MultiplyZ, bool MultiplyW>
        inline Float32Vector Dot( Float32Vector a, Float32Vector b )
        {
            return Dot<MultiplyX, MultiplyY, MultiplyZ, MultiplyW, 1, 1, 1, 1>( a, b );
        }


        // Dot product for the x and y components, writes to all
        inline Float32Vector Dot2( Float32Vector a, Float32Vector b )
        {
            return Dot<1, 1, 0, 0, 1, 1, 1, 1>( a, b );
        }


        // Dot product for the x, y and z components, writes to all
        inline Float32Vector Dot3( Float32Vector a, Float32Vector b )
        {
            return Dot<1, 1, 1, 0, 1, 1, 1, 1>( a, b );
        }


        // Dot product for all components (x, y, z, w) writes to all
        inline Float32Vector Dot4( Float32Vector a, Float32Vector b )
        {
            return Dot<1, 1, 1, 1, 1, 1, 1, 1>( a, b );
        }


        inline Float32Vector Dot2Single( Float32Vector a, Float32Vector b )
        {
            return Dot<1,1,0,0,1,0,0,0>( a, b );
        }


        inline Float32Vector Dot3Single( Float32Vector a, Float32Vector b )
        {
            return Dot<1,1,1,0,1,0,0,0>( a, b );
        }


        inline Float32Vector Dot4Single( Float32Vector a, Float32Vector b )
        {
            return Dot<1,1,1,1,1,0,0,0>( a, b );
        }



        inline Float32Vector Multiply( Float32Vector a, Float32Vector b )
        {
            return _mm_mul_ps( a, b );
        }


        inline Float32Vector MultiplyAdd( Float32Vector a, Float32Vector b, Float32Vector c )
        {
#ifdef AVX
            return _mm_fmadd_ps(a, b, c);
#else
            return Add(Multiply(a, b), c);
#endif
        }


        inline Float32Vector Divide( Float32Vector a, Float32Vector b )
        {
            return _mm_div_ps( a, b );
        }


        // Multiplies only the first value of a and b. The other three values are passed through from a.
        inline Float32Vector MultiplySingle( Float32Vector a, Float32Vector b )
        {
            return _mm_mul_ss( a, b );
        }


        inline Float32Vector Reciproce(Float32Vector a)
        {
            return _mm_rcp_ps(a);
        }


        inline Float32Vector ReciproceSingle(Float32Vector a)
        {
            return _mm_rcp_ss(a);
        }


        inline Float32Vector Subtract( Float32Vector a, Float32Vector b )
        {
            return _mm_sub_ps( a, b );
        }


        inline Float32Vector SubtractSingle( Float32Vector a, Float32Vector b )
        {
            return _mm_sub_ss( a, b );
        }


        inline Float32Vector Add( Float32Vector a, Float32Vector b )
        {
            return _mm_add_ps( a, b );
        }


        inline Float32Vector AddSingle( Float32Vector a, Float32Vector b )
        {
            return _mm_add_ss( a, b );
        }


        inline Float32Vector SquareRoot(Float32Vector f)
        {
            return _mm_sqrt_ps(f);
        }


        inline Float32Vector SquareRootSingle( Float32Vector f )
        {
            return _mm_sqrt_ss( f );
        }


        inline Float32Vector ReciprocalSquareRoot(Float32Vector f)
        {
            return _mm_rsqrt_ps(f);
        }


        inline Float32Vector ReciprocalSquareRootSingle( Float32Vector f )
        {
            return _mm_rsqrt_ss( f );
        }


        inline Float32Vector InterleavedAddAndSubtract( Float32Vector a, Float32Vector b )
        {
            return _mm_addsub_ps( a, b );
        }


        inline Float32Vector HorizontalAdd( Float32Vector a, Float32Vector b )
        {
            return _mm_hadd_ps( a, b );
        }


        inline Float32Vector HorizontalSubtract( Float32Vector a, Float32Vector b )
        {
            return _mm_hsub_ps( a, b );
        }


        inline Float32Vector AbsoluteDifference( Float32Vector a, Float32Vector b )
        {
            return Max( Subtract( a, b ), Subtract( b, a ) );
        }


        // compares a < b per component, if it evaluates to true all bits will be set for that component
        inline Float32Vector LessThan( Float32Vector a, Float32Vector b )
        {
            return _mm_cmplt_ps( a, b );
        }

        // compares a < b for the first component, if it evaluates to true all bits will be set for that component, the rest will be passed on from a
        inline Float32Vector LessThanSingle( Float32Vector a, Float32Vector b )
        {
            return _mm_cmplt_ss( a, b );
        }


        // compares a <= b per component, if it evaluates to true all bits will be set for that component
        inline Float32Vector LessThanOrEqual( Float32Vector a, Float32Vector b )
        {
            return _mm_cmple_ps( a, b );
        }


        // compares a > b per component, if it evaluates to true all bits will be set for that component
        inline Float32Vector GreaterThan( Float32Vector a, Float32Vector b )
        {
            return _mm_cmpgt_ps( a, b );
        }

        // compares a > b for the first component, if it evaluates to true all bits will be set for that component, the rest will be passed on from a
        inline Float32Vector GreaterThanSingle( Float32Vector a, Float32Vector b )
        {
            return _mm_cmpgt_ss( a, b );
        }


        inline Float32Vector GreaterThanOrEqual( Float32Vector a, Float32Vector b )
        {
            return _mm_cmpge_ps(a, b);
        }


        inline unsigned MaskSignBits( Float32Vector a )
        {
            return static_cast<unsigned>(_mm_movemask_ps( a ));
        }


        inline bool AllSignBitsSet( Float32Vector a )
        {
            return MaskSignBits( a ) == ( 1 | 1 << 1 | 1 << 2 | 1 << 3 );
        }


        inline bool AnySignBitsSet( Float32Vector a )
        {
            return MaskSignBits( a ) != 0;
        }


        inline Float32Vector ExclusiveOr( Float32Vector a, Float32Vector b )
        {
            return _mm_xor_ps( a, b );
        }


        // Computes the bitwise AND of the four single-precision, floating-point values of a and b.
        inline Float32Vector And( Float32Vector a, Float32Vector b )
        {
            return _mm_and_ps( a, b );
        }


        inline Float32Vector AndNot( Float32Vector a, Float32Vector b )
        {
            return _mm_andnot_ps( b, a );
        }


        inline Float32Vector Or( Float32Vector a, Float32Vector b )
        {
            return _mm_or_ps( a, b );
        }


        inline Float32Vector Min( Float32Vector a, Float32Vector b )
        {
            return _mm_min_ps( a, b );
        }


        inline Float32Vector Max( Float32Vector a, Float32Vector b )
        {
            return _mm_max_ps( a, b );
        }


        inline Float32Vector MinSingle( Float32Vector a, Float32Vector b )
        {
            return _mm_min_ss( a, b );
        }


        inline Float32Vector MaxSingle( Float32Vector a, Float32Vector b )
        {
            return _mm_max_ss( a, b );
        }


        inline Float32Vector Negate( Float32Vector a )
        {
            return Subtract( ZeroFloat32Vector(), a );
        }


        inline Float32Vector NegateSingle( Float32Vector a )
        {
            return SubtractSingle( ZeroFloat32Vector(), a );
        }


        inline Float32Vector Abs( Float32Vector a )
        {
            //Float32Vector const mask = SetAll( -0.f );
            //return AndNot( a, mask );
            return And( a, Negate( a ) );
        }


        inline Float32Vector AbsSingle( Float32Vector a )
        {
            //Float32Vector const mask = SetAll( -0.f );
            //return AndNot( a, mask );
            return MaxSingle( a, NegateSingle( a ) );
        }


        inline Float32Vector Negative( Float32Vector a )
        {
            //Float32Vector const mask = SetAll( -0.f );
            //return Or( a, mask );
            return Or( a, Negate( a ) );
        }


        // Selects and interleaves the lower two single-precision, floating-point values from a and b.
        // r.x = a.x
        // r.y = b.x
        // r.z = a.y
        // r.w = b.y
        inline Float32Vector InterleaveLow( Float32Vector a, Float32Vector b )
        {
            return _mm_unpacklo_ps( a, b );
        }


        // Selects and interleaves the higher two single-precision, floating-point values from a and b.
        // r.x = a.z
        // r.y = b.z
        // r.z = a.w
        // r.w = b.w
        inline Float32Vector InterleaveHigh( Float32Vector a, Float32Vector b )
        {
            return _mm_unpackhi_ps( a, b );
        }


        // select values from a or b, when true it selects the element from b, otherwise a
        template<bool X, bool Y, bool Z, bool W> inline Float32Vector Blend( Float32Vector a, Float32Vector b )
        {
            const int mask = X | ( Y * 2 ) | ( Z * 4 ) | ( W * 8 );
            return _mm_blend_ps( a, b, mask );
        }


        template<> inline Float32Vector Blend<1, 0, 0, 0>( Float32Vector a, Float32Vector b )
        {
            return _mm_move_ss( a, b );
        }


        template<> inline Float32Vector Blend<0, 1, 1, 1>( Float32Vector a, Float32Vector b )
        {
            return _mm_move_ss( b, a );
        }


        // select values from a or b, select from b when the sign bit of mask is active, thus mask < +0 ? b : a
        inline Float32Vector Blend( Float32Vector a, Float32Vector b, Float32Vector mask )
        {
            return _mm_blendv_ps( a, b, mask );
        }


        // select from a and b, Index0 and Index1 select an element from a and Index2 and Index3 from b
        template<unsigned Index0, unsigned Index1, unsigned Index2, unsigned Index3>
        inline Float32Vector Shuffle( Float32Vector a, Float32Vector b )
        {
            static_assert( Index0 < 4, "Index0 exceeds dimensions." );
            static_assert( Index1 < 4, "Index1 exceeds dimensions." );
            static_assert( Index2 < 4, "Index2 exceeds dimensions." );
            static_assert( Index3 < 4, "Index3 exceeds dimensions." );
            return _mm_shuffle_ps( a, b, _MM_SHUFFLE( Index3, Index2, Index1, Index0 ) );
        }


        template<unsigned Index0, unsigned Index1, unsigned Index2, unsigned Index3>
        inline Float32Vector Swizzle( Float32Vector val )
        {
            return Shuffle<Index0, Index1, Index2, Index3>( val, val );
        }

        // specializations for shuffle equivalents of other intrinsic functions

        // select from a and b, Index0 and Index1 select an element from a and Index2 and Index3 from b
        template<>
        inline Float32Vector Shuffle<0, 1, 0, 1>( Float32Vector a, Float32Vector b )
        {
            return _mm_movelh_ps( a, b );
        }

        // select from a and b, Index0 and Index1 select an element from a and Index2 and Index3 from b
        template<>
        inline Float32Vector Shuffle<2, 3, 2, 3>( Float32Vector a, Float32Vector b )
        {
            return _mm_movehl_ps( b, a );
        }

        // specializations for swizzle equivalents of other intrinsic functions

        template<>
        inline Float32Vector Swizzle<0, 0, 1, 1>( Float32Vector val )
        {
            return InterleaveLow( val, val );
        }


        template<>
        inline Float32Vector Swizzle<2, 2, 3, 3>( Float32Vector val )
        {
            return InterleaveHigh( val, val );
        }

        template<>
        inline Float32Vector Swizzle<1, 1, 3, 3>( Float32Vector val )
        {
            return _mm_movehdup_ps( val );
        }

        template<>
        inline Float32Vector Swizzle<0, 0, 2, 2>( Float32Vector val )
        {
            return _mm_moveldup_ps( val );
        }


        inline Float32Vector Floor( Float32Vector val )
        {
            return _mm_floor_ps( val );
        }
    }
}
