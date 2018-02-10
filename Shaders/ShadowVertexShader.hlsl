#include "Transforms.hlsli"

float4 main( float3 position_in : POSITION ) : SV_POSITION
{
    return LightFromObjectPosition( position_in );
}