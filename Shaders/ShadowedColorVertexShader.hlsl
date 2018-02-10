#include "PositionColorShadowNormalStruct.hlsli"

#include "Transforms.hlsli"
#include "ShadowCoordinates.hlsli"


PositionColorShadowNormal main( float3 position_in : POSITION, float3 normal : NORMAL )
{
    PositionColorShadowNormal output;

    Position position = RelativeWorldFromObjectPosition(position_in);
    ShadowCoordinates( position, output.shadow.position );
    position = CameraFromRelativeWorldPosition(position);
    output.shadow.depth = position.z;
    output.position = ScreenFromCameraPosition(position);

    output.normal = WorldFromObjectNormal(normal);

    output.color = BaseColor;

    return output;
}