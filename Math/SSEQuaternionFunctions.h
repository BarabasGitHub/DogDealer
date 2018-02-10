#pragma once

#include "SSETypes.h"


namespace Math
{
    namespace SSE
    {
        Float32Vector VECTOR_CALL QuaternionMultiply( Float32Vector a, Float32Vector b );

        Float32Vector VECTOR_CALL QuaternionRotate3DVector( Float32Vector vector, Float32Vector quaternion );

        Float32Vector VECTOR_CALL QuaternionConjugate( Float32Vector q );


        // implementations


    }
}