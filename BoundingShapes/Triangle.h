#pragma once

#include <Math\FloatTypes.h>

#include <array>

namespace BoundingShapes
{
    struct Triangle
    {
        // origin, right corner, left corner
        std::array<Math::Float3, 3> corners;
    };
}