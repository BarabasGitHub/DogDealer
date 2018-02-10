#include "ShadowStruct.hlsli"
#include "Types.hlsli"
struct PositionShadowTangentTexture
{
    ProjectedPosition position : SV_POSITION;
    Normal tangent : TANGENT;
    Normal bitangent : BITANGENT;
    Normal normal : NORMAL;
    float2 uv : TEXTURE;
    ShadowData shadow;
};