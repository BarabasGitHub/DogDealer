#include "PositionShadowNormalTextureStruct.hlsli"
#include "Lighting.hlsli"

float4 main( PositionShadowNormalTexture Input ) : SV_TARGET
{
    return CalculateColor(Input.uv, Input.normal, Input.shadow);
}