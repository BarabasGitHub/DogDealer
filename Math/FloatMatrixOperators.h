#pragma once
#include "FloatMatrixTypes.h"
#include "FloatTypes.h"
#include "FloatOperators.h"

namespace Math
{
    // float2x2

    // Assignment operators
    Float2x2& operator+= (Float2x2&, const Float2x2 other );
    Float2x2& operator-= (Float2x2&, const Float2x2 other );
    Float2x2& operator*= (Float2x2&, const Float2x2 & other );
    Float2x2& operator/= (Float2x2&, const Float2x2 other );
    Float2x2& operator+= (Float2x2&, float scalar );
    Float2x2& operator-= (Float2x2&, float scalar );
    Float2x2& operator*= (Float2x2&, float scalar );
    Float2x2& operator/= (Float2x2&, float scalar );
    Float2& operator*= ( Float2& me, Float2x2 float2x2 );

    // Unary operators
    Float2x2 operator+ (Float2x2 const);
    Float2x2 operator- (Float2x2 const);

    // Binary operators
    Float2x2 operator+ ( Float2x2 first, const Float2x2 second );
    Float2x2 operator- ( Float2x2 first, const Float2x2 second );
    Float2x2 operator* ( Float2x2 first, const Float2x2 second );
    Float2x2 operator* ( Float2x2 first, const float second );
    Float2x2 operator/ ( Float2x2 first, const Float2x2 second );
    Float2x2 operator/ ( Float2x2 first, const float second );
    Float2x2 operator* ( float scalar, Float2x2 float2x2 );
    Float2x2 operator/ ( float scalar, Float2x2 float2x2 );
    Float2   operator* ( Float2x2 float2x2, Float2 float2 );
    Float2   operator* ( Float2 float2, Float2x2 float2x2 );

    Float2x2 CoFactorMatrix(Float2x2 const);
    Float2x2 Transpose(Float2x2 const);
    Float2x2 Adjugate(Float2x2 const);
    float Determinant(Float2x2 const);
    Float2x2 Inverse(Float2x2 const);

    float Minor( Float2x2 const &, unsigned row, unsigned column );
    float CoFactor( Float2x2 const &, unsigned row, unsigned column );

    Float2 GetColumn( Float2x2 const & m, unsigned index );
    void SetColumn( Float2x2 & m, unsigned index, Float2 column );

    // float3x3

    Float3x3 CoFactorMatrix(Float3x3 const &);
    Float3x3 Transpose(Float3x3 const & );
    Float3x3 Adjugate(Float3x3 const &) ;
    float Determinant(Float3x3 const &);
    Float3x3 Inverse(Float3x3 const &);

    Float2x2 Minor( Float3x3 const & , unsigned row, unsigned column );
    float CoFactor( Float3x3 const & , unsigned row, unsigned column );


    Float3 GetColumn( Float3x3 const & m, unsigned index );
    void SetColumn( Float3x3 & m, unsigned index, Float3 column );

    // Assignment operators
    Float3x3& operator+= (Float3x3&, const Float3x3 other );
    Float3x3& operator-= (Float3x3&, const Float3x3 other );
    Float3x3& operator*= (Float3x3&, const Float3x3 & other );
    Float3x3& operator/= (Float3x3&, const Float3x3 & other );
    Float3x3& operator+= (Float3x3&, float scalar );
    Float3x3& operator-= (Float3x3&, float scalar );
    Float3x3& operator*= (Float3x3&, float scalar );
    Float3x3& operator/= (Float3x3&, float scalar );
    Float3& operator*= (Float3&, Float3x3 const & float3x3 );

    // Unary operators
    Float3x3 operator+ (Float3x3 const & );
    Float3x3 operator- (Float3x3 const & );

    // Binary operators
    Float3x3 operator+ ( Float3x3 first, const Float3x3 second );
    Float3x3 operator- ( Float3x3 first, const Float3x3 second );
    Float3x3 operator* ( Float3x3 first, Float3x3 const & second );
    Float3x3 operator* ( Float3x3 first, const float second );
    Float3x3 operator/ ( Float3x3 first, const Float3x3 second );
    Float3x3 operator/ ( Float3x3 first, const float second );
    Float3x3 operator* ( float scalar, Float3x3 float3x3 );
    Float3x3 operator/ ( float scalar, Float3x3 float3x3 );
    Float3 operator* ( Float3x3 float3x3, Float3 float3 );
    Float3 operator* ( Float3 float3, Float3x3 const & float3x3 );

    Float3x3 MultiplyTransposed( Float3x3 a, Float3x3 const & b );

    // float4x4

    // Assignment operators
    Float4x4& operator+= (Float4x4&, const Float4x4 & other );
    Float4x4& operator-= (Float4x4&, const Float4x4 & other );
    Float4x4& operator*= (Float4x4&, const Float4x4 & other );
    Float4x4& operator/= (Float4x4&, const Float4x4 & other );
    Float4x4& operator*= (Float4x4&, float scalar );
    Float4x4& operator/= (Float4x4&, float scalar );
    Float4& operator*= (Float4&, Float4x4 const & );

    // Unary operators
    Float4x4 operator+ (Float4x4 const & );
    Float4x4 operator- (Float4x4 const & );

    // Binary operators
    Float4x4 operator+ ( Float4x4 first, Float4x4 const & second );
    Float4x4 operator- ( Float4x4 first, Float4x4 const & second );
    Float4x4 operator* ( Float4x4 first, Float4x4 const & second );
    Float4x4 operator* ( Float4x4 first, const float second );
    Float4x4 operator/ ( Float4x4 first, Float4x4 const & second );
    Float4x4 operator/ ( Float4x4 first, const float second );
    Float4x4 operator* ( float scalar, Float4x4 float4x4 );
    Float4x4 operator/ ( float scalar, Float4x4 float4x4 );
    Float4   operator* ( Float4x4 const & float4x4, Float4 const & float4 );
    Float4   operator* ( Float4 const & float4, Float4x4 const & float4x4 );

    Float4x4 Transpose(Float4x4 const &);
    Float4x4 Adjugate(Float4x4 const & );
    float Determinant(Float4x4 const & );
    Float4x4 Inverse(Float4x4 const & );

    Float3x3 Minor( Float4x4 const &, unsigned row, unsigned column );
    Float2x2 Minor2x2( Float4x4 const &, unsigned row1, unsigned row2, unsigned column1, unsigned column2 );

    float CoFactor( Float4x4 const & , unsigned row, unsigned column );
    float CoFactor2x2( Float4x4 const &, unsigned row1, unsigned row2, unsigned column1, unsigned column2 );

    Float4 GetColumn( Float4x4 const & m, unsigned index );
    void SetColumn( Float4x4 & m, unsigned index, Float4 column );
}
