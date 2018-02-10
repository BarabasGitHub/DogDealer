#include "SSETransformFunctions.h"
#include "SSEFloatFunctions.h"
#include "SSESetFunctions.h"

#include "MathConstants.h"

namespace Math
{
    namespace SSE
    {
        FloatMatrix VECTOR_CALL RotationMatrixFromQuaternion( Float32Vector q )
        {
            return AffineTransformTranslationRotation( ZeroFloat32Vector(), q );
        }


        FloatMatrix VECTOR_CALL AffineTransformTranslationRotation( Float32Vector translation, Float32Vector q )
        {
            // based on rotating the 1,0,0 0,1,0 0,0,1 vectors with q

            // so we don't have to multiply with 2 later
            auto const sqrt2 = SetAll( c_SQRT2 );
            q = Multiply( q, sqrt2 );
            auto const axis = Blend<0,0,0,1>(q, ZeroFloat32Vector());
            auto axis_zxy = Swizzle<2, 0, 1>( axis );
            auto qw_zxy = Multiply( axis_zxy, Swizzle<3, 3, 3, 3>( q ) );
            auto qq_zxy = Multiply( axis_zxy, axis_zxy );
            auto axis_xyx = Swizzle<0, 1, 0>( axis );
            auto axis_yzz = Swizzle<1, 2, 2>( axis );
            auto xy_yz_xz = Multiply( axis_xyx, axis_yzz );

            auto ones = SetAll( 1.f );

            //1 - qq.y - qq.z
            //1 - qq.z - qq.x
            //1 - qq.x - qq.y
            auto qq_yzx = Swizzle<2, 0, 1>( qq_zxy );
            auto diagonal = Subtract( ones, Add( qq_yzx, qq_zxy ) );

            //xy - qw.z
            //yz - qw.x
            //xz - qw.y
            auto off_diagonal_sub = Subtract( xy_yz_xz, qw_zxy );

            //xz + qw.y;
            //xy + qw.z;
            //yz + qw.x;
            auto off_diagonal_add = Add( xy_yz_xz, qw_zxy );
            off_diagonal_add = Swizzle<2, 0, 1>( off_diagonal_add );

            Transpose( diagonal, off_diagonal_sub, off_diagonal_add, translation );
            // we now have
            // 1 - qq.y - qq.z, xy - qw.z, xz + qw.y, translation.x
            // 1 - qq.z - qq.x, yz - qw.x, xy + qw.z, translation.y
            // 1 - qq.x - qq.y, xz - qw.y, yz + qw.x, translation.z
            // 1 -  0   -  0  ,  0 -  0  ,  0 +  0  ,           0
            auto row0 = diagonal;
            auto row1 = off_diagonal_sub;
            auto row2 = off_diagonal_add;
            auto row3 = translation;

            row0 = row0;
            row1 = Swizzle<2, 0, 1, 3>( row1 );
            row2 = Swizzle<1, 2, 0, 3>( row2 );
            row3 = Swizzle<3, 3, 3, 0>( row3 ); // or Blend<0,0,0,1>(ZeroFloat32Vector(), ones);
            // and now it is
            // 1 - qq.y - qq.z, xy - qw.z, xz + qw.y, translation.x
            // xy + qw.z, 1 - qq.z - qq.x, yz - qw.x, translation.y
            // xz - qw.y, yz + qw.x, 1 - qq.x - qq.y, translation.z
            //       0  ,       0  ,             0  ,           1

            return{ { row0, row1, row2, row3 } };
        }
    }
}