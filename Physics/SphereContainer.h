#pragma once

#include "BodyID.h"
#include <BoundingShapes\Sphere.h>
#include <Utilities\Range.h>

#include <vector>

namespace Physics
{
    struct SphereContainer
    {
        std::vector<BoundingShapes::Sphere> spheres;

        std::vector<uint32_t> offsets;
        std::vector<BodyID> bodies;
        std::vector<uint32_t> body_to_offset;

        SphereContainer();
    };


    void AddSphere(BodyID body, BoundingShapes::Sphere box, SphereContainer & self);
    void AddSpheres(BodyID body, Range<BoundingShapes::Sphere const*> boxes, SphereContainer & self);
    void Remove(Range<BodyID const*> bodies, SphereContainer & self);
    bool Contains(BodyID id, SphereContainer const & self );
}
