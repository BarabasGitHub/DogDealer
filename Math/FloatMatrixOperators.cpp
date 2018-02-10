#pragma once
#include "FloatOperators.h"
#include "FloatTypes.h"
#include "FloatMatrixOperators.h"
#include "FloatMatrixTypes.h"
#include "MathFunctions.h"

#include "SSEMatrixFunctions.h"
#include "SSEFloatFunctions.h"
#include "SSEMathConversions.h"

namespace Math
{
    namespace
    {
        inline Float2 GetHalfColumn(Float4x4 const & me, unsigned half, unsigned index)
        {
            assert( half < 2);
            assert( index < 4);
            half *= 2;
            return Float2( me( half, index ), me( half + 1, index ) );
        }
    }


    float Determinant(Float3x3 const & in)
    {
        Float3 cofactors;
        for( auto column = 0u; column < 3; ++column )
        {
            cofactors[column] = CoFactor(in, 0, column );
        }
        return Dot( in.row[0], cofactors );
    }


    Float4x4 Adjugate(Float4x4 const & in)
    {
        // Method of calculation taken from http://www.geometrictools.com/Documentation/LaplaceExpansionTheorem.pdf
        Float2x2 submatrix( GetHalfColumn(in, 0, 0 ), GetHalfColumn(in, 0, 1 ) );
        auto s0 = Determinant(submatrix);

        submatrix.row[1] = GetHalfColumn(in, 0, 2 );
        auto s1 = Determinant(submatrix);

        submatrix.row[1] = GetHalfColumn(in, 0, 3 );
        auto s2 = Determinant(submatrix);

        submatrix.row[0] = GetHalfColumn(in, 0, 2 );
        auto s5 = Determinant(submatrix);

        submatrix.row[0] = GetHalfColumn(in, 0, 1 );
        auto s4 = Determinant(submatrix);

        submatrix.row[1] = GetHalfColumn(in, 0, 2 );
        auto s3 = Determinant(submatrix);

        submatrix = Float2x2( GetHalfColumn(in, 1, 2 ), GetHalfColumn(in, 1, 3 ) );
        auto c5 = Determinant(submatrix);

        submatrix.row[0] = GetHalfColumn(in, 1, 1 );
        auto c4 = Determinant(submatrix);

        submatrix.row[1] = GetHalfColumn(in, 1, 2 );
        auto c3 = Determinant(submatrix);

        submatrix = Float2x2( GetHalfColumn(in, 1, 0 ), GetHalfColumn(in, 1, 1 ) );
        auto c0 = Determinant(submatrix);

        submatrix.row[1] = GetHalfColumn(in, 1, 2 );
        auto c1 = Determinant(submatrix);

        submatrix.row[1] = GetHalfColumn(in, 1, 3 );
        auto c2 = Determinant(submatrix);

        return Float4x4(
            in( 1, 1 ) * c5 - in( 1, 2 ) * c4 + in( 1, 3 ) * c3, -in( 0, 1 ) * c5 + in( 0, 2 ) * c4 - in( 0, 3 ) * c3, in( 3, 1 ) * s5 - in( 3, 2 ) * s4 + in( 3, 3 ) * s3, -in( 2, 1 ) * s5 + in( 2, 2 ) * s4 - in( 2, 3 ) * s3,
            -in( 1, 0 ) * c5 + in( 1, 2 ) * c2 - in( 1, 3 ) * c1, in( 0, 0 ) * c5 - in( 0, 2 ) * c2 + in( 0, 3 ) * c1, -in( 3, 0 ) * s5 + in( 3, 2 ) * s2 - in( 3, 3 ) * s1, in( 2, 0 ) * s5 - in( 2, 2 ) * s2 + in( 2, 3 ) * s1,
            in( 1, 0 ) * c4 - in( 1, 1 ) * c2 + in( 1, 3 ) * c0, -in( 0, 0 ) * c4 + in( 0, 1 ) * c2 - in( 0, 3 ) * c0, in( 3, 0 ) * s4 - in( 3, 1 ) * s2 + in( 3, 3 ) * s0, -in( 2, 0 ) * s4 + in( 2, 1 ) * s2 - in( 2, 3 ) * s0,
            -in( 1, 0 ) * c3 + in( 1, 1 ) * c1 - in( 1, 2 ) * c0, in( 0, 0 ) * c3 - in( 0, 1 ) * c1 + in( 0, 2 ) * c0, -in( 3, 0 ) * s3 + in( 3, 1 ) * s1 - in( 3, 2 ) * s0, in( 2, 0 ) * s3 - in( 2, 1 ) * s1 + in( 2, 2 ) * s0
            );
    }


    float Determinant(Float4x4 const & in)
    {
        Float4 cofactors;
        for( auto column = 0u; column < 4u; ++column )
        {
            cofactors[column] = CoFactor(in, 0, column );
        }
        return Dot( in.row[0], cofactors );
    }


    Float2 operator* ( Float2x2 float2x2, Float2 float2 )
    {
        return{ Dot( float2x2.row[0], float2 ), Dot( float2x2.row[1], float2 ) };
    }


    Float3 operator* ( Float3x3 float3x3, Float3 float3 )
    {
        return{ Dot( float3x3.row[0], float3 ), Dot( float3x3.row[1], float3 ), Dot( float3x3.row[2], float3 ) };
    }


    Float4 operator* ( Float4x4 const & float4x4, Float4 const & float4 )
    {
        static_assert( sizeof( float4x4 ) == sizeof( SSE::FloatMatrix ), "Can't do the trick with reinterpret cast." );
        return SSE::Float4FromSSE( SSE::Multiply4D( reinterpret_cast<SSE::FloatMatrix const &>(float4x4), SSE::SSEFromFloat4( float4 ) ) );
        //{ Dot( float4x4.row[0], float4 ),
        //  Dot( float4x4.row[1], float4 ),
        //  Dot( float4x4.row[2], float4 ),
        //  Dot( float4x4.row[3], float4 ) };
    }


    float Minor( Float2x2 const & me, unsigned row, unsigned column )
    {
        // the bitwise exclusive or (^) flips 1 to 0 and 0 to 1;
        return me( row ^ 1, column ^ 1 );
    }


    float CoFactor( Float2x2 const & me, unsigned row, unsigned column )
    {
        float factor = ( ( row + column ) & 0x1 ) ? -1.0f : 1.0f;
        return factor * Minor( me, row, column );
    }


    Float2x2 CoFactorMatrix( Float2x2 const in )
    {
        return Float2x2( in.row[1].y, -in.row[1].x, -in.row[0].y, in.row[0].x );
    }


    Float2x2 Transpose( Float2x2 const in )
    {
        return Float2x2( GetColumn( in, 0 ), GetColumn( in, 1 ) );
    }


    Float2x2 Adjugate( Float2x2 const in )
    {
        return Float2x2( in.row[1].y, -in.row[0].y, -in.row[1].x, in.row[0].x );
    }


    float Determinant( Float2x2 const in )
    {
        using namespace SSE;
        //auto row01 = Set(in(0,0), in(0,1), in(1,0), in(1,1));
        static_assert( sizeof( in ) == sizeof( Float32Vector ), "Can't load a float2x2 matrix in one FLoat32vector" );
        auto row01 = LoadFloat32Vector( begin( in.row[0]) );
        auto row01_reversed = Swizzle<3,2,1,0>(row01);
        auto r = Multiply(row01, row01_reversed);
        r = HorizontalSubtract(r, r);
        return GetSingle(r);
        // return ( in.row[0].x * in.row[1].y ) - ( in.row[0].y * in.row[1].x );
    }


    Float2x2 Inverse( Float2x2 const in )
    {
        auto determinant = Determinant( in );
        //not sure how useful this assert is
        assert( determinant != 0 );
        return Adjugate( in ) / determinant;
    }

    // Assignment operators
    Float2x2& operator+= ( Float2x2 & me, const Float2x2 other )
    {
        me.row[0] += other.row[0];
        me.row[1] += other.row[1];
        return me;
    }


    Float2x2& operator-= ( Float2x2 & me, const Float2x2 other )
    {
        me.row[0] -= other.row[0];
        me.row[1] -= other.row[1];
        return me;
    }


    Float2x2& operator*= ( Float2x2 & me, Float2x2 const & other )
    {
        auto transposed = Transpose( other );
        me.row[0] = transposed * me.row[0];
        me.row[1] = transposed * me.row[1];
        return me;
    }


    Float2x2& operator/= ( Float2x2 & me, const Float2x2 other )
    {
        auto inverse = Inverse( other );
        me *= inverse;
        return me;
    }


    Float2x2& operator*= ( Float2x2 & me, float scalar )
    {
        me.row[0] *= scalar;
        me.row[1] *= scalar;
        return me;
    }


    Float2x2& operator/= ( Float2x2 & me, float scalar )
    {
        me.row[0] /= scalar;
        me.row[1] /= scalar;
        return me;
    }

    // Unary operators
    Float2x2 operator+ ( Float2x2 & me )
    {
        return me;
    }


    Float2x2 operator- ( Float2x2 & me )
    {
        return Float2x2( -me.row[0], -me.row[1] );
    }

    // Binary operators
    Float2x2 operator+ ( Float2x2 first, const Float2x2 second )
    {
        return first += second;
    }


    Float2x2 operator- ( Float2x2 first, const Float2x2 second )
    {
        return first -= second;
    }


    Float2x2 operator* ( Float2x2 first, const Float2x2 second )
    {
        return first *= second;
    }


    Float2x2 operator* ( Float2x2 first, const float scalar )
    {
        return first *= scalar;
    }


    Float2x2 operator/ ( Float2x2 first, const Float2x2 second )
    {
        return first /= second;
    }


    Float2x2 operator/ ( Float2x2 first, const float scalar )
    {
        return first /= scalar;
    }


    Float2x2 operator* ( float scalar, Float2x2 float2x2 )
    {
        return float2x2 *= scalar;
    }


    Float2x2 operator/ ( float scalar, Float2x2 float2x2 )
    {
        float2x2 = Inverse( float2x2 );
        return float2x2 *= scalar;
    }


    Float2& operator*= ( Float2& me, Float2x2 float2x2 )
    {
        me = Transpose( float2x2 ) * me;
        return me;
    }


    Float2 operator* ( Float2 float2, const Float2x2 float2x2 )
    {
        return float2 *= float2x2;
    }


    Float2x2 Minor( Float3x3 const & me, unsigned row, unsigned column )
    {
        assert( row < 3 );
        assert( column < 3 );
        Float2x2 float2x2(0);
        uint8_t minor_row = 0u;
        for( uint8_t major_row = 0u; major_row < 3u; ++major_row )
        {
            if( row != major_row )
            {
                uint8_t minor_column = 0u;
                for( uint8_t major_column = 0u; major_column < 3u; ++major_column )
                {
                    if( column != major_column )
                    {
                        float2x2( minor_row, minor_column ) = me( major_row, major_column );
                        ++minor_column;
                    }
                }
                ++minor_row;
            }
        }
        return float2x2;
    }


    float CoFactor( Float3x3 const & me, unsigned row, unsigned column )
    {
        float factor = ( ( row + column ) & 0x1 ) ? -1.0f : 1.0f;
        return factor * Determinant( Minor( me, row, column ) );
    }


    Float3x3 CoFactorMatrix( Float3x3 const & in )
    {
        Float3x3 float3x3;
        for( auto row = 0u; row < 3; ++row )
        {
            for( auto column = 0u; column < 3; ++column )
            {
                float3x3( row, column ) = CoFactor( in, row, column );
            }
        }
        return float3x3;
    }


    Float3x3 Transpose( Float3x3 const & in )
    {
        return Float3x3( GetColumn( in, 0 ), GetColumn( in, 1 ), GetColumn( in, 2 ) );
    }


    Float3x3 Adjugate( Float3x3 const & in )
    {
        return Transpose( CoFactorMatrix( in ) );
    }



    Float3x3 Inverse( Float3x3 const & in )
    {
        auto determinant = Determinant( in );
        //not sure how useful this assert is
        assert( determinant != 0 );
        return Adjugate( in ) / determinant;
    }

    // Unary operators
    Float3x3 operator+ ( Float3x3 const & me )
    {
        return me;
    }


    Float3x3 operator- ( Float3x3 const & me )
    {
        return Float3x3( -me.row[0], -me.row[1], -me.row[2] );
    }


    // Assignment operators
    Float3x3& operator+= ( Float3x3 & me, const Float3x3 other )
    {
        me.row[0] += other.row[0];
        me.row[1] += other.row[1];
        me.row[2] += other.row[2];
        return me;
    }


    Float3x3& operator-= ( Float3x3 & me, const Float3x3 other )
    {
        me.row[0] -= other.row[0];
        me.row[1] -= other.row[1];
        me.row[2] -= other.row[2];
        return me;
    }


    Float3x3& operator*= ( Float3x3 & me, Float3x3 const & other )
    {
        auto t = Transpose( other );
        me = MultiplyTransposed( me, t );
        return me;
    }


    Float3x3& operator/= ( Float3x3 & me, const Float3x3 & other )
    {
        auto inversed = Inverse( other );
        me *= inversed;
        return me;
    }


    Float3x3& operator*= ( Float3x3 & me, float scalar )
    {
        me.row[0] *= scalar;
        me.row[1] *= scalar;
        me.row[2] *= scalar;
        return me;
    }


    Float3x3& operator/= ( Float3x3 & me, float scalar )
    {
        me.row[0] /= scalar;
        me.row[1] /= scalar;
        me.row[2] /= scalar;
        return me;
    }

    // Binary operators
    Float3x3 operator+ ( Float3x3 first, const Float3x3 second )
    {
        return first += second;
    }


    Float3x3 operator- ( Float3x3 first, const Float3x3 second )
    {
        return first -= second;
    }


    Float3x3 operator* ( Float3x3 first, Float3x3 const & second )
    {
        auto const t = Transpose( second );
        return MultiplyTransposed( first, t );
    }


    Float3x3 operator* ( Float3x3 first, const float scalar )
    {
        return first *= scalar;
    }


    Float3x3 operator/ ( Float3x3 first, const Float3x3 second )
    {
        return first /= second;
    }


    Float3x3 operator/ ( Float3x3 first, const float scalar )
    {
        return first /= scalar;
    }


    Float3x3 operator* ( float scalar, Float3x3 float3x3 )
    {
        return float3x3 *= scalar;
    }


    Float3x3 operator/ ( float scalar, Float3x3 const & float3x3 )
    {
        return scalar * Inverse( float3x3 );
    }


    Float3& operator*= ( Float3& me, Float3x3 const & float3x3 )
    {
        me = Transpose( float3x3 ) * me;
        return me;
    }


    Float3 operator* ( Float3 float3, Float3x3 const & float3x3 )
    {
        return float3 *= float3x3;
    }


    Float3x3 MultiplyTransposed( Float3x3 a, Float3x3 const & b )
    {
        a.row[0] = b * a.row[0];
        a.row[1] = b * a.row[1];
        a.row[2] = b * a.row[2];
        return a;
    }

    // float4x4

    Float3x3 Minor( Float4x4 const & me, unsigned row, unsigned column )
    {
        assert( row < 4 );
        assert( column < 4 );
        Float3x3 float3x3(0);
        uint8_t minor_row = 0u;
        for( uint8_t major_row = 0u; major_row < 4u; ++major_row )
        {
            if( row != major_row )
            {
                uint8_t minor_column = 0u;
                for( uint8_t major_column = 0u; major_column < 4u; ++major_column )
                {
                    if( column != major_column )
                    {
                        float3x3( minor_row, minor_column ) = me( major_row, major_column );
                        ++minor_column;
                    }
                }
                ++minor_row;
            }
        }
        return float3x3;
    }


    Float2x2 Minor2x2( Float4x4 const & me, unsigned row1, unsigned row2, unsigned column1, unsigned column2 )
    {
        assert( row1 < 4 );
        assert( row2 < 4 );
        assert( column1 < 4 );
        assert( column2 < 4 );
        assert( row1 != row2 );
        assert( column1 != column2 );
        Float2x2 float2x2(0);
        uint8_t minor_row = 0u;
        for( uint8_t row = 0u; row < 4u; ++row )
        {
            if( row != row1 && row != row2 )
            {
                uint8_t minor_column = 0u;
                for( uint8_t column = 0u; column < 4u; ++column )
                {
                    if( column != column1 && column != column2 )
                    {
                        float2x2( minor_row, minor_column ) = me( row, column );
                        ++minor_column;
                    }
                }
                ++minor_row;
            }
        }
        return float2x2;
    }


    float CoFactor( Float4x4 const & me, unsigned row, unsigned column )
    {
        // (-1)^(row + column)
        float factor = (( row + column ) & 0x1) ? -1.0f : 1.0f;
        return factor * Determinant( Minor( me, row, column ) );
    }


    float CoFactor2x2( Float4x4 const & me, unsigned row1, unsigned row2, unsigned column1, unsigned column2 )
    {
        // (-1)^(row[1] + row[2] + column1 + column2)
        float factor = ( ( row1 + row2 + column1 + column2 ) & 0x1 ) ? -1.0f : 1.0f;
        return factor * Determinant( Minor2x2( me, row1, row2, column1, column2 ) );
    }


    void SetTranslation( Float4x4 & me, Float3 const & translation )
    {
        me.row[3] = Float4( translation.x, translation.y, translation.z, 1.f );
    }


    Float4x4 Transpose( Float4x4 const & in )
    {
        using namespace SSE;
        //return Float4x4( GetColumn( in, 0 ), GetColumn( in, 1 ), GetColumn( in, 2 ), GetColumn( in, 3 ) );
        auto m = SSEFromFloat4x4(in);
        Transpose(m.row[0], m.row[1], m.row[2], m.row[3]);
        return Float4x4FromSSE(m);
    }


    Float4x4 Inverse( Float4x4 const & in )
    {
        auto determinant = Determinant( in );
        //not sure how useful this assert is
        assert( determinant != 0 );
        return Adjugate( in ) / determinant;
    }

    // Unary operators
    Float4x4 operator+ ( Float4x4 const & me )
    {
        return me;
    }
    Float4x4 operator- ( Float4x4 const & me )
    {
        return Float4x4(0) - me;
    }


    // Assignment operators
    Float4x4& operator+= ( Float4x4 & me, Float4x4 const & other )
    {
        using namespace SSE;
        me.row[0] = Float4FromSSE( Add( SSEFromFloat4( me.row[0] ), SSEFromFloat4( other.row[0] ) ) );
        me.row[1] = Float4FromSSE( Add( SSEFromFloat4( me.row[1] ), SSEFromFloat4( other.row[1] ) ) );
        me.row[2] = Float4FromSSE( Add( SSEFromFloat4( me.row[2] ), SSEFromFloat4( other.row[2] ) ) );
        me.row[3] = Float4FromSSE( Add( SSEFromFloat4( me.row[3] ), SSEFromFloat4( other.row[3] ) ) );
        return me;
    }


    Float4x4& operator-= ( Float4x4 & me, Float4x4 const & other )
    {
        using namespace SSE;
        me.row[0] = Float4FromSSE( Subtract( SSEFromFloat4( me.row[0] ), SSEFromFloat4( other.row[0] ) ) );
        me.row[1] = Float4FromSSE( Subtract( SSEFromFloat4( me.row[1] ), SSEFromFloat4( other.row[1] ) ) );
        me.row[2] = Float4FromSSE( Subtract( SSEFromFloat4( me.row[2] ), SSEFromFloat4( other.row[2] ) ) );
        me.row[3] = Float4FromSSE( Subtract( SSEFromFloat4( me.row[3] ), SSEFromFloat4( other.row[3] ) ) );
        return me;
    }


    Float4x4& operator*= ( Float4x4 & me, Float4x4 const & other )
    {
        using namespace SSE;
        auto matrix = SSEFromFloat4x4( other );
        // manually unrolling the stupid loops generates about twice as fast code here...
        {
            {
                auto & row = me[0];
                auto result = Multiply4D( SSEFromFloat4( row ), matrix );
                row = Float4FromSSE( result );
            }
            {
                auto & row = me[1];
                auto result = Multiply4D( SSEFromFloat4( row ), matrix );
                row = Float4FromSSE( result );
            }
            {
                auto & row = me[2];
                auto result = Multiply4D( SSEFromFloat4( row ), matrix );
                row = Float4FromSSE( result );
            }
            {
                auto & row = me[3];
                auto result = Multiply4D( SSEFromFloat4( row ), matrix );
                row = Float4FromSSE( result );
            }
        }
        return me;
    }


    Float4x4& operator/= ( Float4x4 & me, const Float4x4 & other )
    {
        me *= Inverse( other );
        return me;
    }


    Float4x4& operator*= ( Float4x4 & me, float scalar )
    {
        using namespace SSE;
        auto scale = SetAll( scalar );
        me.row[0] = Float4FromSSE( Multiply( SSEFromFloat4( me.row[0] ), scale ) );
        me.row[1] = Float4FromSSE( Multiply( SSEFromFloat4( me.row[1] ), scale ) );
        me.row[2] = Float4FromSSE( Multiply( SSEFromFloat4( me.row[2] ), scale ) );
        me.row[3] = Float4FromSSE( Multiply( SSEFromFloat4( me.row[3] ), scale ) );
        return me;
    }


    Float4x4& operator/= ( Float4x4 & me, float scalar )
    {
        using namespace SSE;
        auto scale = SetAll( scalar );
        me.row[0] = Float4FromSSE( Divide( SSEFromFloat4( me.row[0] ), scale ) );
        me.row[1] = Float4FromSSE( Divide( SSEFromFloat4( me.row[1] ), scale ) );
        me.row[2] = Float4FromSSE( Divide( SSEFromFloat4( me.row[2] ), scale ) );
        me.row[3] = Float4FromSSE( Divide( SSEFromFloat4( me.row[3] ), scale ) );
        return me;
    }

    // Binary operators
    Float4x4 operator+ ( Float4x4 first, Float4x4 const & second )
    {
        return first += second;
    }
    Float4x4 operator- ( Float4x4 first, Float4x4 const & second )
    {
        return first -= second;
    }
    Float4x4 operator* ( Float4x4 first, Float4x4 const & second )
    {
        return first *= second;
    }
    Float4x4 operator* ( Float4x4 first, const float second )
    {
        return first *= second;
    }
    Float4x4 operator/ ( Float4x4 first, Float4x4 const & second )
    {
        return first /= second;
    }
    Float4x4 operator/ ( Float4x4 float4x4, const float scalar )
    {
        return float4x4 /= scalar;
    }
    Float4x4 operator* ( float scalar, Float4x4 float4x4 )
    {
        return float4x4 *= scalar;
    }
    Float4x4 operator/ ( float scalar, Float4x4 float4x4 )
    {
        return scalar * Inverse( float4x4 );
    }

    Float4& operator*= ( Float4& float4, Float4x4 const & float4x4 )
    {
        // float4 = Transpose( float4x4 ) * float4;
        float4 = float4 * float4x4;
        return float4;
    }

    Float4 operator* ( Float4 const & float4, Float4x4 const & float4x4 )
    {
        static_assert( sizeof( float4x4 ) == sizeof( SSE::FloatMatrix ), "Can't do the trick with reinterpret cast." );
        return SSE::Float4FromSSE( SSE::Multiply4D(  SSE::SSEFromFloat4( float4 ), reinterpret_cast<SSE::FloatMatrix const &>(float4x4) ) );
        // return float4 *= float4x4;
    }


    Float2 GetColumn( Float2x2 const & m, unsigned index )
    {
        return Float2( m(0,index), m(1,index) );
    }


    void SetColumn( Float2x2 & m, unsigned index, Float2 column )
    {
        m(0,index) = column[0];
        m(1,index) = column[1];
    }


    Float3 GetColumn( Float3x3 const & m, unsigned index )
    {
        return Float3( m.row[0][index], m.row[1][index], m.row[2][index] );
    }


    void SetColumn( Float3x3 & m, unsigned index, Float3 column )
    {
        m.row[0][index] = column[0];
        m.row[1][index] = column[1];
        m.row[2][index] = column[2];
    }


    Float4 GetColumn( Float4x4 const & m, unsigned index )
    {
        return Float4( m.row[0][index], m.row[1][index], m.row[2][index], m.row[3][index] );
    }


    void SetColumn( Float4x4 & m, unsigned index, Float4 column )
    {
        m.row[0][index] = column[0];
        m.row[1][index] = column[1];
        m.row[2][index] = column[2];
        m.row[3][index] = column[3];
    }

}
