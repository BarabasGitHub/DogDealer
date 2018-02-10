#ifndef LIGHT_BUFFER_GUARD
#define LIGHT_BUFFER_GUARD

#include "LightConstants.hlsli"

cbuffer Light
{
    // normal in world space
    float3 LightNormal;
    float3 LightColor;
    // world -> light -> projection
    float4x4 LightMatrices[c_max_number_of_light_transforms];
    // float4 LightFarPlaneDistances[(c_max_number_of_light_transforms + 3) / 4];
    float LightFarPlaneDistances[c_max_number_of_light_transforms];
};
#endif