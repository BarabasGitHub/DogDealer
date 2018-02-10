#pragma once

#include <vector>
#include <cstdint>

std::vector<uint32_t> ReorderFaces( std::vector<uint32_t> const & indices, unsigned vertex_count );