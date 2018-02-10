#ifndef HLSL_TRANSFORMS_GUARD
#define HLSL_TRANSFORMS_GUARD

#include "Types.hlsli"

#include "ObjectBuffer.hlsli"
#include "CameraBuffer.hlsli"
#include "SingleLightTransformBuffer.hlsli"

Position TransformPosition(Position position, float4x3 AffineTransform)
{
    return mul(float4(position, 1), AffineTransform);
    // return mul(position, (float3x3)AffineTransform) + AffineTransform[3];
}


Position RotatePosition(Position position, float3x3 RotationTransform)
{
    return mul(position, RotationTransform);
}


Normal RotateNormal(Normal normal, float3x3 RotationTransform)
{
    return mul(normal, RotationTransform);
}


ProjectedPosition ProjectPosition(Position position, float4x4 projection_transform)
{
    return mul(float4(position, 1), projection_transform);
}


Position RelativeWorldFromObjectPosition(Position position)
{
    return TransformPosition(position, TransformMatrix);
}


Position RelativeWorldFromWorldPosition(Position position)
{
    return position - CameraPosition;
}


Position WorldFromRelativeWorldPosition(Position position)
{
    return position + CameraPosition;
}


Position CameraFromRelativeWorldPosition(Position position)
{
    return RotatePosition(position, CameraMatrix);
}


ProjectedPosition ScreenFromCameraPosition(Position position)
{
    return ProjectPosition(position, ProjectionMatrix);
}


ProjectedPosition ScreenFromRelativeWorldPosition(Position position)
{
    return ScreenFromCameraPosition(CameraFromRelativeWorldPosition(position));
}


ProjectedPosition ScreenFromObjectPosition(Position position)
{
    return ScreenFromRelativeWorldPosition(RelativeWorldFromObjectPosition(position));
}


ProjectedPosition LightFromRelativeWorldPosition(Position position)
{
    return ProjectPosition(position, LightMatrix);
}


ProjectedPosition LightFromObjectPosition(Position position)
{
    return LightFromRelativeWorldPosition(RelativeWorldFromObjectPosition(position));
}


Normal WorldFromObjectNormal(Normal normal)
{
    return RotateNormal(normal, (float3x3)TransformMatrix);
}

#endif