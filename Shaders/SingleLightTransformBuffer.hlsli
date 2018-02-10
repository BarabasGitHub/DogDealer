#ifndef SINGLE_LIGHT_TRANSFORM_BUFFER_GUARD
#define SINGLE_LIGHT_TRANSFORM_BUFFER_GUARD
cbuffer SingleLightTransform
{
    float4x4 LightMatrix;
};
#endif