#pragma once

#include "AxisAlignedBox.h"

#include <array>
#include <cstdint>
#include <vector>

namespace BoundingShapes
{
    struct AxisAlignedBoxHierarchy
    {
        struct Node
        {
            static uint8_t const c_leaf_entries = sizeof( AxisAlignedBox ) / sizeof( uint32_t );
            union
            {
                // box for nodes
                AxisAlignedBox box;
                // index for leafs to your data
                std::array<uint32_t, c_leaf_entries> indices;
            };
            uint32_t escape_index;
        };

        std::vector<Node> nodes;
    };
}