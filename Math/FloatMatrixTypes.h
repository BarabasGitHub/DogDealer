#pragma once

#include "ForwardDeclarations.h"
#include "FloatTypes.h"
#include "Identity.h"

namespace Math
{
    struct Float2x2
    {
        Float2 row[2];
        Float2x2() = default;
        constexpr Float2x2( Identity );
        explicit constexpr Float2x2( float s );
        constexpr Float2x2( float m00, float m01,
                  float m10, float m11 );
        constexpr Float2x2( Float2 row0, Float2 row1 );

        Float2 & operator[]( unsigned index );
        Float2 const & operator[]( unsigned index ) const;

        float& operator()( unsigned row, unsigned column );
        float operator()( unsigned row, unsigned column ) const;
    };


    struct Float3x3
    {
        Float3 row[3];

        Float3x3() = default;
        constexpr Float3x3( Identity );
        explicit constexpr Float3x3( float s );
        constexpr Float3x3( float m00, float m01, float m02,
                  float m10, float m11, float m12,
                  float m20, float m21, float m22 );
        constexpr Float3x3( Float3 row0, Float3 row1, Float3 row3 );

        Float3 & operator[]( unsigned index );
        Float3 const & operator[]( unsigned index ) const;

        float& operator()( unsigned row, unsigned column );
        float operator()( unsigned row, unsigned column ) const;

    };

    struct Float4x4
    {
        Float4 row[4];
        Float4x4() = default;
        constexpr Float4x4( Identity );
        explicit constexpr Float4x4( float s );
        constexpr Float4x4(
            float m00, float m01, float m02, float m03,
            float m10, float m11, float m12, float m13,
            float m20, float m21, float m22, float m23,
            float m30, float m31, float m32, float m33 );
        constexpr Float4x4( Float4 row0, Float4 row1, Float4 row2, Float4 row3 = Float4( 0, 0, 0, 1 ) );

        Float4 & operator[]( unsigned index );
        Float4 const & operator[]( unsigned index ) const;

        float& operator()( unsigned row, unsigned column );
        float operator()( unsigned row, unsigned column ) const;
    };


    inline auto  begin(Float2x2 & in) {return in.row;}
    inline auto  begin(Float2x2 const & in) {return in.row;}
    inline auto  end(Float2x2 & in) {return in.row + 2;}
    inline auto  end(Float2x2 const & in) {return in.row + 2;}

    inline auto  begin(Float3x3 & in) {return in.row;}
    inline auto  begin(Float3x3 const & in) {return in.row;}
    inline auto  end( Float3x3 & in )  { return in.row + 3; }
    inline auto  end(Float3x3 const & in) {return in.row + 3;}

    inline auto  begin(Float4x4 & in) {return in.row;}
    inline auto  begin(Float4x4 const & in) {return in.row;}
    inline auto  end(Float4x4 & in) {return in.row + 4;}
    inline auto  end(Float4x4 const & in) {return in.row + 4;}


    // implementation of constructors and accessors

    inline constexpr Float2x2::Float2x2( Float2 row0, Float2 row1 ) : row{row0, row1} {}
    inline constexpr Float2x2::Float2x2( Identity ) : Float2x2( { 1, 0 }, { 0, 1 } ) {}
    inline constexpr Float2x2::Float2x2( float s ) : Float2x2( s, s ) {};
    inline constexpr Float2x2::Float2x2( float m00, float m01, float m10, float m11 ) : Float2x2( { m00, m01 }, { m10, m11 }) {}

    inline constexpr Float3x3::Float3x3( Float3 row0, Float3 row1, Float3 row2 ) : row{row0, row1, row2} {}
    inline constexpr Float3x3::Float3x3( Identity ) : Float3x3( { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } ) {}
    inline constexpr Float3x3::Float3x3( float s ) : Float3x3( s, s, s ) {};
    inline constexpr Float3x3::Float3x3( float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22 ) : Float3x3( { m00, m01, m02 }, { m10, m11, m12 }, { m20, m21, m22 }  ) {}

    inline constexpr Float4x4::Float4x4( Float4 row0, Float4 row1, Float4 row2, Float4 row3 ) : row{row0, row1, row2, row3} {}
    inline constexpr Float4x4::Float4x4( Identity ) : Float4x4( { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } ) {}
    inline constexpr Float4x4::Float4x4( float s ) : Float4x4( s, s, s, s ) {};
    inline constexpr Float4x4::Float4x4( float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33 ) : Float4x4( { m00, m01, m02, m03 }, { m10, m11, m12, m13 }, { m20, m21, m22, m23 }, { m30, m31, m32, m33 }) {}

    inline Float2 & Float2x2::operator[]( unsigned index ){ assert( index < 2 ); return this->row[index]; }
    inline Float2 const & Float2x2::operator[]( unsigned index ) const { assert( index < 2 ); return this->row[index]; }
    inline float & Float2x2::operator()(unsigned row_index, unsigned column) { return (*this)[row_index][column]; }
    inline float Float2x2::operator()(unsigned row_index, unsigned column) const { return (*this)[row_index][column]; }

    inline Float3 & Float3x3::operator[]( unsigned index ){ assert( index < 3 ); return this->row[index]; }
    inline Float3 const & Float3x3::operator[]( unsigned index ) const { assert( index < 3 ); return this->row[index]; }
    inline float & Float3x3::operator()(unsigned row_index, unsigned column) { return (*this)[row_index][column]; }
    inline float Float3x3::operator()(unsigned row_index, unsigned column) const { return (*this)[row_index][column]; }

    inline Float4 & Float4x4::operator[]( unsigned index ){ assert( index < 4 ); return this->row[index]; }
    inline Float4 const & Float4x4::operator[]( unsigned index ) const { assert( index < 4 ); return this->row[index]; }
    inline float & Float4x4::operator()(unsigned row_index, unsigned column) { return (*this)[row_index][column]; }
    inline float Float4x4::operator()(unsigned row_index, unsigned column) const { return (*this)[row_index][column]; }

}
