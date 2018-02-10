#include "PositionShadowNormalTextureStruct.hlsli"
#include "ShadowCoordinates.hlsli"
#include "Transforms.hlsli"
#include "Types.hlsli"

PositionShadowNormalTexture main( Position position_in : POSITION, Normal normal_in : NORMAL, float2 uv : TEXTURE, AffineTransform local_transform_matrix : INSTANCE_WORLDMATRIX )
{
    PositionShadowNormalTexture output;

    Position position = TransformPosition(position_in, local_transform_matrix );
    position = RelativeWorldFromObjectPosition(position);
    ShadowCoordinates( position, output.shadow.position );
    position = CameraFromRelativeWorldPosition(position);
    output.shadow.depth = position.z;
    output.position = ScreenFromCameraPosition(position);

    Normal normal = RotateNormal( normal_in, ( float3x3 ) local_transform_matrix );
    output.normal = WorldFromObjectNormal(normal);

    output.uv = uv;
    return output;
}