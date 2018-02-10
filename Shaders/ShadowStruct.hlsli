#ifndef SHADOW_DATA_GUARD
#define SHADOW_DATA_GUARD
#include "LightConstants.hlsli"
#include "Types.hlsli"

struct ShadowData
{
    Position position[c_max_number_of_light_transforms] : SHADOW_POSITION;
    float depth : DEPTH;
};

#endif