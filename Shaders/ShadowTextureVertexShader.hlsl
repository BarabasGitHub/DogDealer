#include "Transforms.hlsli"
#include "PositionTextureStruct.hlsli"

PositionTexture main( Position position_in : POSITION, float2 uv : TEXTURE )
{
    Position position = RelativeWorldFromObjectPosition(position_in);
    PositionTexture output;
    output.position = LightFromRelativeWorldPosition( position );
    output.uv = uv;
    return output;
}