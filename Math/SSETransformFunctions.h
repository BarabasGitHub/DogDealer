#pragma once
#include "SSETypes.h"

namespace Math
{
    namespace SSE
    {
        FloatMatrix VECTOR_CALL RotationMatrixFromQuaternion( Float32Vector q );
        FloatMatrix VECTOR_CALL AffineTransformTranslationRotation( Float32Vector translation, Float32Vector rotation_quaternion );
    }
}