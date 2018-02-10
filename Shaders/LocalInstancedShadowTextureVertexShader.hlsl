#include "PositionTextureStruct.hlsli"
#include "Transforms.hlsli"
#include "Types.hlsli"

PositionTexture main( Position position_in : POSITION, float2 uv : TEXTURE, AffineTransform local_object_matrix : INSTANCE_WORLDMATRIX )
{
    Position position = TransformPosition( position_in, local_object_matrix );
    PositionTexture output;
    output.position = LightFromObjectPosition( position );
    output.uv = uv;
    return output;
}