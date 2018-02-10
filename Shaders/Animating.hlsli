#include "Transforms.hlsli"

#include "BonesBuffer.hlsli"

Position ObjectFromBonePosition(Position position, uint index)
{
    return TransformPosition( position, (float4x3)BoneMatrices[index] );
}


Normal ObjectFromBoneNormal(Normal normal, uint index)
{
    return RotateNormal( normal, (float3x3)BoneMatrices[index] );
}


void ObjectFromBonePositionAndNormal(Position position_in, Normal normal_in, uint4 bone_indices, float4 bone_weights, out Position position, out Normal normal )
{
    position = 0;
    normal = 0;
    [unroll] for( uint i = 0; i < 4; ++i )
    {
        //if( bone_weights[i] == 0 ) break;
        position += ObjectFromBonePosition( position_in, bone_indices[i] ) * bone_weights[i];
        normal += ObjectFromBoneNormal( normal_in, bone_indices[i] ) * bone_weights[i];
    }
}


Position ObjectFromBonePosition(Position position_in, uint4 bone_indices, float4 bone_weights)
{
    Position position = 0;
    [unroll] for( uint i = 0; i < 4; ++i )
    {
        //if( bone_weights[i] == 0 ) break;
        position += ObjectFromBonePosition( position_in, bone_indices[i] ) * bone_weights[i];
    }
    return position;
}