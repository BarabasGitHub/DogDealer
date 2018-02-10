#pragma once

#include "FloatTypes.h"
#include "SSETypes.h"


namespace Math
{
    namespace SSE
    {
        Float32Vector Float32FromSignedInteger32(IntegerVector in);
        IntegerVector SignedInteger32FromFloat32(Float32Vector in);
        IntegerVector UnsignedInteger64FromInteger32(IntegerVector in);
        // only converts the first half
        IntegerVector SignedInteger16FromUnsignedInteger8(IntegerVector in);

        IntegerVector SignedInteger8FromSignedInteger16(IntegerVector half1, IntegerVector half2);
        IntegerVector UnsignedInteger8FromSignedInteger16(IntegerVector half1, IntegerVector half2);

        inline Float32Vector Float32FromSignedInteger32( IntegerVector in )
        {
            return _mm_cvtepi32_ps( in );
        }


        inline IntegerVector SignedInteger32FromFloat32(Float32Vector in)
        {
            return _mm_cvtps_epi32(in);
        }


        inline IntegerVector UnsignedInteger64FromInteger32(IntegerVector in)
        {
            return _mm_cvtepi32_epi64(in);
        }


        inline IntegerVector SignedInteger16FromUnsignedInteger8(IntegerVector in)
        {
            return _mm_cvtepu8_epi16(in);
        }


        inline IntegerVector UnsignedInteger8FromSignedInteger16(IntegerVector half1, IntegerVector half2)
        {
            return _mm_packus_epi16(half1, half2);
        }


        inline IntegerVector SignedInteger8FromSignedInteger16(IntegerVector half1, IntegerVector half2)
        {
            return _mm_packs_epi16(half1, half2);
        }

    }
}