#include "SSETypes.h"
#include "SSEFloatFunctions.h"

#include "SSEMathConversions.h"

namespace Math
{
    namespace SSE
    {
        Float32Vector VECTOR_CALL QuaternionConjugate( Float32Vector q )
        {
            auto minus_q = Negate(q);
            return Blend<0,0,0,1>(minus_q, q);
        }


        Float32Vector VECTOR_CALL QuaternionMultiply( Float32Vector a, Float32Vector b )
        {
            // original scalar version
            //auto vector = ( me.w * GetAxis( other ) + other.w * GetAxis( me ) + Cross( GetAxis( me ), GetAxis( other ) ) );
            //auto w = me.w * other.w - Dot( GetAxis( me ), GetAxis( other ) );
            //return{ vector, w };

            auto aw = Swizzle<3, 3, 3, 3>( a );
            auto bw = Swizzle<3, 3, 3, 3>( b );
            auto t0 = Multiply( aw, b );
            auto t1 = Multiply( bw, a );
            auto cross = Cross( a, b );
            // the dot product calculates 'too much' on purpose; it also uses the w component
            auto dot = Dot<1, 1, 1, 1, 0, 0, 0, 1>( a, b );
            // because here t0 + t1 produce 2 * a.w * b.w
            auto total = Add( t0, t1 );
            auto crossdot = Subtract( cross, dot );
            // so here one a.w * b.w gets cancelled out again
            auto result = Add( total, crossdot );
            return result;
        }


        Float32Vector VECTOR_CALL QuaternionRotate3DVector( Float32Vector v, Float32Vector r )
        {
            // original scalar version
            // auto cross = Cross( GetAxis( r ), v );
            // auto cross2 = Cross( GetAxis( r ), cross );
            // auto temp = r.w * cross + cross2;
            // return v + 2 * temp;

            auto v_yzx = Swizzle<1, 2, 0>( v );
            auto r_yzx = Swizzle<1, 2, 0>( r );

            // cross product 1
            auto t1 = Multiply( r, v_yzx );
            auto t2 = Multiply( v, r_yzx );
            auto cross_zxy = Subtract( t1, t2 );
            auto cross = Swizzle<1, 2, 0>( cross_zxy );

            // cross product 2, making smart use of already computed stuff
            auto t3_yzx = Multiply( r_yzx, cross_zxy );
            auto t4_yzx = Swizzle<1, 2, 0>( Multiply( cross, r_yzx ) );
            auto cross2 = Subtract( t3_yzx, t4_yzx );

            // finishing up
            auto const t5 = Multiply( Swizzle<3, 3, 3, 3>( r ), cross );
            auto t6 = Add(t5, cross2);
            return Add(v, Add(t6, t6));
        }
    }
}