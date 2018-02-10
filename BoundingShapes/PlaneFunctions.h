#pragma once

#include "Plane.h"

namespace BoundingShapes
{

    Plane CreatePlane( Math::Float4 plane_equation );
    Plane CreatePlane( Math::Float3 normal, Math::Float3 point );

    float Distance( Plane const & plane, Math::Float3 point );

    Plane Lerp(Plane a, Plane b, float factor);
    Plane Normalize(Plane p);
    Plane Transform(Plane p, Math::Float4x4 const & transform_matrix);

    Math::Float4 Float4FromPlane(Plane const & a);
}