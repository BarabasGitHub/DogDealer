#pragma once

#include "BodyID.h"
#include <Conventions\Orientation.h>

namespace Physics
{
    struct BodyAndOrientationPair
    {
        BodyID body1, body2;
        Orientation orientation1, orientation2;
    };
}
