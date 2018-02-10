#pragma once

#include "FloatTypes.h"
#include "IntegerTypes.h"
#include "FloatMatrixTypes.h"
#include "SSESetFunctions.h"
#include "SSEFloatFunctions.h"
#include "SSELoadStore.h"

namespace Math
{
    namespace SSE
    {
        inline Float2 Float2FromSSE( Float32Vector v )
        {
            Float2 r;
            static_assert( sizeof( r ) == sizeof( float ) * 2, "Can't use Store2, because the Float2 is not exactly the size of two floats." );
            Store2( v, begin( r ) );
            return r;
        }


        inline Float3 Float3FromSSE( Float32Vector v )
        {
            Float3 r;
            Store2( v, begin( r ) );
            // r.z = GetSingle( Swizzle<2,3,2,3>( v ) );
            r.z = GetSingle( Swizzle<2>( v ) );
            return r;
        }


        inline Float4 Float4FromSSE( Float32Vector v )
        {
            Float4 r;
            static_assert( sizeof( r ) == sizeof( v ), "Size of Float4 does not match that of a SSE register." );
            Store( v, begin( r ) );
            return r;
        }


        inline Quaternion QuaternionFromSSE( Float32Vector v )
        {
            static_assert( sizeof( Float4 ) == sizeof( Quaternion ), "Size of Float4 does not match that of a Quaternion, cannot do reinterpret cast." );
            auto f4 = Float4FromSSE( v );
            return reinterpret_cast<Quaternion&>( f4 );
        }


        inline Float32Vector SSEFromFloat4( Float4 const & f )
        {
            static_assert( sizeof( Float32Vector ) == sizeof( f ), "Size of Float4 does not match that of a SSE register." );
            return LoadFloat32Vector( begin( f ) );
        }


        inline Float32Vector SSEFromQuaternion( Quaternion const & f )
        {
            static_assert( sizeof( Float4 ) == sizeof( Quaternion ), "Size of Float4 does not match that of a Quaternion, cannot do reinterpret cast." );
            return SSEFromFloat4( reinterpret_cast<Float4 const&>( f ) );
        }



        inline Float32Vector SSEFromFloat3( Float3 const & f )
        {
            // auto z = SetSingle( f.z );
            // auto xy = Load2( begin( f ) );
            // return Shuffle<0,1,0,1>(xy, z)
            auto xyzw = LoadFloat32Vector( begin(f) );
            return Blend<0,0,0,1>(xyzw, ZeroFloat32Vector());
        }


        inline Float32Vector SSEFromFloat2( Float2 const & f )
        {
            return Load2( begin( f ) );
        }


        inline FloatMatrix SSEFromFloat4x4( Float4x4 const & m )
        {
            return{ { SSEFromFloat4( m[0] ), SSEFromFloat4( m[1] ), SSEFromFloat4( m[2] ), SSEFromFloat4( m[3] ) } };
        }


        inline FloatMatrix SSEFromFloat3x3( Float3x3 const & m )
        {
            return{ { SSEFromFloat3( m[0] ), SSEFromFloat3( m[1] ), SSEFromFloat3( m[2] ), ZeroFloat32Vector() } };
        }


        inline Float4x4 Float4x4FromSSE( FloatMatrix const & m )
        {
            return{ Float4FromSSE( m.row[0] ), Float4FromSSE( m.row[1] ), Float4FromSSE( m.row[2] ), Float4FromSSE( m.row[3] ) };
        }



        inline IntegerVector SSEFromUnsigned2( Unsigned2 const & u)
        {
            return Load2(begin(u));
        }


        inline Unsigned4 Unsigned4FromSSE( IntegerVector u )
        {
            Unsigned4 r;
            Store( u, begin( r ) );
            return r;
        }

    }
}