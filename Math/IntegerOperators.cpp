#include "IntegerTypes.h"
#include "IntegerOperators.h"

namespace Math
{
    // Assignment operators
    Int2& operator+= ( Int2& me, const Int2 other )
    {
        me = me + other;
        return me;
    }
    Int2& operator-= ( Int2& me, const Int2 other )
    {
        me = me - other;
        return me;

    }
    Int2& operator*= ( Int2& me, const Int2 other )
    {
        me = me * other;
        return me;
    }
    Int2& operator/= ( Int2& me, const Int2 other )
    {
        me = me / other;
        return me;
    }

    // Unary operators
    Int2 operator+ ( Int2 const me )
    {
        return me;
    }
    Int2 operator- ( Int2 const me )
    {
        return Int2( -me.x, -me.y );
    }

    // Binary operators
    Int2 operator+ ( Int2 a, const Int2 b )
    {
        Int2 c;
        c.x = a.x + b.x;
        c.y = a.y + b.y;
        return c;
    }
    Int2 operator- ( Int2 a, const Int2 b )
    {
        Int2 c;
        c.x = a.x - b.x;
        c.y = a.y - b.y;
        return c;
    }
    Int2 operator* ( Int2 a, const Int2 b )
    {
        Int2 c;
        c.x = a.x * b.x;
        c.y = a.y * b.y;
        return c;
    }
    Int2 operator/ ( Int2 a, const Int2 b )
    {
        Int2 c;
        c.x = a.x / b.x;
        c.y = a.y / b.y;
        return c;
    }


    // Unary operators
    Int3 operator+ ( Int3 me )
    {
        return me;
    }
    Int3 operator- ( Int3 me )
    {
        return Int3( -me.x, -me.y, -me.z );
    }

    // Assignment operators
    Int3& operator+= ( Int3& me, const Int3 other )
    {
        me = me + other;
        return me;
    }

    Int3& operator-= ( Int3& me, const Int3 other )
    {
        me = me - other;
        return me;
    }

    Int3& operator*= ( Int3& me, const Int3 other )
    {
        me = me * other;
        return me;
    }

    Int3& operator/= ( Int3& me, const Int3 other )
    {
        me = me / other;
        return me;
    }

    Int3 operator+ ( Int3 a, const Int3 & b )
    {
        Int3 c;
        c.x = a.x + b.x;
        c.y = a.y + b.y;
        c.z = a.z + b.z;
        return c;
    }

    Int3 operator- ( Int3 a, const Int3 & b )
    {
        Int3 c;
        c.x = a.x - b.x;
        c.y = a.y - b.y;
        c.z = a.z - b.z;
        return c;
    }

    Int3 operator* ( Int3 a, const Int3 & b )
    {
        Int3 c;
        c.x = a.x * b.x;
        c.y = a.y * b.y;
        c.z = a.z * b.z;
        return c;
    }

    Int3 operator/ ( Int3 a, const Int3 & b )
    {
        Int3 c;
        c.x = a.x / b.x;
        c.y = a.y / b.y;
        c.z = a.z / b.z;
        return c;
    }

    // comparison
    bool operator==( Int3 const & a, Int3 const & b )
    {
        return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
    }

    bool operator!=( Int3 const & a, Int3 const & b )
    {
        return !( a == b );
    }


    // Unary operators
    Int4 operator+ ( Int4 const & me )
    {
        return me;
    }
    Int4 operator- ( Int4 const & me )
    {
        return Int4( -me.x, -me.y, -me.z, -me.w );
    }

    // Assignment operators
    Int4& operator+= ( Int4& me, const Int4 other )
    {
        me = me + other;
        return me;
    }
    Int4& operator-= ( Int4& me, const Int4 other )
    {
        me = me - other;
        return me;
    }
    Int4& operator*= ( Int4& me, const Int4 other )
    {
        me = me * other;
        return me;
    }
    Int4& operator/= ( Int4& me, const Int4 other )
    {
        me = me / other;
        return me;
    }

    Int4 operator+ ( Int4 a, const Int4 & b )
    {
        Int4 c;
        c.x = a.x + b.x;
        c.y = a.y + b.y;
        c.z = a.z + b.z;
        c.w = a.w + b.w;
        return c;
    }
    Int4 operator- ( Int4 a, const Int4 & b )
    {
        Int4 c;
        c.x = a.x - b.x;
        c.y = a.y - b.y;
        c.z = a.z - b.z;
        c.w = a.w - b.w;
        return c;
    }
    Int4 operator* ( Int4 a, const Int4 & b )
    {
        Int4 c;
        c.x = a.x * b.x;
        c.y = a.y * b.y;
        c.z = a.z * b.z;
        c.w = a.w * b.w;
        return c;
    }
    Int4 operator/ ( Int4 a, const Int4 & b )
    {
        Int4 c;
        c.x = a.x / b.x;
        c.y = a.y / b.y;
        c.z = a.z / b.z;
        c.w = a.w / b.w;
        return c;
    }

    // Assignment operators
    Unsigned2& operator+= ( Unsigned2& me, const Unsigned2 other )
    {
        me = me + other;
        return me;
    }
    Unsigned2& operator-= ( Unsigned2& me, const Unsigned2 other )
    {
        me = me - other;
        return me;

    }
    Unsigned2& operator*= ( Unsigned2& me, const Unsigned2 other )
    {
        me = me * other;
        return me;
    }
    Unsigned2& operator/= ( Unsigned2& me, const Unsigned2 other )
    {
        me = me / other;
        return me;
    }

    // Unary operators
    Unsigned2 operator+ ( Unsigned2 const me )
    {
        return me;
    }

    // Binary operators
    Unsigned2 operator+ ( Unsigned2 a, const Unsigned2 b )
    {
        Unsigned2 c;
        c.x = a.x + b.x;
        c.y = a.y + b.y;
        return c;
    }
    Unsigned2 operator- ( Unsigned2 a, const Unsigned2 b )
    {
        Unsigned2 c;
        c.x = a.x - b.x;
        c.y = a.y - b.y;
        return c;
    }
    Unsigned2 operator* ( Unsigned2 a, const Unsigned2 b )
    {
        Unsigned2 c;
        c.x = a.x * b.x;
        c.y = a.y * b.y;
        return c;
    }
    Unsigned2 operator/ ( Unsigned2 a, const Unsigned2 b )
    {
        Unsigned2 c;
        c.x = a.x / b.x;
        c.y = a.y / b.y;
        return c;
    }


    Unsigned2 operator& (Unsigned2 first, const Unsigned2 second)
    {
        return{first.x & second.x, first.y & second.y};
    }


    Unsigned2 operator| (Unsigned2 first, const Unsigned2 second)
    {
        return{first.x | second.x, first.y | second.y};
    }


    Unsigned2 operator^ (Unsigned2 first, const Unsigned2 second)
    {
        return{first.x ^ second.x, first.y ^ second.y};
    }


    Unsigned2 operator~ (Unsigned2 first)
    {
        return{~first.x, ~first.y};
    }


    // Unary operators
    Unsigned3 operator+ ( Unsigned3 me )
    {
        return me;
    }

    // Assignment operators
    Unsigned3& operator+= ( Unsigned3& me, const Unsigned3 other )
    {
        me = me + other;
        return me;
    }

    Unsigned3& operator-= ( Unsigned3& me, const Unsigned3 other )
    {
        me = me - other;
        return me;
    }

    Unsigned3& operator*= ( Unsigned3& me, const Unsigned3 other )
    {
        me = me * other;
        return me;
    }

    Unsigned3& operator/= ( Unsigned3& me, const Unsigned3 other )
    {
        me = me / other;
        return me;
    }

    Unsigned3 operator+ ( Unsigned3 a, const Unsigned3 & b )
    {
        Unsigned3 c;
        c.x = a.x + b.x;
        c.y = a.y + b.y;
        c.z = a.z + b.z;
        return c;
    }

    Unsigned3 operator- ( Unsigned3 a, const Unsigned3 & b )
    {
        Unsigned3 c;
        c.x = a.x - b.x;
        c.y = a.y - b.y;
        c.z = a.z - b.z;
        return c;
    }

    Unsigned3 operator* ( Unsigned3 a, const Unsigned3 & b )
    {
        Unsigned3 c;
        c.x = a.x * b.x;
        c.y = a.y * b.y;
        c.z = a.z * b.z;
        return c;
    }

    Unsigned3 operator/ ( Unsigned3 a, const Unsigned3 & b )
    {
        Unsigned3 c;
        c.x = a.x / b.x;
        c.y = a.y / b.y;
        c.z = a.z / b.z;
        return c;
    }

    // comparison
    bool operator==( Unsigned3 const & a, Unsigned3 const & b )
    {
        return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
    }

    bool operator!=( Unsigned3 const & a, Unsigned3 const & b )
    {
        return !( a == b );
    }

    // Unary operators
    Unsigned4 operator+ ( Unsigned4 const & me )
    {
        return me;
    }

    // Assignment operators
    Unsigned4& operator+= ( Unsigned4& me, const Unsigned4 other )
    {
        me = me + other;
        return me;
    }
    Unsigned4& operator-= ( Unsigned4& me, const Unsigned4 other )
    {
        me = me - other;
        return me;
    }
    Unsigned4& operator*= ( Unsigned4& me, const Unsigned4 other )
    {
        me = me * other;
        return me;
    }
    Unsigned4& operator/= ( Unsigned4& me, const Unsigned4 other )
    {
        me = me / other;
        return me;
    }

    Unsigned4 operator+ ( Unsigned4 a, const Unsigned4 & b )
    {
        Unsigned4 c;
        c.x = a.x + b.x;
        c.y = a.y + b.y;
        c.z = a.z + b.z;
        c.w = a.w + b.w;
        return c;
    }
    Unsigned4 operator- ( Unsigned4 a, const Unsigned4 & b )
    {
        Unsigned4 c;
        c.x = a.x - b.x;
        c.y = a.y - b.y;
        c.z = a.z - b.z;
        c.w = a.w - b.w;
        return c;
    }
    Unsigned4 operator* ( Unsigned4 a, const Unsigned4 & b )
    {
        Unsigned4 c;
        c.x = a.x * b.x;
        c.y = a.y * b.y;
        c.z = a.z * b.z;
        c.w = a.w * b.w;
        return c;
    }
    Unsigned4 operator/ ( Unsigned4 a, const Unsigned4 & b )
    {
        Unsigned4 c;
        c.x = a.x / b.x;
        c.y = a.y / b.y;
        c.z = a.z / b.z;
        c.w = a.w / b.w;
        return c;
    }

}