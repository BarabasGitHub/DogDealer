#ifndef TERRAIN_2D_BUFFER_GUARD
#define TERRAIN_2D_BUFFER_GUARD

cbuffer TerrainConstantData
{
    uint2 PatchDimensions;
    float2 PatchSize;
};


cbuffer TerrainPatchDataBuffer
{
    struct TerrainPatchDataStruct
    {
    float2 start;
    uint2 texture_offset;
    uint2 edges_to_double;
    uint lod_level;
    } TerrainPatchData;
};


#endif