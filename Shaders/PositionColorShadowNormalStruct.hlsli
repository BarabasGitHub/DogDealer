#include "ShadowStruct.hlsli"
#include "Types.hlsli"
struct PositionColorShadowNormal
{
    ProjectedPosition position : SV_POSITION;
    float4 color    : COLOR;
    Normal normal : NORMAL;
    ShadowData shadow;
};