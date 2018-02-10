#ifndef CAMERA_BUFFER_GUARD
#define CAMERA_BUFFER_GUARD

cbuffer Camera
{
    float4x4 InverseCameraProjectionMatrix;
    // float4x4 CameraMatrix4x4;
    float3x3 CameraMatrix;
    float3 CameraPosition;
};


cbuffer Projection
{
    float4x4 ProjectionMatrix;
};

#endif