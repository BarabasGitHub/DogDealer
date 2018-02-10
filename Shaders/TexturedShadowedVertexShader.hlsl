#include "PositionShadowNormalTextureStruct.hlsli"
#include "ShadowCoordinates.hlsli"
#include "Transforms.hlsli"

PositionShadowNormalTexture main( float3 position_in : POSITION, float3 normal_in : NORMAL, float2 uv : TEXTURE )
{
    PositionShadowNormalTexture output;

    Position position = RelativeWorldFromObjectPosition(position_in);
    ShadowCoordinates( position, output.shadow.position );
    position = CameraFromRelativeWorldPosition( position );
    output.shadow.depth = position.z;
    output.position = ScreenFromCameraPosition( position );

    output.normal = WorldFromObjectNormal( normal_in );

    output.uv = uv;
    return output;
}