#include "PositionShadowNormalTextureStruct.hlsli"
#include "ShadowCoordinates.hlsli"
#include "Transforms.hlsli"
#include "Animating.hlsli"
#include "CameraBuffer.hlsli"

PositionShadowNormalTexture main( Position position_in : POSITION, Normal normal_in : NORMAL, float2 uv : TEXTURE, uint4 blend_index : BLENDINDICES, float4 blend_weight : BLENDWEIGHTS )
{
    Position position;
    Normal normal;
    ObjectFromBonePositionAndNormal(position_in, normal_in, blend_index, blend_weight, position, normal);
    PositionShadowNormalTexture output;
    position = RelativeWorldFromObjectPosition(position);
    ShadowCoordinates( position, output.shadow.position );
    position = CameraFromRelativeWorldPosition(position);
    output.shadow.depth = position.z;
    output.position = ScreenFromCameraPosition(position);

    output.normal = WorldFromObjectNormal(normal_in);
    output.uv = uv;
    return output;
}