#pragma once

#include <Math\FloatTypes.h>
#include <vector>

namespace Logic
{
    struct NavigationMesh;
    struct PortalList;    

    std::vector<Math::Float2> FindPath(Math::Float3 const start,
                                    Math::Float3 const destination,
                                    NavigationMesh const & navigation_mesh);

    // For UnitTests
    float GetPointDistanceFromTriangle3D(Math::Float3 const position,
        Math::Float3 vertex_0,
        Math::Float3 vertex_1,
        Math::Float3 vertex_2);
}
