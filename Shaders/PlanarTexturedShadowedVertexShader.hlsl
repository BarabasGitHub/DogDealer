#include "PositionShadowNormalTextureStruct.hlsli"
#include "ShadowCoordinates.hlsli"
#include "Transforms.hlsli"
#include "CameraBuffer.hlsli"

PositionShadowNormalTexture main( float3 position_in : POSITION, float3 normal_in : NORMAL)
{
    PositionShadowNormalTexture output;
    Position position = RelativeWorldFromObjectPosition(position_in);
    ShadowCoordinates( position, output.shadow.position );
    position = CameraFromRelativeWorldPosition(position);
    output.shadow.depth = position.z;
    output.position = ScreenFromCameraPosition(position);

    output.normal = WorldFromObjectNormal(normal_in);

    float texture_stretch = .25;
    float2 uv = position_in.xy * texture_stretch;

    output.uv = uv;
    return output;
}