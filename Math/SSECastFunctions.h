#pragma once

#include "SSETypes.h"

namespace Math
{
    namespace SSE
    {
        inline IntegerVector CastToIntegerFromFloat( Float32Vector in )
        {
            return _mm_castps_si128( in );
        }


        inline Float32Vector CastToFloatFromInteger( IntegerVector in )
        {
            return _mm_castsi128_ps( in );
        }
    }
}