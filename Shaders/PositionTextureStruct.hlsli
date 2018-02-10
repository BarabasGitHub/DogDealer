#include "Types.hlsli"
struct PositionTexture
{
    ProjectedPosition position : SV_POSITION;
    float2 uv : TEXTURE;
};