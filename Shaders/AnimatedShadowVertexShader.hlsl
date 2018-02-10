#include "Animating.hlsli"
#include "Transforms.hlsli"

float4 main( float3 position_in : POSITION, uint4 blend_index : BLENDINDICES, float4 blend_weight : BLENDWEIGHTS ) : SV_POSITION
{
    Position position = ObjectFromBonePosition(position_in, blend_index, blend_weight);
    return LightFromObjectPosition(position);
}