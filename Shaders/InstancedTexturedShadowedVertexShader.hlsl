#include "PositionShadowNormalTextureStruct.hlsli"
#include "ShadowCoordinates.hlsli"
#include "Transforms.hlsli"
#include "Types.hlsli"

PositionShadowNormalTexture main( Position position_in : POSITION, Normal normal_in : NORMAL, float2 uv : TEXTURE, AffineTransform transform_matrix : INSTANCE_WORLDMATRIX )
{
    PositionShadowNormalTexture output;

    Position position = TransformPosition(position_in, transform_matrix );
    ShadowCoordinates( position, output.shadow.position );
    position = CameraFromRelativeWorldPosition(position);
    output.shadow.depth = position.z;
    output.position = ScreenFromCameraPosition(position);

    output.normal = RotateNormal( normal_in, (float3x3)transform_matrix );

    output.uv = uv;
    return output;
}