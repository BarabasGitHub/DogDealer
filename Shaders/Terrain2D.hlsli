
#include "GridFunctions.hlsli"
#include "Terrain2DBuffer.hlsli"

Texture2D HeightMap;
SamplerState ClampedLinearSampler;

uint2 CalculateVertexIndex(uint vertex_id, uint2 edges_to_double)
{
    uint2 index2d = Calculate2DindexFrom1D(vertex_id, PatchDimensions.x);
    uint2 at_edge = index2d == edges_to_double;
    index2d = index2d & ~at_edge.yx;
    return index2d;
}


float2 CalculateVertexHorizontalWorldCoordinates(float2 start, uint2 vertex_index, uint lod_level)
{
    float2 size = PatchSize;
    float2 increase = size / float2(PatchDimensions - 1);
    vertex_index *= 1 << lod_level;
    float2 coordinate = start + increase * vertex_index;
    return coordinate;
}


float GetTerrainHeightAndNormal(uint2 vertex_index, uint2 offset, uint lod_level, out Normal normal)
{
    float4 normal_and_height = HeightMap.Load( uint3(vertex_index + offset, 0));
    // float4 heights;
    // heights.x = normal_and_height.a;
    // heights.x = HeightMap.Load( uint3(vertex_index + offset + uint2(0, 0), 0)).a;
    // heights.y = HeightMap.Load( uint3(vertex_index + uint2(1, 0) + offset, 0)).a;
    // heights.w = HeightMap.Load( uint3(vertex_index + uint2(0, 1) + offset, 0)).a;
    // heights.z = HeightMap.Load( uint3(vertex_index + uint2(1, 1) + offset, 0)).a;
    // float2 dimesions;
    // HeightMap.GetDimensions(dimesions.x, dimesions.y);
    // float4 normal_and_height = HeightMap.SampleLevel( ClampedLinearSampler, float2(vertex_index + offset) / (dimesions - 1), lod_level);
    float height = normal_and_height.w;
    // // the directx11 presentation says it should be like
    // // w | z
    // // -----
    // // x | y
    // heights = HeightMap.GatherAlpha( ClampedLinearSampler, float2(vertex_index + offset) / (dimesions - 1), lod_level);
    // heights = HeightMap.GatherRed( ClampedLinearSampler, float2(vertex_index + offset) / (dimesions - 1), lod_level);
    // float2 step_size = CalculateVertexHorizontalWorldCoordinates(0, uint2(1, 1), lod_level);
    // float3 t = float3(2 * step_size.x, 0,  heights.y - heights.x + heights.z - heights.w);
    // float3 b = float3(0, 2 * step_size.y, heights.w - heights.x + heights.z - heights.y);
    // normal = normalize(cross(t,b));
    // return heights.x;
    normal = normalize(normal_and_height.xyz);
    return height;
}


void TerrainWorldPositionAndNormal(uint vertex_id, out Position position, out Normal normal)
{
    uint2 vertex_index = CalculateVertexIndex(vertex_id, TerrainPatchData.edges_to_double);
    position.xy = CalculateVertexHorizontalWorldCoordinates(TerrainPatchData.start, vertex_index, TerrainPatchData.lod_level);
    position.z = GetTerrainHeightAndNormal(vertex_index, TerrainPatchData.texture_offset, TerrainPatchData.lod_level, normal);
}


Position TerrainWorldPosition(uint vertex_id)
{
    Position position;
    Normal unused;
    TerrainWorldPositionAndNormal(vertex_id, position, unused);
    return position;
}