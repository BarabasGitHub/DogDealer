#pragma once

#include "DensityFunction.h"

#include <Math\FloatTypes.h>

#include <BoundingShapes\Ray.h>
#include <BoundingShapes\AxisAlignedBox.h>
#include <BoundingShapes\OrientedBox.h>
#include <BoundingShapes\Sphere.h>

#include <Utilities\Range.h>

namespace Physics
{

    float IntersectionTime(BoundingShapes::Ray const & ray, DensityFunctionType const & density_function, float accuracy);

    float IntersectionTime(BoundingShapes::Ray const & ray, BoundingShapes::AxisAlignedBox const & box);

    float IntersectionTime(BoundingShapes::Ray const & ray, BoundingShapes::OrientedBox const & box);

    float IntersectionTime(BoundingShapes::Ray const & ray, BoundingShapes::Sphere const & box);

    float IntersectionTime(BoundingShapes::Ray const & ray, Range<BoundingShapes::OrientedBox const *> boxes);

    float IntersectionTime(BoundingShapes::Ray const & ray, Range<BoundingShapes::Sphere const *> spheres);

    Math::Float3 IntersectionPoint(BoundingShapes::Ray const & ray, DensityFunctionType const & density_function, float accuracy);

    Math::Float3 IntersectionPoint(BoundingShapes::Ray const & ray, BoundingShapes::AxisAlignedBox const & box);

    Math::Float3 IntersectionPoint(BoundingShapes::Ray const & ray, BoundingShapes::OrientedBox const & box);

    Math::Float3 IntersectionPoint(BoundingShapes::Ray const & ray, Range<BoundingShapes::OrientedBox const *> boxes);

    Math::Float3 IntersectionPoint(BoundingShapes::Ray const & ray, Range<BoundingShapes::Sphere const *> spheres);

}
