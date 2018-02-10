#include "FloatOperators.h"
#include "MathFunctions.h"
#include "SSEFloatFunctions.h"
#include "SSEMathConversions.h"

#include <cmath>


namespace Math
{
    // Assignment operators
    Float2& operator+= ( Float2& me, const Float2 other )
    {
        me = me + other;
        return me;
    }
    Float2& operator-= ( Float2& me, const Float2 other )
    {
        me = me - other;
        return me;

    }
    Float2& operator*= ( Float2& me, const Float2 other )
    {
        me = me * other;
        return me;
    }
    Float2& operator/= ( Float2& me, const Float2 other )
    {
        me = me / other;
        return me;
    }

    // Unary operators
    Float2 operator+ ( Float2 const me )
    {
        return me;
    }
    Float2 operator- ( Float2 const me )
    {
        return Float2( -me.x, -me.y );
    }

    // Binary operators
    Float2 operator+ ( Float2 a, const Float2 b )
    {
        Float2 c;
        c.x = a.x + b.x;
        c.y = a.y + b.y;
        return c;
    }
    Float2 operator- ( Float2 a, const Float2 b )
    {
        Float2 c;
        c.x = a.x - b.x;
        c.y = a.y - b.y;
        return c;
    }
    Float2 operator* ( Float2 a, const Float2 b )
    {
        Float2 c;
        c.x = a.x * b.x;
        c.y = a.y * b.y;
        return c;
    }
    Float2 operator/ ( Float2 a, const Float2 b )
    {
        Float2 c;
        c.x = a.x / b.x;
        c.y = a.y / b.y;
        return c;
    }

    // comparison
    bool operator==( Float2 const & a, Float2 const & b )
    {
        return a[0] == b[0] && a[1] == b[1];
    }

    bool operator!=( Float2 const & a, Float2 const & b )
    {
        return !( a == b );
    }

    // Unary operators
    Float3 operator+ ( Float3 me )
    {
        return me;
    }
    Float3 operator- ( Float3 me )
    {
        return Float3( -me.x, -me.y, -me.z );
    }

    // Assignment operators
    Float3& operator+= ( Float3& me, const Float3 other )
    {
        me = me + other;
        return me;
    }

    Float3& operator-= ( Float3& me, const Float3 other )
    {
        me = me - other;
        return me;
    }

    Float3& operator*= ( Float3& me, const Float3 other )
    {
        me = me * other;
        return me;
    }

    Float3& operator/= ( Float3& me, const Float3 other )
    {
        me = me / other;
        return me;
    }

    Float3 operator+ ( Float3 a, const Float3 & b )
    {
        Float3 c;
        c.x = a.x + b.x;
        c.y = a.y + b.y;
        c.z = a.z + b.z;
        return c;
    }

    Float3 operator- ( Float3 a, const Float3 & b )
    {
        Float3 c;
        c.x = a.x - b.x;
        c.y = a.y - b.y;
        c.z = a.z - b.z;
        return c;
    }

    Float3 operator* ( Float3 a, const Float3 & b )
    {
        Float3 c;
        c.x = a.x * b.x;
        c.y = a.y * b.y;
        c.z = a.z * b.z;
        return c;
    }

    Float3 operator/ ( Float3 a, const Float3 & b )
    {
        Float3 c;
        c.x = a.x / b.x;
        c.y = a.y / b.y;
        c.z = a.z / b.z;
        return c;
    }

    // comparison
    bool operator==( Float3 const & a, Float3 const & b )
    {
        return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
    }

    bool operator!=( Float3 const & a, Float3 const & b )
    {
        return !( a == b );
    }

    // Unary operators
    Float4 operator+ ( Float4 const & me )
    {
        return me;
    }
    Float4 operator- ( Float4 const & me )
    {
        return Float4( -me.x, -me.y, -me.z, -me.w );
    }

    // Assignment operators
    Float4& operator+= ( Float4& me, const Float4 other )
    {
        me = me + other;
        return me;
    }
    Float4& operator-= ( Float4& me, const Float4 other )
    {
        me = me - other;
        return me;
    }
    Float4& operator*= ( Float4& me, const Float4 other )
    {
        me = me * other;
        return me;
    }
    Float4& operator/= ( Float4& me, const Float4 other )
    {
        me = me / other;
        return me;
    }

    Float4 operator+ ( Float4 a, const Float4 & b )
    {
        Float4 c;
        c.x = a.x + b.x;
        c.y = a.y + b.y;
        c.z = a.z + b.z;
        c.w = a.w + b.w;
        return c;
    }
    Float4 operator- ( Float4 a, const Float4 & b )
    {
        Float4 c;
        c.x = a.x - b.x;
        c.y = a.y - b.y;
        c.z = a.z - b.z;
        c.w = a.w - b.w;
        return c;
    }
    Float4 operator* ( Float4 a, const Float4 & b )
    {
        Float4 c;
        c.x = a.x * b.x;
        c.y = a.y * b.y;
        c.z = a.z * b.z;
        c.w = a.w * b.w;
        return c;
    }
    Float4 operator/ ( Float4 a, const Float4 & b )
    {
        Float4 c;
        c.x = a.x / b.x;
        c.y = a.y / b.y;
        c.z = a.z / b.z;
        c.w = a.w / b.w;
        return c;
    }


    bool operator==( Float4 const & a, Float4 const & b )
    {
        return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
    }


    bool operator!=( Float4 const & a, Float4 const & b )
    {
        return !( a == b );
    }

    // Unary operators
    Quaternion operator+ ( Quaternion const & me )
    {
        return me;
    }


    Quaternion operator- ( Quaternion const & me )
    {
        return Quaternion( -me.x, -me.y, -me.z, -me.w );
    }

    // Assignment operators
    Quaternion& operator+= ( Quaternion& me, Quaternion const & other )
    {
        me = me + other;
        return me;
    }


    Quaternion& operator-= ( Quaternion& me, Quaternion const & other )
    {
        me = me - other;
        return me;
    }


    Quaternion& operator*= ( Quaternion& me, Quaternion const & other )
    {
        me = me * other;
        return me;
    }


    Quaternion& operator*= ( Quaternion& me, float scalar )
    {
        me = me * scalar;
        return me;
    }
    Quaternion& operator/= ( Quaternion& me, float scalar )
    {
        me = me / scalar;
        return me;
    }

    Quaternion operator+ ( Quaternion a, const Quaternion b )
    {
        Quaternion c;
        c.x = a.x + b.x;
        c.y = a.y + b.y;
        c.z = a.z + b.z;
        c.w = a.w + b.w;
        return c;
    }
    Quaternion operator- ( Quaternion a, const Quaternion b )
    {
        Quaternion c;
        c.x = a.x - b.x;
        c.y = a.y - b.y;
        c.z = a.z - b.z;
        c.w = a.w - b.w;
        return c;
    }
    //Quaternion operator* ( Quaternion first, const Quaternion second )
    //{
    //    me = Quaternion(
    //        Float3( me.y * other.z - me.z * other.y,
    //        me.z * other.x - me.x * other.z,
    //        me.x * other.y - me.y * other.x )
    //        +
    //        ( me.w * GetAxis( other ) + other.w * GetAxis( me )),
    //        me.w * other.w - ( me.x * other.x + me.y * other.y + me.z * other.z ));
    //    return me;
    //}
    Quaternion operator* ( Quaternion q, const float scalar )
    {
        Quaternion c;
        c.x = q.x * scalar;
        c.y = q.y * scalar;
        c.z = q.z * scalar;
        c.w = q.w * scalar;
        return c;
    }

    Quaternion operator/ ( Quaternion q, const float scalar )
    {
        Quaternion c;
        c.x = q.x / scalar;
        c.y = q.y / scalar;
        c.z = q.z / scalar;
        c.w = q.w / scalar;
        return c;
    }


    Quaternion operator* ( float scalar, Quaternion quaternion )
    {
        return quaternion * scalar;
    }


    bool operator==( Quaternion const & a, Quaternion const & b )
    {
        return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
    }


    bool operator!=( Quaternion const & a, Quaternion const & b )
    {
        return !( a == b );
    }



    Quaternion Conjugate( Quaternion const & in )
    {
        return Quaternion( -in.x, -in.y, -in.z, in.w );
    }


    Float3 const & GetAxis( Quaternion const & in )
    {
        //return Float3( in.x, in.y, in.z );
        return reinterpret_cast<Float3 const &>(in);
    }


    Quaternion operator* (Quaternion me, Quaternion const & other )
    {
        auto vector = ( me.w * GetAxis(other) + other.w * GetAxis(me) + Cross( GetAxis(me), GetAxis(other) ) );
        auto w = me.w * other.w - Dot( GetAxis(me), GetAxis(other) );
        return{ vector, w };
    }


    Quaternion Inverse(Quaternion const & in)
    {
        return Conjugate(in) / Dot( in, in );
    }


    float GetAngle(Quaternion const & in)
    {
        //return std::acos(in.w) * 2.0f;

        // this should be more stable?
        auto length = Norm( GetAxis(in) );

        return 2 * std::atan2( length, in.w );
    }
}