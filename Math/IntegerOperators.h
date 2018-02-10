#pragma once
#include "IntegerTypes.h"

namespace Math
{
    // Unary operators
    Int2 operator+ (Int2 const me );
    Int2 operator- (Int2 const me );

    // Assignment operators
    Int2& operator+= (Int2& me, const Int2 other );
    Int2& operator-= (Int2& me, const Int2 other );
    Int2& operator*= (Int2& me, const Int2 other );
    Int2& operator/= (Int2& me, const Int2 other );

    // Binary operators
    Int2 operator+ ( Int2 first, const Int2 second );
    Int2 operator- ( Int2 first, const Int2 second );
    Int2 operator* ( Int2 first, const Int2 second );
    Int2 operator/ ( Int2 first, const Int2 second );

    // move the elements around, output will be (input[Index0], input[Index1])
    template<unsigned Index0, unsigned Index1>
    Int2 Swizzle( Int2 const & input );

    // Unary operators
    Int3 operator+ (Int3 me);
    Int3 operator- (Int3 me);

    // Assignment operators
    Int3& operator+= (Int3& me, const Int3 other );
    Int3& operator-= (Int3& me, const Int3 other );
    Int3& operator*= (Int3& me, const Int3 other );
    Int3& operator/= (Int3& me, const Int3 other );

    Int3 operator+ ( Int3 first, const Int3 & second );
    Int3 operator- ( Int3 first, const Int3 & second );
    Int3 operator* ( Int3 first, const Int3 & second );
    Int3 operator/ ( Int3 first, const Int3 & second );

    // move the elements around, output will be (input[Index0], input[Index1], input[Index2])
    template<unsigned Index0, unsigned Index1, unsigned Index2>
    Int3 Swizzle( Int3 const & input );

    // Unary operators
    Int4 operator+ (Int4 const & me );
    Int4 operator- (Int4 const & me );

    // Assignment operators
    Int4& operator+= (Int4& me, const Int4 other );
    Int4& operator-= (Int4& me, const Int4 other );
    Int4& operator*= (Int4& me, const Int4 other );
    Int4& operator/= (Int4& me, const Int4 other );

    Int4 operator+ ( Int4 first, const Int4 & second );
    Int4 operator- ( Int4 first, const Int4 & second );
    Int4 operator* ( Int4 first, const Int4 & second );
    Int4 operator/ ( Int4 first, const Int4 & second );

    // move the elements around, output will be (input[Index0], input[Index1], input[Index2], input[Index3])
    template<unsigned Index0, unsigned Index1, unsigned Index2, unsigned Index3>
    Int4 Swizzle( Int4 const & input );

    // Unary operators
    Unsigned2 operator+ (Unsigned2 const me );

    // Assignment operators
    Unsigned2& operator+= (Unsigned2& me, const Unsigned2 other );
    Unsigned2& operator-= (Unsigned2& me, const Unsigned2 other );
    Unsigned2& operator*= (Unsigned2& me, const Unsigned2 other );
    Unsigned2& operator/= (Unsigned2& me, const Unsigned2 other );

    // Binary operators
    Unsigned2 operator+ ( Unsigned2 first, const Unsigned2 second );
    Unsigned2 operator- ( Unsigned2 first, const Unsigned2 second );
    Unsigned2 operator* ( Unsigned2 first, const Unsigned2 second );
    Unsigned2 operator/ ( Unsigned2 first, const Unsigned2 second );

    // bitwise operators
    Unsigned2 operator& (Unsigned2 first, const Unsigned2 second);
    Unsigned2 operator| (Unsigned2 first, const Unsigned2 second);
    Unsigned2 operator^ (Unsigned2 first, const Unsigned2 second);
    Unsigned2 operator~ (Unsigned2 first);

    // move the elements around, output will be (input[Index0], input[Index1])
    template<unsigned Index0, unsigned Index1>
    Unsigned2 Swizzle( Unsigned2 const & input );


    // Unary operators
    Unsigned3 operator+ (Unsigned3 me);

    // Assignment operators
    Unsigned3& operator+= (Unsigned3& me, const Unsigned3 other );
    Unsigned3& operator-= (Unsigned3& me, const Unsigned3 other );
    Unsigned3& operator*= (Unsigned3& me, const Unsigned3 other );
    Unsigned3& operator/= (Unsigned3& me, const Unsigned3 other );

    Unsigned3 operator+ ( Unsigned3 first, const Unsigned3 & second );
    Unsigned3 operator- ( Unsigned3 first, const Unsigned3 & second );
    Unsigned3 operator* ( Unsigned3 first, const Unsigned3 & second );
    Unsigned3 operator/ ( Unsigned3 first, const Unsigned3 & second );

    // move the elements around, output will be (input[Index0], input[Index1], input[Index2])
    template<unsigned Index0, unsigned Index1, unsigned Index2>
    Unsigned3 Swizzle( Unsigned3 const & input );

    // Unary operators
    Unsigned4 operator+ (Unsigned4 const & me );

    // Assignment operators
    Unsigned4& operator+= (Unsigned4& me, const Unsigned4 other );
    Unsigned4& operator-= (Unsigned4& me, const Unsigned4 other );
    Unsigned4& operator*= (Unsigned4& me, const Unsigned4 other );
    Unsigned4& operator/= (Unsigned4& me, const Unsigned4 other );

    Unsigned4 operator+ ( Unsigned4 first, const Unsigned4 & second );
    Unsigned4 operator- ( Unsigned4 first, const Unsigned4 & second );
    Unsigned4 operator* ( Unsigned4 first, const Unsigned4 & second );
    Unsigned4 operator/ ( Unsigned4 first, const Unsigned4 & second );


    // move the elements around, output will be (input[Index0], input[Index1], input[Index2], input[Index3])
    template<unsigned Index0, unsigned Index1, unsigned Index2, unsigned Index3>
    Unsigned4 Swizzle( Unsigned4 const & input );
}

#include "IntegerOperators.inl"