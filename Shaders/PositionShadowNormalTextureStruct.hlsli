#include "ShadowStruct.hlsli"
#include "Types.hlsli"
struct PositionShadowNormalTexture
{
    ProjectedPosition position : SV_POSITION;
    Normal normal   : NORMAL;
    float2 uv       : TEXTURE;
    ShadowData shadow;
};