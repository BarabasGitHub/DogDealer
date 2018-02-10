#pragma once

#include "FloatTypes.h"
#include "IntegerTypes.h"
#include "FixedPoint.h"

namespace Math
{
    constexpr Int2 Int2FromUnsigned2( Unsigned2 const & in );
    constexpr Int3 Int3FromUnsigned3( Unsigned3 const & in );
    constexpr Int4 Int4FromUnsigned4( Unsigned4 const & in );

    constexpr Int2 Int2FromFloat2( Float2 const & in );
    constexpr Int3 Int3FromFloat3( Float3 const & in );
    constexpr Int4 Int4FromFloat4( Float4 const & in );

    constexpr Unsigned2 Unsigned2FromInt2( Int2 const & in );
    constexpr Unsigned3 Unsigned3FromInt3( Int3 const & in );
    constexpr Unsigned4 Unsigned4FromInt4( Int4 const & in );

    constexpr Unsigned2 Unsigned2FromFloat2( Float2 const & in );
    constexpr Unsigned3 Unsigned3FromFloat3( Float3 const & in );
    constexpr Unsigned4 Unsigned4FromFloat4( Float4 const & in );

    constexpr Float2 Float2FromUnsigned2( Unsigned2 const & in );
    constexpr Float3 Float3FromUnsigned3( Unsigned3 const & in );
    constexpr Float4 Float4FromUnsigned4( Unsigned4 const & in );

    constexpr Float2 Float2FromInt2( Int2 const & in );
    constexpr Float3 Float3FromInt3( Int3 const & in );
    constexpr Float4 Float4FromInt4( Int4 const & in );

    FixedPoint FixedPointFromFloat(float f);
    FixedPoint FixedPointFromDouble(double f);
    float FloatFromFixedPoint(FixedPoint f);
    double DoubleFromFixedPoint(FixedPoint f);
}


// implementations

namespace Math
{
    inline Int2 constexpr Int2FromUnsigned2( Unsigned2 const & in )
    {
        return Int2{ int(in[0]), int(in[1]) };
    }

    inline Int3 constexpr Int3FromUnsigned3( Unsigned3 const & in )
    {
        return Int3{ int(in.x), int(in.y), int(in.z) };
    }

    inline Int4 constexpr Int4FromUnsigned4( Unsigned4 const & in )
    {
        return Int4{ int(in.x), int(in.y), int(in.z), int(in.w) };
    }


    inline Int2 constexpr Int2FromFloat2( Float2 const & in )
    {
        return Int2{ int(in[0]), int(in[1]) };
    }

    inline Int3 constexpr Int3FromFloat3( Float3 const & in )
    {
        return Int3{ int(in.x), int(in.y), int(in.z) };
    }

    inline constexpr Int4 Int4FromFloat4( Float4 const & in )
    {
        return Int4{ int(in.x), int(in.y), int(in.z), int(in.w) };
    }

    inline constexpr Unsigned2 Unsigned2FromInt2( Int2 const & in )
    {
        return Unsigned2{ unsigned(in[0]), unsigned(in[1]) };
    }

    inline constexpr Unsigned3 Unsigned3FromInt3( Int3 const & in )
    {
        return Unsigned3{ unsigned(in.x), unsigned(in.y), unsigned(in.z) };
    }

    inline constexpr Unsigned4 Unsigned4FromInt4( Int4 const & in )
    {
        return Unsigned4{ unsigned(in.x), unsigned(in.y), unsigned(in.z), unsigned(in.w) };
    }

    inline constexpr Unsigned2 Unsigned2FromFloat2( Float2 const & in )
    {
        return Unsigned2{ unsigned(in[0]), unsigned(in[1]) };
    }

    inline constexpr Unsigned3 Unsigned3FromFloat3( Float3 const & in )
    {
        return Unsigned3{ unsigned(in.x), unsigned(in.y), unsigned(in.z) };
    }

    inline constexpr Unsigned4 Unsigned4FromFloat4( Float4 const & in )
    {
        return Unsigned4{ unsigned(in.x), unsigned(in.y), unsigned(in.z), unsigned(in.w) };
    }

    inline constexpr Float2 Float2FromUnsigned2( Unsigned2 const & in )
    {
        return Float2{ float(in[0]), float(in[1]) };
    }

    inline constexpr Float3 Float3FromUnsigned3( Unsigned3 const & in )
    {
        return Float3{ float(in[0]), float(in[1]), float(in[2]) };
    }

    inline constexpr Float4 Float4FromUnsigned4( Unsigned4 const & in )
    {
        return Float4{ float(in[0]), float(in[1]), float(in[2]), float(in[3]) };
    }

    inline constexpr Float2 Float2FromInt2( Int2 const & in )
    {
        return Float2{ float(in[0]), float(in[1]) };
    }

    inline constexpr Float3 Float3FromInt3( Int3 const & in )
    {
        return Float3{ float(in[0]), float(in[1]), float(in[2]) };
    }

    inline constexpr Float4 Float4FromInt4( Int4 const & in )
    {
        return Float4{ float(in[0]), float(in[1]), float(in[2]), float(in[3]) };
    }



    inline FixedPoint Math::FixedPointFromFloat(float f)
    {
        FixedPoint fp;
        // float integer_part = std::floor(f);
        // fp.integer = static_cast<decltype(fp.integer)>(integer_part);
        // float fraction_part = f - integer_part;
        // fp.fraction = static_cast<decltype(fp.fraction)>(fraction_part * float(FixedPoint::c_fraction_factor));
        fp.full = static_cast<decltype(fp.full)>(f * float(FixedPoint::c_fraction_factor));
        return fp;
    }


    inline FixedPoint Math::FixedPointFromDouble(double f)
    {
        FixedPoint fp;
        // double integer_part = std::floor(f);
        // fp.integer = static_cast<decltype(fp.integer)>(integer_part);
        // double fraction_part = f - integer_part;
        // fp.fraction = static_cast<decltype(fp.fraction)>(fraction_part * double(FixedPoint::c_fraction_factor));
        fp.full = static_cast<decltype(fp.full)>(f * double(FixedPoint::c_fraction_factor));
        return fp;
    }


    inline float Math::FloatFromFixedPoint(FixedPoint f)
    {
        // float integer_part = float(f.integer);
        // float fraction_part = float(f.fraction) * (1.0f / float(FixedPoint::c_fraction_factor));
        // return integer_part + fraction_part;
        return float(f.full) / float(FixedPoint::c_fraction_factor);
    }


    inline double Math::DoubleFromFixedPoint(FixedPoint f)
    {
        // double integer_part = double(f.integer);
        // double fraction_part = double(f.fraction) * (1.0 / double(FixedPoint::c_fraction_factor));
        // return integer_part + fraction_part;
        return double(f.full) / double(FixedPoint::c_fraction_factor);
    }
}
