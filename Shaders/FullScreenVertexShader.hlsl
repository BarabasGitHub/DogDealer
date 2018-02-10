#include "PlainPositionColorStruct.hlsli"

PositionColor main( uint vertex_index : SV_VertexID  )
{
    const float depth = 1;
    const float4 positions[3] = { float4( 3, 1, depth, 1 ), float4( -1, 1, depth, 1 ), float4( -1, -3, depth, 1 ) };
    PositionColor output;
    output.position = positions[vertex_index];
    output.color = float4(1,1,1,1);
    return output;
}