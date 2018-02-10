#pragma once

#include "BodyID.h"
#include <Utilities\IDGenerator.h>

namespace Physics
{
    typedef IDGenerator<BodyID> BodyIDGenerator;
    extern template BodyIDGenerator;
}
