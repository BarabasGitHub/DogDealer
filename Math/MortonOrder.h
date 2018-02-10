#pragma once

#include "ForwardDeclarations.h"

#include "Utilities\Range.h"
#include <Utilities\MinMax.h>

#include <cstdint>
#include <string>

namespace Math
{

    // Only uses the first 21 bits of each component and generates a 64 bit index
    uint64_t MortonIndex64Bit(Math::Unsigned3 const & value);

    // Uses the full 32 bits of each component and generates a 64 bit index
    uint64_t MortonIndex64Bit(Math::Unsigned2 const & value);

    // Uses the first 16 bits of each component and generates a 32 bit index
    uint32_t MortonIndex32Bit(Math::Unsigned2 const & value);
    // Reverse of the above
    Math::Unsigned2 Index2DFromMorton(uint32_t index);

    uint32_t IncreaseX2D(uint32_t index, int32_t amount = 1);
    uint32_t IncreaseY2D(uint32_t index, int32_t amount = 1);

    // does stuff
    void ComputeMortonOrder( Range<Math::Float3 const *> points, MinMax<Float3> & minmax, Range<uint64_t *> morton_codes );

    std::string ToString2D(Range<uint32_t*> morton_data);
    std::string ToString2D(Range<uint32_t const*> morton_data);
}