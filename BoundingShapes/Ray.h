#pragma once

#include <Math\FloatTypes.h>

namespace BoundingShapes
{
    struct Ray
    {
        Math::Float3 start;
        Math::Float3 direction; // should always be normalized
    };

}
