#pragma once

#include <Math\FloatTypes.h>

#include <functional>

namespace Physics
{
    typedef std::function<void(Math::Float3 const position, float & density, Math::Float3 & gradient)> DensityFunctionType;
}