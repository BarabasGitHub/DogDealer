#include "LightConstants.hlsli"
#include "LightBuffer.hlsli"
#include "ShadowStruct.hlsli"

Texture2DArray<float> ShadowMap;
SamplerComparisonState ShadowSampler;

float Shadow( in ShadowData shadow_data, out uint shadow_map_index )
{
    shadow_map_index = 0;
    while( shadow_map_index < c_max_number_of_light_transforms )
    {
        if( LightFarPlaneDistances[shadow_map_index] < shadow_data.depth ) break;
        ++shadow_map_index;
    }
    float3 position = shadow_data.position[shadow_map_index];
    float depth = position.z;
    position.z = shadow_map_index;
    float shade = ShadowMap.SampleCmpLevelZero( ShadowSampler, position.xyz, depth );
    return shade;
}


float Shadow( in ShadowData shadow_data )
{
    uint index;
    return Shadow(shadow_data, index);
}


float DebugCascadedShadowMaps(in ShadowData shadow_data, out float3 color, out uint shadow_map_index)
{
    float3 colors[] = {float3(1,1,0), float3(0,1,0), float3(0,1,1), float3(0,0,1), float3(1,0,1), float3(1,0,0), float3(1,1,1)};
    float shadow = Shadow(shadow_data, shadow_map_index);
    color = colors[shadow_map_index];
    return shadow;
}


float DebugCascadedShadowMaps(in ShadowData shadow_data, out float3 color)
{
    uint shadow_map_index;
    return DebugCascadedShadowMaps(shadow_data, color, shadow_map_index);
}