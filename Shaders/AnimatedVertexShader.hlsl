#include "PlainPositionColorStruct.hlsli"
#include "Transforms.hlsli"
#include "CameraBuffer.hlsli"
#include "Animating.hlsli"

PositionColor main( float3 position_in : POSITION, uint4 blend_index : BLENDINDICES, float4 blend_weight : BLENDWEIGHTS )
{
    Position position = ObjectFromBonePosition(position_in, blend_index, blend_weight);
    PositionColor output;
    output.position = ScreenFromObjectPosition(position);
    output.color = BaseColor;
    return output;
}