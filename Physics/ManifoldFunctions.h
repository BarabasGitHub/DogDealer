#pragma once

#include "DensityFunction.h"

#include <BoundingShapes\AxisAlignedBox.h>
#include <BoundingShapes\OrientedBox.h>
#include <BoundingShapes\Sphere.h>
#include <BoundingShapes\SphereHierarchyMesh.h>
#include <BoundingShapes\AxisAlignedBoxHierarchyMesh.h>
#include <BoundingShapes\Triangle.h>

#include <Conventions\CollisionManifold.h>

#include <Utilities\Range.h>

namespace Physics
{

    Manifold CreateManifold( BoundingShapes::Sphere sphere1, BoundingShapes::Sphere sphere2 );
    Manifold CreateManifold( BoundingShapes::Sphere sphere, BoundingShapes::AxisAlignedBox const & box );
    Manifold CreateManifold( BoundingShapes::Sphere sphere, BoundingShapes::OrientedBox const & box );
    Manifold CreateManifold( BoundingShapes::Sphere sphere, DensityFunctionType const & sample_function );
    Manifold CreateManifold( BoundingShapes::AxisAlignedBox box1, BoundingShapes::AxisAlignedBox box2 );
    Manifold CreateManifold( BoundingShapes::AxisAlignedBox const & box, BoundingShapes::Triangle const & triangle );
    Manifold CreateManifold( BoundingShapes::OrientedBox box1, BoundingShapes::OrientedBox box2 );
    Manifold CreateManifold( BoundingShapes::OrientedBox const & box, BoundingShapes::Triangle const & triangle );

    Manifold CreateManifold( BoundingShapes::OrientedBox const & box, DensityFunctionType const & sample_function );

	Manifold CreateManifold( BoundingShapes::OrientedBox const & box, BoundingShapes::SphereHierarchyMesh const & mesh );
    Manifold CreateManifold( BoundingShapes::OrientedBox const & box, BoundingShapes::AxisAlignedBoxHierarchyMesh const & mesh );

    // the input gets destroyed!
    Manifold CreateManifold(Range<float *> penetration_depths, Range<Math::Float3 *> separation_axes, Range<Math::Float3 *> positions, Range<uint8_t *> ages);

    Manifold MergeManifolds( Manifold const & a, Manifold const & b );

    void AgeManifolds(Range<Manifold *> manifolds, uint8_t expire_age_threshold);

    void UpdatePenetrationDepth(Range<Manifold *> manifolds, float resolved_fraction);
}