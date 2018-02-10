#pragma once

#include <Math\FloatTypes.h>

namespace BoundingShapes
{
    struct Plane
    {
        Math::Float3 normal;
        float distance;
    };
}