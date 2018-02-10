#include "PositionShadowTangentTexture.hlsli"
#include "ShadowCoordinates.hlsli"
#include "Transforms.hlsli"

PositionShadowTangentTexture main( float3 position_in : POSITION, float3 tangent_in : TANGENT, float3 bitangent_in : BITANGENT, int handedness : HANDEDNESS, float2 uv : TEXTURE )
{
    PositionShadowTangentTexture output;

    Position position = RelativeWorldFromObjectPosition(position_in);
    ShadowCoordinates( position, output.shadow.position );
    position = CameraFromRelativeWorldPosition(position);
    output.shadow.depth = position.z;
    output.position = ScreenFromCameraPosition(position);

    Normal tangent = WorldFromObjectNormal( tangent_in );
    Normal bitangent = WorldFromObjectNormal( bitangent_in );
    Normal normal = handedness * cross( tangent, bitangent );

    output.tangent = tangent;
    output.bitangent = bitangent;
    output.normal = normal;

    output.uv = uv;
    return output;
}