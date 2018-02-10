#ifndef OBJECT_BUFFER_GUARD
#define OBJECT_BUFFER_GUARD
#include "Types.hlsli"
cbuffer Object
{
    // float4x4 TransformMatrix4;
    AffineTransform TransformMatrix;
    float4 BaseColor;
};


// #define TransformMatrix (float4x3)TransformMatrix4

#endif