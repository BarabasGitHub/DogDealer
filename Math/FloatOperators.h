#pragma once
#include "FloatTypes.h"

namespace Math
{
    // Unary operators
    Float2 operator+ (Float2 const me );
    Float2 operator- (Float2 const me );

    // Assignment operators
    Float2& operator+= (Float2& me, const Float2 other );
    Float2& operator-= (Float2& me, const Float2 other );
    Float2& operator*= (Float2& me, const Float2 other );
    Float2& operator/= (Float2& me, const Float2 other );

    // Binary operators
    Float2 operator+ ( Float2 first, const Float2 second );
    Float2 operator- ( Float2 first, const Float2 second );
    Float2 operator* ( Float2 first, const Float2 second );
    Float2 operator/ ( Float2 first, const Float2 second );

    // comparison
    bool operator==( Float2 const & a, Float2 const & b );
    bool operator!=( Float2 const & a, Float2 const & b );

    // move the elements around, output will be (input[Index0], input[Index1])
    template<unsigned Index0, unsigned Index1>
    Float2 Swizzle( Float2 const & input );

    // Unary operators
    Float3 operator+ (Float3 me);
    Float3 operator- (Float3 me);

    // Assignment operators
    Float3& operator+= (Float3& me, const Float3 other );
    Float3& operator-= (Float3& me, const Float3 other );
    Float3& operator*= (Float3& me, const Float3 other );
    Float3& operator/= (Float3& me, const Float3 other );

    Float3 operator+ ( Float3 first, const Float3 & second );
    Float3 operator- ( Float3 first, const Float3 & second );
    Float3 operator* ( Float3 first, const Float3 & second );
    Float3 operator/ ( Float3 first, const Float3 & second );

    // comparison
    bool operator==(Float3 const & a, Float3 const & b);
    bool operator!=( Float3 const & a, Float3 const & b );

    // move the elements around, output will be (input[Index0], input[Index1], input[Index2])
    template<unsigned Index0, unsigned Index1, unsigned Index2>
    Float3 Swizzle( Float3 const & input );

    // Unary operators
    Float4 operator+ (Float4 const & me );
    Float4 operator- (Float4 const & me );

    // Assignment operators
    Float4& operator+= (Float4& me, const Float4 other );
    Float4& operator-= (Float4& me, const Float4 other );
    Float4& operator*= (Float4& me, const Float4 other );
    Float4& operator/= (Float4& me, const Float4 other );

    Float4 operator+ ( Float4 first, const Float4 & second );
    Float4 operator- ( Float4 first, const Float4 & second );
    Float4 operator* ( Float4 first, const Float4 & second );
    Float4 operator/ ( Float4 first, const Float4 & second );

    // comparison
    bool operator==( Float4 const & a, Float4 const & b );
    bool operator!=( Float4 const & a, Float4 const & b );

    // move the elements around, output will be (input[Index0], input[Index1], input[Index2], input[Index3])
    template<unsigned Index0, unsigned Index1, unsigned Index2, unsigned Index3>
    Float4 Swizzle( Float4 const & input );

    // Unary operators
    Quaternion operator+ (Quaternion const & me);
    Quaternion operator- (Quaternion const & me);

    // Assignment operators
    Quaternion& operator-= (Quaternion& me, Quaternion const & other );
    Quaternion& operator+= (Quaternion& me, Quaternion const & other );
    Quaternion& operator*= (Quaternion& me, Quaternion const & other );
    Quaternion& operator*= (Quaternion& me, float scalar );
    Quaternion& operator/= (Quaternion& me, float scalar );

    Quaternion operator+ ( Quaternion first, const Quaternion second );
    Quaternion operator- ( Quaternion first, const Quaternion second );
    Quaternion operator* ( Quaternion first, Quaternion const & second );
    Quaternion operator* ( Quaternion quaternion, const float scalar );
    Quaternion operator/ ( Quaternion quaternion, const float scalar );
    Quaternion operator* ( float scalar, Quaternion quaternion );

    // comparison
    bool operator==( Quaternion const & a, Quaternion const & b );
    bool operator!=( Quaternion const & a, Quaternion const & b );

    Quaternion Conjugate(Quaternion const &);
    Quaternion Inverse(Quaternion const &);

    float GetAngle(Quaternion const &);
    Float3 const & GetAxis(Quaternion const &);

}

#include "FloatOperators.inl"