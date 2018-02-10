#pragma once
namespace Graphics
{
    enum struct ConstantBufferType
    {
        Object,
        Camera,
        Projection,
        Bones,
        Sky,
        Light,
        SingleLightTransform,
        Terrain2D,
        Terrain2DPatch,
    };
}