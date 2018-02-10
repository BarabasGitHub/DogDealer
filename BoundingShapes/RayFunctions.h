#pragma once

#include "Ray.h"
#include <Conventions\Orientation.h>

namespace BoundingShapes
{
    Ray RayFromStartAndDirection(Math::Float3 start, Math::Float3 direction);

    Math::Float3 PointAlongRay(Ray const & ray, float time);

    Ray TransformByOrientation( Ray const & ray, Orientation const & orientation );

    Ray Rotate( Ray const & ray, Math::Quaternion const & rotation );
}



