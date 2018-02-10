#include "PlainPositionColorStruct.hlsli"
#include "Transforms.hlsli"

PositionColor main( float3 position_in : POSITION )
{
    float3 local_corner_position = mul(abs(position_in), abs((float3x3)TransformMatrix)) * sign(position_in) + TransformMatrix[3].xyz;
    PositionColor output;
    output.position = ScreenFromRelativeWorldPosition(local_corner_position);
    output.color = BaseColor;
    return output;
}