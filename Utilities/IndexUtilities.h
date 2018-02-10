#pragma once

#include <Math\IntegerOperators.h>

#include <cstdint>

// i = x + count.x * y
inline Math::Unsigned2 constexpr Calculate2DindexFrom1D( uint32_t i, uint32_t count_x )
{
    return{i % count_x, i / count_x };
}


inline uint32_t constexpr Calculate1DindexFrom2D( Math::Unsigned2 index2d, uint32_t count_x )
{
    return index2d.x + count_x * index2d.y;
}


inline uint32_t constexpr Calculate1DindexFrom3D( Math::Unsigned3 index3d, uint32_t count_x, uint32_t count_y )
{
    return index3d.x + count_x * ( index3d.y + count_y * index3d.z );
}

// i = x + count.x * (y + count.y * z)
inline Math::Unsigned3 constexpr Calculate3DindexFrom1D( uint32_t i, uint32_t count_x, uint32_t count_y )
{
    // i = x + count.x * (y + count.y * z)
    //auto count_xy = count_x * count_y;
    //auto x = i;
    //auto y = i / count_x;
    //auto z = i / count_xy;
    //x -= count_x * y;
    //y -= z * count_y;

    return{ i - count_x * (i/count_x), i / count_x - (i/(count_x * count_y))  * count_y, i / (count_x * count_y) };
}


