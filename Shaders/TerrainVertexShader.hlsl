#include "PositionShadowNormalTextureStruct.hlsli"
#include "ShadowCoordinates.hlsli"
#include "Transforms.hlsli"
#include "CameraBuffer.hlsli"
#include "Terrain2D.hlsli"

PositionShadowNormalTexture main( uint vertex_id : SV_VertexID )
{
    PositionShadowNormalTexture output;
    Position world_position;
    TerrainWorldPositionAndNormal(vertex_id, world_position, output.normal);
    Position position = RelativeWorldFromWorldPosition(world_position);
    ShadowCoordinates( position, output.shadow.position );
    position = CameraFromRelativeWorldPosition(position);
    output.shadow.depth = position.z;
    output.position = ScreenFromCameraPosition(position);

    float texture_stretch = .25;
    float2 uv = world_position.xy * texture_stretch;
    // float2 colors[] = {float2(1,1), float2(0,1), float2(1,0)};
    // uv = colors[TerrainPatchData.lod_level % 3] / float(TerrainPatchData.lod_level / 3 + 1);
    output.uv = uv;
    return output;
}