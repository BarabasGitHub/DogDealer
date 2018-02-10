#include "MortonOrder.h"

#include "FloatOperators.h"
#include "Conversions.h"
#include "SSEIntegerFunctions.h"
#include "SSESetFunctions.h"
#include "SSELoadStore.h"
#include "SSEConversions.h"
#include "SSEMathConversions.h"
#include "MathFunctions.h"

#include <Utilities\MinMax.h>
#include <Utilities\Range.h>

#include <algorithm>
#include <cassert>
#include <vector>
#include <sstream>

namespace Math
{

    struct MortonVolume
    {
        MinMax<Float3> minmax;
        std::vector<uint64_t> codes;
    };

    namespace
    {

        constexpr auto half_mask(size_t bits)
        {
            return (1u << bits/2u) - 1u;
        }


        const uint32_t patterns_32bit[2][5] =
        {
            {
                // distance 1
                0x0000ffff,
                0x00ff00ff,
                0x0f0f0f0f,
                0x33333333,
                0x55555555,
            },
            {
                // distance 2
                0x000003ff,
                0x030000ff,
                0x0300f00f,
                0x030c30c3,
                0x09249249,
            }
        };

        const uint32_t shifts_32bit[2][4] = {{8, 4, 2, 1}, {16, 8, 4, 2}};

        const uint64_t patterns_64bit[2][6] =
        {
            {
                // distance 1
                0x00000000ffffffff,
                0x0000ffff0000ffff,
                0x00ff00ff00ff00ff,
                0x0f0f0f0f0f0f0f0f,
                0x3333333333333333,
                0x5555555555555555,
            },
            {
                // distance 2
                0x00000000001fffff,
                0x001f00000000ffff,
                0x001f0000ff0000ff,
                0x100f00f00f00f00f,
                0x10c30c30c30c30c3,
                0x1249249249249249,
            }
        };


        const uint32_t shifts_64bit[2][5] = {{16, 8, 4, 2, 1}, {32, 16, 8, 4, 2}};

        uint32_t Separate( uint32_t x, uint32_t distance )
        {
            x = (                    x                   ) & patterns_32bit[distance - 1][0];
            x = ( x | ( x << shifts_32bit[distance - 1][0] ) ) & patterns_32bit[distance - 1][1];
            x = ( x | ( x << shifts_32bit[distance - 1][1] ) ) & patterns_32bit[distance - 1][2];
            x = ( x | ( x << shifts_32bit[distance - 1][2] ) ) & patterns_32bit[distance - 1][3];
            x = ( x | ( x << shifts_32bit[distance - 1][3] ) ) & patterns_32bit[distance - 1][4];
            return x;
        }


        // processes four elements at the same time
        SSE::IntegerVector Separate32Bit( SSE::IntegerVector x, uint32_t distance )
        {
            // x = ---- ---- ---- ---- fedc ba98 7654 3210
            // x = ---- ---- fedc ba98 ---- ---- 7654 3210
            // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
            // x = --fe --dc --ba --98 --76 --54 --32 --10
            // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
            using namespace SSE;
            x = And(                        x,                                 SetAll( patterns_32bit[distance - 1][0] ) );
            x = And( Or( x, ShiftLeft64Bit( x,  shifts_32bit[distance - 1][0] ) ), SetAll( patterns_32bit[distance - 1][1] ) );
            x = And( Or( x, ShiftLeft64Bit( x,  shifts_32bit[distance - 1][1] ) ), SetAll( patterns_32bit[distance - 1][2] ) );
            x = And( Or( x, ShiftLeft64Bit( x,  shifts_32bit[distance - 1][2] ) ), SetAll( patterns_32bit[distance - 1][3] ) );
            x = And( Or( x, ShiftLeft64Bit( x,  shifts_32bit[distance - 1][3] ) ), SetAll( patterns_32bit[distance - 1][4] ) );
            return x;
        }


        uint64_t Separate( uint64_t x, uint64_t distance )
        {
            x &= patterns_64bit[distance - 1][0];                      // x = ---- ---- ---- ---- ---- ---- ---- ---- vuts rqpo nmlk jihg fedc ba98 7654 3210
            x = ( x ^ ( x << shifts_64bit[distance - 1][0] ) ) & patterns_64bit[distance - 1][1]; // x = ---- ---- ---- ---- vuts rqpo nmlk jihg ---- ---- ---- ---- fedc ba98 7654 3210
            x = ( x ^ ( x << shifts_64bit[distance - 1][1] ) ) & patterns_64bit[distance - 1][2]; // x = ---- ---- fedc ba98 ---- ---- 7654 3210 ---- ---- fedc ba98 ---- ---- 7654 3210 // numbers aren't right, but you get the idea
            x = ( x ^ ( x << shifts_64bit[distance - 1][2] ) ) & patterns_64bit[distance - 1][3]; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210 ---- fedc ---- ba98 ---- 7654 ---- 3210
            x = ( x ^ ( x << shifts_64bit[distance - 1][3] ) ) & patterns_64bit[distance - 1][4]; // x = --fe --dc --ba --98 --76 --54 --32 --10 --fe --dc --ba --98 --76 --54 --32 --10
            x = ( x ^ ( x << shifts_64bit[distance - 1][4] ) ) & patterns_64bit[distance - 1][5]; // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0 -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
            return x;
        }


        // processes two elements at the same time
        // assumes the IntegerVector contains two 64 bit numbers
        SSE::IntegerVector VECTOR_CALL Separate64Bit( SSE::IntegerVector x, uint64_t distance )
        {
            // x = ---- ---- ---- ---- ---- ---- ---- ---- vuts rqpo nmlk jihg fedc ba98 7654 3210
            // x = ---- ---- ---- ---- vuts rqpo nmlk jihg ---- ---- ---- ---- fedc ba98 7654 3210
            // x = ---- ---- fedc ba98 ---- ---- 7654 3210 ---- ---- fedc ba98 ---- ---- 7654 3210 // numbers aren't right, but you get the idea
            // x = ---- fedc ---- ba98 ---- 7654 ---- 3210 ---- fedc ---- ba98 ---- 7654 ---- 3210
            // x = --fe --dc --ba --98 --76 --54 --32 --10 --fe --dc --ba --98 --76 --54 --32 --10
            // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0 -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
            using namespace SSE;
            x = And(                x,                 SetAll( patterns_64bit[distance - 1][0] ) );
            x = And( Or( x, ShiftLeft64Bit( x, shifts_64bit[distance - 1][0]) ), SetAll( patterns_64bit[distance - 1][1] ) );
            x = And( Or( x, ShiftLeft64Bit( x, shifts_64bit[distance - 1][1]) ), SetAll( patterns_64bit[distance - 1][2] ) );
            x = And( Or( x, ShiftLeft64Bit( x, shifts_64bit[distance - 1][2]) ), SetAll( patterns_64bit[distance - 1][3] ) );
            x = And( Or( x, ShiftLeft64Bit( x, shifts_64bit[distance - 1][3]) ), SetAll( patterns_64bit[distance - 1][4] ) );
            x = And( Or( x, ShiftLeft64Bit( x, shifts_64bit[distance - 1][4]) ), SetAll( patterns_64bit[distance - 1][5] ) );
            return x;
        }

        uint32_t Compact(uint32_t x, uint32_t distance)
        {
          x &= patterns_32bit[distance - 1][4];                  // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
          x = (x | (x >>  shifts_32bit[distance - 1][3] )) & patterns_32bit[distance - 1][3]; // x = --fe --dc --ba --98 --76 --54 --32 --10
          x = (x | (x >>  shifts_32bit[distance - 1][2] )) & patterns_32bit[distance - 1][2]; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
          x = (x | (x >>  shifts_32bit[distance - 1][1] )) & patterns_32bit[distance - 1][1]; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
          x = (x | (x >>  shifts_32bit[distance - 1][0] )) & patterns_32bit[distance - 1][0]; // x = ---- ---- ---- ---- fedc ba98 7654 3210
          return x;
        }


        uint64_t Compact(uint64_t x, uint64_t distance)
        {
          x &= patterns_64bit[distance - 1][5];                  // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
          x = (x ^ (x >> shifts_64bit[distance - 1][4])) & patterns_64bit[distance - 1][4]; // x = --fe --dc --ba --98 --76 --54 --32 --10
          x = (x ^ (x >> shifts_64bit[distance - 1][3])) & patterns_64bit[distance - 1][3]; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
          x = (x ^ (x >> shifts_64bit[distance - 1][2])) & patterns_64bit[distance - 1][2]; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
          x = (x ^ (x >> shifts_64bit[distance - 1][1])) & patterns_64bit[distance - 1][1]; // x = ---- ---- ---- ---- fedc ba98 7654 3210
          x = (x ^ (x >> shifts_64bit[distance - 1][0])) & patterns_64bit[distance - 1][0]; // x = ---- ---- ---- ---- fedc ba98 7654 3210
          return x;
        }


        uint32_t VECTOR_CALL MortonIndex32BitFrom2x16Bit(SSE::IntegerVector values)
        {
            auto xy = Separate32Bit(values, 1);
            uint32_t xy_array[4];
            SSE::Store(xy, xy_array);
            auto result = xy_array[0] | (xy_array[1] << 1);
            return result;
        }


        uint64_t VECTOR_CALL MortonIndex64BitFrom2x32Bit(SSE::IntegerVector values)
        {
            auto xy = Separate64Bit(SSE::UnsignedInteger64FromInteger32(values), 1);
            uint64_t xy_array[2];
            SSE::Store(xy, xy_array);
            auto result = xy_array[0] | (xy_array[1] << 1);
            return result;
        }

        // uses only the first 21 bits of each number
        uint64_t VECTOR_CALL MortonIndex64BitFrom3x32Bit(SSE::IntegerVector values)
        {
            auto xy = Separate64Bit(SSE::UnsignedInteger64FromInteger32(values), 2);
            auto z = Separate( uint64_t(SSE::Extract32BitUnsigned<2>(values)), 2 );
            //auto zw = Separate64Bit(SSE::UnsignedInteger64FromInteger32(SSE::Swizzle32Bit<2,3>(values)), 2);
            //xy = SSE::Or(xy, SSE::ShiftLeft64Bit(zw, 2));
            uint64_t xy_array[2];
            SSE::Store(xy, xy_array);
            auto result = xy_array[0] | (xy_array[1] << 1) | (z << 2);
            return result;
        }


        MinMax<SSE::Float32Vector> VECTOR_CALL MinMaxPerComponent(Range<Math::Float3 const *> points)
        {
            using namespace SSE;
            assert(!IsEmpty(points));
            auto first = SSEFromFloat3(First(points));
            MinMax<Float32Vector> minmax = {first, first};
            PopFirst(points);
            for(auto i = 0u; i < Size(points); ++i)
            {
                auto current = SSEFromFloat3(points[i]);
                minmax.max = Max(current, minmax.max);
                minmax.min = Min(current, minmax.min);
            }
            return minmax;
        }
    }


    uint64_t MortonIndex64Bit(Math::Unsigned3 const & value)
    {
        // we also load the non-existing 4th element, but it shouldn't be used anyway, so it's ok.
        return MortonIndex64BitFrom3x32Bit(SSE::Load(begin(value)));
    }


    uint64_t MortonIndex64Bit(Math::Unsigned2 const & value)
    {
        return MortonIndex64BitFrom2x32Bit(SSE::SSEFromUnsigned2(value));
    }


    uint32_t MortonIndex32Bit(Math::Unsigned2 const & value)
    {
        return Separate(value.x, 1) | (Separate(value.y, 1) << 1);
        //return MortonIndex32BitFrom2x16Bit(SSE::SSEFromUnsigned2(value));
    }


    Math::Unsigned2 Index2DFromMorton(uint32_t index)
    {
        return {Compact(index, 1), Compact(index >> 1, 1)};
    }


    namespace
    {
        auto Increase(uint32_t index, uint32_t mask, int32_t amount)
        {
            auto a_part = index & mask;
            auto b_part = index & ~mask;
            // increment
            a_part += amount;
            // carry the numbers over the gaps
            a_part += ~mask;
            // clear the gaps
            a_part &= mask;
            return b_part + a_part;
        }
    }


    uint32_t IncreaseX2D(uint32_t index, int32_t amount)
    {
        auto x_mask = patterns_32bit[0][4];
        return Increase(index, x_mask, amount);
    }


    uint32_t IncreaseY2D(uint32_t index, int32_t amount)
    {
        auto y_mask = patterns_32bit[0][4] << 1;
        return Increase(index, y_mask, amount);
    }


    void ComputeMortonOrder(Range<Math::Float3 const *> points, MinMax<Float3> & minmax_out, Range<uint64_t *> morton_codes)
    {
        assert(Size(points) == Size(morton_codes));

        auto minmax = MinMaxPerComponent(points);
        minmax_out = {SSE::Float3FromSSE(minmax.min), SSE::Float3FromSSE(minmax.max)};
        auto scale = SSE::Divide(SSE::SetAll(float((1 << 21) - 1)), SSE::Subtract(minmax.max, minmax.min));

        for( auto i = 0u; i < Size(points); ++i )
        {
            // first scale the point such that we make maximum use of the morton space
            auto fpoint = SSE::SSEFromFloat3(points[i]);
            fpoint = SSE::Subtract(fpoint, minmax.min);
            fpoint = SSE::Multiply(scale, fpoint);

            auto ipoint = SSE::SignedInteger32FromFloat32(fpoint);
            morton_codes[i] = MortonIndex64BitFrom3x32Bit(ipoint);
        }

    }


    std::string ToString2D(Range<uint32_t*> morton_data)
    {
        return ToString2D(Range<uint32_t const*>(morton_data));
    }

    std::string ToString2D(Range<uint32_t const*> morton_data)
    {
        std::stringstream stream;
        auto dimensions = uint32_t(Sqrt(float(Size(morton_data))));
        for( auto y = 0u; y < dimensions; ++y )
        {
            for( auto x = 0u; x < dimensions; ++x )
            {
                auto index = MortonIndex32Bit({x, y});
                stream << morton_data[index];
                if(x + 1 < dimensions)
                {
                    stream << ", ";
                }
            }
            stream << ";" << std::endl;
        }
        return stream.str();
    }
}