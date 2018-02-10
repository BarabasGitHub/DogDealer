#pragma once

#include <Math\FloatTypes.h>

namespace BoundingShapes
{
    struct AxisAlignedBox
    {
        Math::Float3 center;
        Math::Float3 extent;
    };

};