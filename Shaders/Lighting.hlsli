#include "ShadeFactor.hlsli"
#include "ShadowTest.hlsli"
#include "SkyBuffer.hlsli"
#include "LightBuffer.hlsli"

SamplerState LinearSampler;
Texture2D DiffuseMap;


float4 CalculateColor(float4 diffuse_color, float3 normal, ShadowData shadow_data )
{
    float3 ambient_color = .5;
    float light_factor = ShadeFactor( normal );
    float3 shadow_color = 0;
    // comment this line and uncomment the next to debug the cascaded shadow maps
    light_factor *= Shadow( shadow_data );
    // light_factor *= DebugCascadedShadowMaps( shadow_data, shadow_color);
    float3 light_color = lerp(shadow_color, LightColor, light_factor);
    float3 sky_color = SkyColor * saturate(normal.z + 1);
    float3 color = lerp(lerp(ambient_color, sky_color, 0.5), light_color, 0.5);
    return float4(diffuse_color.rgb * color, diffuse_color.a);
}


float4 CalculateColor(float2 uv, float3 normal, ShadowData shadow_data)
{
    float4 color = DiffuseMap.Sample( LinearSampler, uv );
    return CalculateColor(color, normal, shadow_data);
}


float4 CalculateColor(float alpha_factor, float2 uv, float3 normal, ShadowData shadow_data)
{
    float4 color = DiffuseMap.Sample( LinearSampler, uv );
    color.a *= alpha_factor;
    // clip(color.a - 0.5);
    return CalculateColor(color, normal, shadow_data);
}
