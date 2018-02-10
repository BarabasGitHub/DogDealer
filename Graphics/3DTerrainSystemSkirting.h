#pragma once
#include "3DTerrainSystemStructs.h"

namespace Graphics{


    void ExtractBorderVertices(
        std::vector<DensityAndGradient> const & local_density_and_gradients,
        Math::Unsigned3 const grid_size,
        Math::Unsigned3 const cell_cube_count,
        Math::Float3 const cell_scale,
        Math::Float3 point_offset,
        VertexData& vd);
}