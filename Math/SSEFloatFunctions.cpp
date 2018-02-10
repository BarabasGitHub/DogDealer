#include "SSEFloatFunctions.h"

#include "SSECastFunctions.h"
#include "SSEIntegerFunctions.h"
#include <cassert>

namespace Math
{
    namespace SSE
    {
        Float32Vector VECTOR_CALL Clamp( Float32Vector min, Float32Vector max, Float32Vector input )
        {
            return Min(max, Max(min, input));
            // auto l = LessThan(input, min);
            // auto g = GreaterThan(input, max);
            // return Or(Or(And(l, min), And(g, max)), AndNot(input, Or(l, g)));
        }


        Float32Vector VECTOR_CALL ClampSingle( Float32Vector min, Float32Vector max, Float32Vector input )
        {
            return MinSingle(max, MaxSingle(min, input));
        }


        Float32Vector VECTOR_CALL Cross( Float32Vector a, Float32Vector b )
        {
            auto t1 = Multiply( a, Swizzle<1, 2, 0>( b ) );
            auto t2 = Multiply( b, Swizzle<1, 2, 0>( a ) );
            auto result = Swizzle<1, 2, 0>( Subtract( t1, t2 ) );
            return result;
        }



        Float32Vector VECTOR_CALL Lerp( Float32Vector a, Float32Vector b, Float32Vector blend_factor )
        {
            return Add(a, Multiply( Subtract( b, a ), blend_factor) );
        }


        FloatMatrix VECTOR_CALL Lerp( FloatMatrix a, FloatMatrix const & b, Float32Vector blend_factor )
        {
            a.row[0] = Lerp( a.row[0], b.row[0], blend_factor );
            a.row[1] = Lerp( a.row[1], b.row[1], blend_factor );
            a.row[2] = Lerp( a.row[2], b.row[2], blend_factor );
            a.row[3] = Lerp( a.row[3], b.row[3], blend_factor );
            return a;
        }


        Float32Vector VECTOR_CALL Nlerp4( Float32Vector a, Float32Vector b, Float32Vector blend_factor )
        {
            auto lerped = Lerp(a, b, blend_factor);
            return Normalize4(lerped);
        }


        Float32Vector VECTOR_CALL Normalize4(Float32Vector v)
        {
            auto length = Dot4(v, v);
            assert(GetSingle(length) != 0);
            //auto rsr = ReciprocalSquareRoot(length);
            //return Multiply(v, rsr);
            return Divide(v, SquareRoot(length));
        }


        Float32Vector VECTOR_CALL CopySign( Float32Vector magnitude, Float32Vector sign )
        {
            auto abs_sign = Abs( sign );

            // if sign was + then it will be the same as |sign| and we will be left with 0
            // if sign was - then it will have the same value but differing sign and we will be left with -0
            // after the XOR
            sign = ExclusiveOr( sign, abs_sign );
            // sign is either -0.f or 0.f here

            // get rid of the sign of the magnitude
            magnitude = Abs( magnitude );
            // magnitude is positive, no sign bit set

            // combine -0/0 with |magnitude|
            auto result = Or( magnitude, sign );
            // result has magnitude of magnitude and sign of sign.
            return result;
        }

        // transposes four rows as if they were a 4x4 matrix
        void VECTOR_CALL Transpose( Float32Vector & row0, Float32Vector & row1, Float32Vector & row2, Float32Vector & row3 )
        {
            //_MM_TRANSPOSE4_PS( row0, row1, row2, row3 );

            // slightly faster than the TRANSPOSE4 macro
            //auto x01_y01 = InterleaveLow( row0, row1 );
            //auto z01_w01 = InterleaveHigh( row0, row1 );
            //auto x23_y23 = InterleaveLow( row2, row3 );
            //row0 = Shuffle<0, 1, 0, 1>( x01_y01, x23_y23 );
            //row1 = Shuffle<2, 3, 2, 3>( x01_y01, x23_y23 );
            //auto z23_w23 = InterleaveHigh( row2, row3 );
            //row2 = Shuffle<0, 1, 0, 1>( z01_w01, z23_w23 );
            //row3 = Shuffle<2, 3, 2, 3>( z01_w01, z23_w23 );

            // even faster than the floating point instructions
            auto x01_y01 = InterleaveLow( CastToIntegerFromFloat( row0 ), CastToIntegerFromFloat( row1 ) );
            auto x23_y23 = InterleaveLow( CastToIntegerFromFloat( row2 ), CastToIntegerFromFloat( row3 ) );
            auto r0 = InterleaveLow64( x01_y01, x23_y23 );
            auto r1 = InterleaveHigh64( x01_y01, x23_y23 );
            auto z01_w01 = InterleaveHigh( CastToIntegerFromFloat( row0 ), CastToIntegerFromFloat( row1 ) );
            row0 = CastToFloatFromInteger( r0 );
            row1 = CastToFloatFromInteger( r1 );
            auto z23_w23 = InterleaveHigh( CastToIntegerFromFloat( row2 ), CastToIntegerFromFloat( row3 ) );
            auto r2 = InterleaveLow64( z01_w01, z23_w23 );
            auto r3 = InterleaveHigh64( z01_w01, z23_w23 );
            row2 = CastToFloatFromInteger( r2 );
            row3 = CastToFloatFromInteger( r3 );
        }

    }
}
