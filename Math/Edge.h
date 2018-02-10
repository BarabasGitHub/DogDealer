#pragma once

#include <vector>

namespace Math
{
    struct Edge
    {
        // indices of the vertices that this edge connects
        uint32_t source, destination;

        Edge() = default;
        Edge(uint32_t s, uint32_t d) : source(s), destination(d) {}
    };
}
