#include "PositionShadowNormalTextureStruct.hlsli"
#include "ShadowCoordinates.hlsli"
#include "Transforms.hlsli"
#include "CameraBuffer.hlsli"
#include "Terrain2D.hlsli"

ProjectedPosition main( uint vertex_id : SV_VertexID ) : SV_POSITION
{
    Position world_position = TerrainWorldPosition(vertex_id);
    Position position = RelativeWorldFromWorldPosition(world_position);
    return LightFromRelativeWorldPosition(position);
}