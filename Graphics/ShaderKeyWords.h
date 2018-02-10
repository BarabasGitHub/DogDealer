#pragma once

#include <string>

namespace Graphics
{
    namespace KeyWord
    {
        // input names
        std::string const
            instance( "INSTANCE_" ),
            position( "POSITION" ),
            normal( "NORMAL" ),
            tangent( "TANGENT" ),
            bitangent( "BITANGENT" ),
            handedness( "HANDEDNESS" ),
            texture( "TEXTURE" ),
            color( "COLOR" ),
            blendindices( "BLENDINDICES" ),
            blendweights( "BLENDWEIGHTS" ),
            worldmatrix( "WORLDMATRIX" ),
        // constant buffer names
            bones( "Bones" ),
            camera( "Camera" ),
            light( "Light" ),
            object( "Object" ),
            projection( "Projection" ),
            singlelighttransform("SingleLightTransform"),
            sky( "Sky" ),
            terrain2d_constant("TerrainConstantData"),
            terrain2d_patch("TerrainPatchDataBuffer"),
        // texture names
            diffusemap( "DiffuseMap" ),
            normalmap( "NormalMap" ),
            specularmap( "SpecularMap" ),
            specmap( "SpecMap" ),
            shadowmap( "ShadowMap" ),
            heightmap( "HeightMap" ),
            detailmap( "DetailMap" );
    }
}