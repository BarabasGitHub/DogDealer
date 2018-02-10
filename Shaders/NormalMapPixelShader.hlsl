#include "PositionShadowTangentTexture.hlsli"
#include "TangentDirections.hlsli"
#include "Lighting.hlsli"

float4 main( PositionShadowTangentTexture Input ) : SV_TARGET
{
    float3 normal = NormalFromNormalMap( Input.uv, Input.tangent, Input.bitangent, Input.normal );
    return CalculateColor(Input.uv, normal, Input.shadow);
}