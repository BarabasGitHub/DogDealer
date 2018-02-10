#pragma once

namespace Graphics
{
    enum struct TextureFiltering
    {
        NearestNeighbour,
        Linear,
        Anisotropic_2x,
        Anisotropic_4x,
        Anisotropic_8x,
        Anisotropic_16x
    };
}