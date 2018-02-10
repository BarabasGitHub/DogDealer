#include "AxisAlignedBoxSSEFunctions.h"

using namespace Math::SSE;

namespace BoundingShapes
{
    // see AxisAlignedBox RotateAroundCenter( AxisAlignedBox box, Math::Float3x3 rotation )
    // assumes either the last element of extent is zero or the last elements of the rows of the rotation matrix (or both)
    Math::SSE::Float32Vector VECTOR_CALL RotateExtentSSE( Math::SSE::Float32Vector extent, Math::SSE::FloatMatrix const & rotation )
    {
        using namespace Math::SSE;
        Math::SSE::FloatMatrix matrix;
        matrix.row[0] = Abs( rotation.row[0] );
        matrix.row[1] = Abs( rotation.row[1] );
        matrix.row[2] = Abs( rotation.row[2] );
        matrix.row[3] = ZeroFloat32Vector();
        return Math::SSE::Multiply3D(matrix, extent);
    }


    void VECTOR_CALL TransformAxisAlignedBoxSSE( Math::SSE::Float32Vector& center, Math::SSE::Float32Vector& extent, Math::SSE::FloatMatrix const & transform )
    {
        center = Math::SSE::TransformPosition3D( center, transform );

        extent = RotateExtentSSE( extent, transform );
    }


    MinMax<Math::SSE::Float32Vector> VECTOR_CALL GetMinMax(Math::SSE::Float32Vector center, Math::SSE::Float32Vector extent)
    {
        return{ Subtract(center, extent), Add(center, extent) };
    }
}
