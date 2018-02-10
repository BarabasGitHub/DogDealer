#pragma once

#include <Math\FloatTypes.h>

namespace BoundingShapes
{
    struct OrientedBox
    {
        Math::Float3 center;
        Math::Float3 extent;
        Math::Quaternion rotation;
    };

}