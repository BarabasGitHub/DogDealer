#pragma once

#include "BodyID.h"
#include <BoundingShapes\OrientedBox.h>
#include <Utilities\Range.h>

#include <vector>

namespace Physics
{
    struct OrientedBoxContainer
    {
        std::vector<BoundingShapes::OrientedBox> boxes;

        std::vector<uint32_t> offsets;
        std::vector<BodyID> bodies;
        std::vector<uint32_t> body_to_offset;

        OrientedBoxContainer();
    };


    void AddBox(BodyID entity, BoundingShapes::OrientedBox box, OrientedBoxContainer & self);
    void AddBoxes(BodyID entity, Range<BoundingShapes::OrientedBox const*> boxes, OrientedBoxContainer & self);
    void Remove(Range<BodyID const*> bodies, OrientedBoxContainer & self);
    bool Contains(BodyID id, OrientedBoxContainer const & self );
}
