#pragma once

#include "Inertia.h"
#include <Utilities\Range.h>

namespace BoundingShapes
{
    struct OrientedBox;
    struct Sphere;
}

namespace Physics
{
    Inertia CreateInertia( 
        Math::Float3 moment_of_inertia_around_each_axis, 
        float mass 
        );

    Inertia CreateSphereInertia( 
        float radius, 
        float mass 
        );

    Inertia CreateInertia( 
        BoundingShapes::Sphere const & sphere, 
        float mass 
        );

    Inertia CreateBoxInertia( 
        Math::Float3 size, 
        float mass 
        );

    Inertia CreateInertia( 
        BoundingShapes::OrientedBox const & box, 
        float mass 
        );

    Inertia CreateInverseBoxInertia( 
        Math::Float3 box_size, 
        float mass 
        );

    Math::Float3 CalculateInertiaAroundAxisForBox( 
        Math::Float3 box_size, 
        float mass 
        );

    Inertia Invert( Inertia i );

    // Also works for the inverse moment of inertia
    Math::Float3x3 RotateMomentOfInertia( 
        Math::Float3x3 moment_of_inertia, 
        const Math::Quaternion& rotation 
        );

    Inertia Add(
        Inertia a,
        Inertia b);

    Inertia TranslateAwayFromCenterOfMass(
        Inertia i, 
        Math::Float3 translation
        );

    void CalculateTotalInertia(
        Range<BoundingShapes::Sphere const *> spheres, 
        float total_mass, 
        Inertia & total_inertia, 
        Math::Float3 & center_of_mass
        );

    void CalculateTotalInertia(
        Range<BoundingShapes::OrientedBox const *> boxes, 
        float total_mass, 
        Inertia & total_inertia_output
        );

    void CalculateTotalInertia(
        Range<BoundingShapes::OrientedBox const *> boxes, 
        float total_mass, 
        Inertia & total_inertia, 
        Math::Float3 & center_of_mass
        );

    void CalculateTotalInertia(
        Range<BoundingShapes::Sphere const *> spheres, 
        Range<BoundingShapes::OrientedBox const *> boxes, 
        float total_mass, 
        Inertia & total_inertia, 
        Math::Float3 & center_of_mass
        );

    void Combine(
        Inertia a,
        Math::Float3 center_of_mass_a, 
        Inertia b, 
        Math::Float3 center_of_mass_b, 
        Inertia & i_out,
        Math::Float3 & center_of_mass_out
        );

    Inertia Combine(
        Inertia const & a, 
        Math::Float3 const & center_of_mass_a, 
        Inertia const & b, 
        Math::Float3 const & center_of_mass_b
        );
}