#pragma once

#include <intrin.h>
#include <cstdint>

namespace Math
{
    namespace SSE
    {
        // typedefs for the different vectors, so we don't have to use the ugly __________
        typedef __m128 Float32Vector;
        typedef __m128i IntegerVector;

        struct FloatMatrix
        {
            Float32Vector row[4];
        };

#define VECTOR_CALL __vectorcall

    }
}