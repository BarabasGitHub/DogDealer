#include "PositionColorShadowNormalWorldStruct.hlsli"
#include "Transforms.hlsli"
#include "ShadowCoordinates.hlsli"


PositionColorShadowNormalWorld main( float3 position_in : POSITION, float3 normal : NORMAL )
{
    PositionColorShadowNormalWorld output;

    Position position = RelativeWorldFromObjectPosition(position_in);
    output.world_position = WorldFromRelativeWorldPosition(position);

    ShadowCoordinates( position, output.shadow.position );
    position = CameraFromRelativeWorldPosition(position);
    output.shadow.depth = position.z;
    output.position = ScreenFromCameraPosition(position);

    output.normal = WorldFromObjectNormal(normal);

    output.color = BaseColor;
    return output;
}