#include "PositionNormalStruct.hlsli"
#include "LightBuffer.hlsli"
#include "SkyBuffer.hlsli"

float3 main(PositionNormal Input) : SV_TARGET
{
    float3 direction = normalize(Input.normal);
    float latitude = saturate(direction.z);
    float3 sky_color = SkyColor;
    // float3 sky_color = float3(0.25, 0.45, 1.0);
    sky_color = lerp(length(sky_color)/sqrt(3), sky_color, pow(latitude, .2));
    // sky_color *= saturate(sqrt(latitude) + .1);
    float sunny = pow(saturate(1.025 - distance(direction, LightNormal)), 20);
    float3 color = lerp( sky_color, LightColor, sunny);
    return color;
}