#include "PositionNormalStruct.hlsli"
#include "CameraBuffer.hlsli"


PositionNormal main( uint vertex_index : SV_VertexID  )
{
    const float depth = 1;
    PositionNormal output;
    vertex_index = vertex_index < 3 ? vertex_index : 6 - vertex_index;
    // const float4 positions[4] = { float4( -1, 1, depth, 1 ), float4( -1, -1, depth, 1 ), float4( 1, 1, depth, 1 ), float4(1, -1, depth, 1) };
    // output.position = positions[vertex_index];
    output.position.x = (vertex_index & 2) ? 1 : -1;
    output.position.y = (vertex_index & 1) ? -1 : 1;
    output.position.z = depth;
    output.position.w = 1;

    float4 corner4 = mul(output.position, InverseCameraProjectionMatrix);
    corner4.z += CameraPosition.z * corner4.w;
    // output.normal = normalize(corner4.xyz);
    output.normal = corner4.xyz;
    return output;
}