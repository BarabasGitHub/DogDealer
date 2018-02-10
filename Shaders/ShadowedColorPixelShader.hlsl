#include "PositionColorShadowNormalStruct.hlsli"
#include "Lighting.hlsli"

float4 main(PositionColorShadowNormal Input) : SV_TARGET
{
    return CalculateColor(Input.color, Input.normal, Input.shadow);
}