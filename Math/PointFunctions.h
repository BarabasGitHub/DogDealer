#pragma once

#include "FloatTypes.h"
#include <Utilities\Range.h>

#include <cstdint>

namespace Math
{
    uint32_t FindFurthestPointIndex(Float3 const reference_point, Range<Float3 const * > positions );

    // finds the point furthest from the reference points in a quadratic sum sense (Is that how you call it?)
    uint32_t FindFurthestPointIndex(Range<Float3 const * __restrict> reference_points, Range<Float3 const  * __restrict> positions );

    uint32_t FindClosestPointIndex(uint32_t reference_index, Range<Float3 const *> points);
}