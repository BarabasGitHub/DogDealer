#include "PlainPositionColorStruct.hlsli"

float4 main(PositionColor Input) : SV_TARGET
{   
    float4 color = Input.color;
    //color.rgb *= 5 * Input.position.z / Input.position.w;     
	return color;
}