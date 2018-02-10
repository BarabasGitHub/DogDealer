#include "PositionColorShadowNormalWorldStruct.hlsli"
#include "Lighting.hlsli"

#include "Noise.hlsli"

float4 main( PositionColorShadowNormalWorld Input ) : SV_TARGET
{
    float3 noise = 1;
    float gradient1;
    float3 gradient3;
    noise.r *= SimplexNoise( Input.world_position.x / 11 + 123, gradient1 );
    noise.g *= SimplexNoise( Input.world_position.y / 15, gradient1 );
    noise.b *= SimplexNoise( Input.world_position.z / 10 - 23, gradient1 );
    noise *= SimplexNoise( (Input.world_position.xzy) / 20, gradient3 );
    return CalculateColor(Input.color * float4(.5 + noise *.5, 1), Input.normal, Input.shadow);
}