#pragma once

#include <Math\FloatTypes.h>

#include <array>
#include <vector>

namespace BoundingShapes
{
    template<typename ShapeType>
    struct BoundingShapeHierarchyMesh
    {
        struct Node
        {
            union
            {
                ShapeType shape;
                std::array<unsigned, 3> vertex_indices;
            };
            unsigned escape_index;
        };

        std::vector<Node> nodes;
        std::vector<Math::Float3> vertex_positions;
    };
}