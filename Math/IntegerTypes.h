#pragma once

#include <cassert>

namespace Math
{
    struct Int2
    {
        int x, y;

        Int2() = default;
        constexpr Int2( int x, int y );
        constexpr Int2( int s );

        decltype(auto) operator[](unsigned index);
        decltype(auto) constexpr operator[](unsigned index) const;
    };


    struct Int3
    {
        int x, y, z;

        Int3() = default;
        constexpr Int3( int x, int y, int z );
        constexpr Int3( int s );

        decltype(auto) operator[](unsigned index);
        decltype(auto) constexpr operator[](unsigned index) const;
    };


    struct Int4
    {
        int x, y, z, w;

        Int4() = default;
        constexpr Int4( int x, int y, int z, int w );
        constexpr Int4( int s );

        decltype(auto) operator[]( unsigned index );
        decltype(auto) constexpr operator[]( unsigned index ) const;
    };

    struct Unsigned2
    {
        unsigned x, y;

        Unsigned2() = default;
        constexpr Unsigned2( unsigned x, unsigned y );
        constexpr Unsigned2( unsigned s );

        decltype(auto) operator[]( unsigned index );
        decltype(auto) constexpr operator[]( unsigned index ) const;
    };


    struct Unsigned3
    {
        unsigned x, y, z;

        Unsigned3() = default;
        constexpr Unsigned3( unsigned x, unsigned y, unsigned z );
        constexpr Unsigned3( unsigned s );

        decltype(auto) operator[]( unsigned index );
        decltype(auto) constexpr operator[]( unsigned index ) const;
    };


    struct Unsigned4
    {
        unsigned x, y, z, w;

        Unsigned4() = default;
        constexpr Unsigned4( unsigned x, unsigned y, unsigned z, unsigned w );
        constexpr Unsigned4( unsigned s );

        decltype(auto) operator[](unsigned index);
        decltype(auto) constexpr operator[](unsigned index) const;
    };

    inline int* begin(Int2 & in) {return &in.x;}
    inline int const * begin(Int2 const & in) {return &in.x;}
    inline int* end(Int2 & in) {return &in.y + 1;}
    inline int const * end(Int2 const & in) {return &in.y + 1;}

    inline int* begin(Int3 & in) {return &in.x;}
    inline int const * begin(Int3 const & in) {return &in.x;}
    inline int* end(Int3 & in) {return &in.z + 1;}
    inline int const * end(Int3 const & in) {return &in.z + 1;}

    inline int* begin(Int4 & in) {return &in.x;}
    inline int const * begin(Int4 const & in) {return &in.x;}
    inline int* end(Int4 & in) {return &in.w + 1;}
    inline int const * end(Int4 const & in) {return &in.w + 1;}

    inline unsigned* begin(Unsigned2 & in) {return &in.x;}
    inline unsigned const * begin(Unsigned2 const & in) {return &in.x;}
    inline unsigned* end(Unsigned2 & in) {return &in.y + 1;}
    inline unsigned const * end(Unsigned2 const & in) {return &in.y + 1;}

    inline unsigned* begin(Unsigned3 & in) {return &in.x;}
    inline unsigned const * begin(Unsigned3 const & in) {return &in.x;}
    inline unsigned* end(Unsigned3 & in) {return &in.z + 1;}
    inline unsigned const * end(Unsigned3 const & in) {return &in.z + 1;}

    inline unsigned* begin(Unsigned4 & in) {return &in.x;}
    inline unsigned const * begin(Unsigned4 const & in) {return &in.x;}
    inline unsigned* end(Unsigned4 & in) {return &in.w + 1;}
    inline unsigned const * end(Unsigned4 const & in) {return &in.w + 1;}


    // implementation of constructors and accessors

    inline constexpr Int2::Int2( int x, int y ) : x( x ), y( y ) {}
    inline constexpr Int2::Int2( int s ) : x( s ), y( s ) {}
    
    inline constexpr Int3::Int3( int x, int y, int z ) : x( x ), y( y ), z( z ) {};
    inline constexpr Int3::Int3( int s ) : x( s ), y( s ), z( s ) {}

    inline constexpr Int4::Int4( int x, int y, int z, int w ) : x( x ), y( y ), z( z ), w( w ) {}
    inline constexpr Int4::Int4( int s ) : x( s ), y( s ), z( s ), w( s ) {}

    inline constexpr Unsigned2::Unsigned2( unsigned x, unsigned y ) : x( x ), y( y ) {}
    inline constexpr Unsigned2::Unsigned2( unsigned s ) : x( s ), y( s ) {}

    inline constexpr Unsigned3::Unsigned3( unsigned x, unsigned y, unsigned z ) : x( x ), y( y ), z( z ) {};
    inline constexpr Unsigned3::Unsigned3( unsigned s ) : x( s ), y( s ), z( s ) {}

    inline constexpr Unsigned4::Unsigned4( unsigned x, unsigned y, unsigned z, unsigned w ) : x( x ), y( y ), z( z ), w( w ) {}
    inline constexpr Unsigned4::Unsigned4( unsigned s ) : x( s ), y( s ), z( s ), w( s ) {}


#include "AccessOperatorsMacros.h"

    AccessOperators(Int2, 2);
    AccessOperators(Int3, 3);
    AccessOperators(Int4, 4);
    AccessOperators(Unsigned2, 2);
    AccessOperators(Unsigned3, 3);
    AccessOperators(Unsigned4, 4);

#include "UndefAccessOperatorsMacros.h"

}
