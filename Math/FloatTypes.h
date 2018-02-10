#pragma once

#include "ForwardDeclarations.h"
#include "Identity.h"
#include <cassert>

namespace Math
{
    struct Float2
    {
        float x, y;

        Float2() = default;
        constexpr Float2( float x, float y );
        constexpr Float2( float s );

        decltype(auto) operator[]( unsigned index );
        constexpr decltype(auto) operator[]( unsigned index ) const;
    };

    struct Float3
    {
        float x, y, z;

        Float3() = default;
        constexpr Float3( float x, float y, float z );
        constexpr Float3( float s );

        decltype(auto) operator[]( unsigned index );
        constexpr decltype(auto) operator[]( unsigned index ) const;

    };

    struct Float4
    {
        float x, y, z, w;

        Float4() = default;
        constexpr Float4( float x, float y, float z, float w );
        constexpr Float4( float s );
        constexpr Float4( Float3 v, float w );

        decltype(auto) operator[]( unsigned index );
        constexpr decltype(auto) operator[]( unsigned index ) const;

    };

    struct Quaternion
    {
        float x, y, z, w;

        Quaternion() = default;
        constexpr Quaternion( Float3 float3, float w );
        constexpr Quaternion( float x, float y, float z, float w );
        constexpr Quaternion(Identity);

        constexpr explicit Quaternion( Float4 float4 );
        constexpr explicit operator Float4() const;

        decltype(auto) operator[]( unsigned index );
        constexpr decltype(auto) operator[]( unsigned index ) const;

    };

    inline float* begin( Float2 & in ) { return &in.x; }
    inline float const * begin( Float2 const & in ) { return &in.x; }
    inline float* end( Float2 & in ) { return begin(in)+ 2; }
    inline float const * end( Float2 const & in ) { return begin(in) + 2; }

    inline float* begin( Float3 & in ) { return &in.x; }
    inline float const * begin( Float3 const & in ) { return &in.x; }
    inline float* end( Float3 & in ) { return begin(in) + 3; }
    inline float const * end( Float3 const & in ) { return begin(in) + 3; }

    inline float* begin( Float4 & in ) { return &in.x; }
    inline float const * begin( Float4 const & in ) { return &in.x; }
    inline float* end( Float4 & in ) { return begin(in) + 4; }
    inline float const * end( Float4 const & in ) { return begin(in) + 4; }

    // implementation of constructors and accessors

    inline constexpr Float2::Float2( float x, float y ) : x( x ), y( y ) {}
    inline constexpr Float2::Float2( float s ) : Float2( s, s ) {}

    inline constexpr Float3::Float3( float x, float y, float z ) : x( x ), y( y ), z( z ) {};
    inline constexpr Float3::Float3( float s ) : Float3( s, s, s ) {}

    inline constexpr Float4::Float4( float x, float y, float z, float w ) : x( x ), y( y ), z( z ), w( w ) {}
    inline constexpr Float4::Float4( float s ) : Float4( s, s, s, s ) {}
    inline constexpr Float4::Float4( Float3 v, float w ): Float4(v.x, v.y, v.z, w) {}

    inline constexpr Quaternion::Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    inline constexpr Quaternion::Quaternion( Float3 float3, float w ) : Quaternion( float3.x, float3.y, float3.z, w ) {}
    inline constexpr Quaternion::Quaternion( Identity ) : Quaternion( Float3( 0 ), 1 ) {}

    inline constexpr Quaternion::Quaternion( Float4 float4 ) : Quaternion( float4.x, float4.y, float4.z, float4.w ) {}
    inline constexpr Quaternion::operator Float4() const { return{ x, y, z, w }; }


#include "AccessOperatorsMacros.h"

    AccessOperators(Float2, 2);
    AccessOperators(Float3, 3);
    AccessOperators(Float4, 4);
    AccessOperators(Quaternion, 4);

#include "UndefAccessOperatorsMacros.h"

}
