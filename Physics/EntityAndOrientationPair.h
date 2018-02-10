#pragma once

#include <Conventions\Orientation.h>
#include <Conventions\EntityID.h>

namespace Physics
{
    struct EntityAndOrientationPair
    {
        EntityID entity1, entity2;
        Orientation orientation1, orientation2;
    };
}
