#pragma once
#include "SSETypes.h"

namespace Math
{
    namespace SSE
    {
        // assumes the last component of the output will be zero
        Float32Vector VECTOR_CALL Multiply3D( FloatMatrix const & matrix, Float32Vector vector3d );
        // assumes the last component of the output will be zero
        Float32Vector VECTOR_CALL Multiply3D( Float32Vector vector3d, FloatMatrix const & matrix );

        Float32Vector VECTOR_CALL Multiply4D( FloatMatrix const & matrix, Float32Vector vector4d );
        Float32Vector VECTOR_CALL Multiply4D( Float32Vector vector4d, FloatMatrix const & matrix );

        // transforms a 3d position, basically x, y, z, 1
        // the output is a 3d 'vector', with w == 0
        Float32Vector VECTOR_CALL TransformPosition3D( Float32Vector position, FloatMatrix const & tranform );
        // transforms a 3d vector (actually x, y, z, 0)
        // the output is also a 3d vector, with w == 0
        Float32Vector VECTOR_CALL TransformVector3D( Float32Vector vector3d, FloatMatrix const & tranform );


        void VECTOR_CALL Transpose(FloatMatrix & m);

        FloatMatrix VECTOR_CALL Multiply( FloatMatrix const & a, FloatMatrix const & b );
    }
}