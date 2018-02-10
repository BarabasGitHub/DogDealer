#include "PositionShadowNormalTextureStruct.hlsli"
#include "Lighting.hlsli"
#include "DistanceAlpha.hlsli"

float4 main( PositionShadowNormalTexture Input ) : SV_TARGET
{
    float alpha = CalculateDistanceAlpha(Input.position);

    return CalculateColor(alpha, Input.uv, Input.normal, Input.shadow);
    // return saturate(float4(alpha, alpha, alpha, 1));
}
