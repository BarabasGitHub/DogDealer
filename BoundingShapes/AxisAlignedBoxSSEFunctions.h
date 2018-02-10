#pragma once

#include <Math\SSE.h>
#include <Utilities\MinMax.h>

namespace BoundingShapes
{

    Math::SSE::Float32Vector VECTOR_CALL RotateExtentSSE( Math::SSE::Float32Vector extent, Math::SSE::FloatMatrix const & transform );

    // transforms the given input
    void VECTOR_CALL TransformAxisAlignedBoxSSE( Math::SSE::Float32Vector& center, Math::SSE::Float32Vector& extent , Math::SSE::FloatMatrix const & transform );

    MinMax<Math::SSE::Float32Vector> VECTOR_CALL GetMinMax(Math::SSE::Float32Vector center, Math::SSE::Float32Vector extent);
}
