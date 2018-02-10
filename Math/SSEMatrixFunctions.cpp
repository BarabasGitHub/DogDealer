#include "SSEMatrixFunctions.h"
#include "SSEFloatFunctions.h"

namespace Math
{
    using namespace SSE;

    Float32Vector VECTOR_CALL SSE::Multiply3D( FloatMatrix const & matrix, Float32Vector vector3d )
    {
        auto x = Multiply( matrix.row[0], vector3d );
        auto y = Multiply( matrix.row[1], vector3d );
        auto z = Multiply( matrix.row[2], vector3d );
        auto w = ZeroFloat32Vector();
        auto xy = HorizontalAdd( x, y );
        auto zw = HorizontalAdd( z, w );
        auto xyzw = HorizontalAdd( xy, zw );
        return xyzw;
    }


    Float32Vector VECTOR_CALL SSE::Multiply3D( Float32Vector vector3d,  FloatMatrix const & matrix )
    {
       auto x = Multiply( matrix.row[0], Swizzle<0, 0, 0, 0>( vector3d ) );
       auto y = Multiply( matrix.row[1], Swizzle<1, 1, 1, 1>( vector3d ) );
       auto z = Multiply( matrix.row[2], Swizzle<2, 2, 2, 2>( vector3d ) );
       auto r1 = Add( x, y );
       auto r = Add( r1, z );
       return r;
    }


    Float32Vector VECTOR_CALL SSE::Multiply4D( FloatMatrix const & matrix, Float32Vector vector4d )
    {
        auto x = Multiply( matrix.row[0], vector4d );
        auto y = Multiply( matrix.row[1], vector4d );
        auto z = Multiply( matrix.row[2], vector4d );
        auto w = Multiply( matrix.row[3], vector4d );
        auto xy = HorizontalAdd( x, y );
        auto zw = HorizontalAdd( z, w );
        auto xyzw = HorizontalAdd( xy, zw );
        return xyzw;
    }


    Float32Vector VECTOR_CALL SSE::Multiply4D( Float32Vector vector4d, FloatMatrix const & matrix )
    {
        auto x = Multiply( matrix.row[0], Swizzle<0,0,0,0>(vector4d) );
        auto y = Multiply( matrix.row[1], Swizzle<1,1,1,1>(vector4d) );
        auto z = Multiply( matrix.row[2], Swizzle<2,2,2,2>(vector4d) );
        auto w = Multiply( matrix.row[3], Swizzle<3,3,3,3>(vector4d) );
        auto r1 = Add( x, y );
        auto r2 = Add( z, w );
        auto r = Add( r1, r2 );
        return r;
    }


    void VECTOR_CALL SSE::Transpose(FloatMatrix & m)
    {
        Transpose(m.row[0], m.row[1], m.row[2], m.row[3]);
    }


    FloatMatrix VECTOR_CALL SSE::Multiply( FloatMatrix const & a, FloatMatrix const & b)
    {
        FloatMatrix r;
        r.row[0] = Multiply4D(a.row[0], b);
        r.row[1] = Multiply4D(a.row[1], b);
        r.row[2] = Multiply4D(a.row[2], b);
        r.row[3] = Multiply4D(a.row[3], b);
        return r;
    }


    Float32Vector VECTOR_CALL SSE::TransformPosition3D( Float32Vector position, FloatMatrix const & tranform )
    {
        position = Blend<0,0,0,1>( position, SetAll( 1.f ) );
        return Multiply3D( tranform, position );
    }


    Float32Vector VECTOR_CALL SSE::TransformVector3D( Float32Vector vector3d, FloatMatrix const & tranform )
    {
        vector3d = Blend<0, 0, 0, 1>( vector3d, ZeroFloat32Vector() );
        return Multiply3D( tranform, vector3d );
    }
}