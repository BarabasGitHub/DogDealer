#pragma once

#include "AxisAlignedBox.h"
#include "OrientedBox.h"

namespace BoundingShapes
{
    AxisAlignedBox const & AxisAlignedBoxPartOfOrientedBox(OrientedBox const & box);
}


namespace BoundingShapes
{
    inline AxisAlignedBox const & AxisAlignedBoxPartOfOrientedBox(OrientedBox const & box)
    {
        return reinterpret_cast<AxisAlignedBox const &>(box);
    }
}
