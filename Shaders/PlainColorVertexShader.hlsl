#include "PlainPositionColorStruct.hlsli"
#include "Transforms.hlsli"


PositionColor main( float3 position_in : POSITION )
{
    PositionColor output;
    output.position = ScreenFromObjectPosition(position_in);
    output.color = BaseColor;
    return output;
}