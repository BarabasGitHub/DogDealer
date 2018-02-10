#pragma once

#include "Sphere.h"

#include <Conventions\Orientation.h>

#include <Utilities\Range.h>


namespace BoundingShapes
{

    Sphere CreateSphere( Range<Math::Float3 const *> points );
    /// create a sphere with only the points that are referenced by the indices
    Sphere CreateSphere( Range<Math::Float3 const *> points, Range<unsigned const *> indices );

    Sphere Transform( Sphere sphere, Math::Float4x4 const & transform );
    Sphere TransformByOrientation( Sphere sphere, Orientation const & orientation );

    Sphere Rotate( Sphere sphere, Math::Quaternion const & rotation );

    float Volume(Sphere const & s);
}
