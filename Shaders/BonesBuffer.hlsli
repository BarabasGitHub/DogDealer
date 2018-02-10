#ifndef BONES_BUFFER_GUARD
#define BONES_BUFFER_GUARD
#include "Types.hlsli"
cbuffer Bones
{
    // real max:
    // AffineTransform BoneMatrices[1365];
    // but smaller sizes compile much faster
    AffineTransform BoneMatrices[64];
};
#endif