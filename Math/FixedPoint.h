#pragma once

#include <cstdint>
#include <type_traits>
#pragma warning(push)
#pragma warning(disable: 4201)

namespace Math
{
    template<typename UnderlyingType = int64_t, size_t NumberOfFractionBits = 32>
    struct FixedPointTemplate
    {
        static auto const c_fraction_bits = NumberOfFractionBits;
        union
        {
            UnderlyingType full;
            // order of fraction and integer part is important here, because we use the full XX bit integer to do the computations
            // this only works correctly if the fractional part comes first
            struct
            {
                std::make_unsigned_t<UnderlyingType> fraction : NumberOfFractionBits;
                UnderlyingType integer : (sizeof(UnderlyingType) * 8) - NumberOfFractionBits;
            };
        };

        // FixedPoint::fraction == actual_fraction * c_fraction_factor
        static auto const c_fraction_factor = uint64_t(1) << NumberOfFractionBits;
    };

    typedef FixedPointTemplate<> FixedPoint;
}

#pragma warning(pop)
