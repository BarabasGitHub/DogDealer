#include "ShadowStruct.hlsli"
#include "Types.hlsli"

struct PositionColorShadowNormalWorld
{
    ProjectedPosition position : SV_POSITION;
    float4 color    : COLOR;
    Normal normal : NORMAL;
	Position world_position : POSITION;
    ShadowData shadow;
};