#pragma once

#include "BodyID.h"

#include <Conventions\EntityID.h>

#include <vector>

namespace Physics
{
    // keeps track of what entities all bodies belong to
    struct BodyEntityMapping
    {
        // contains which entity each body belongs to
        std::vector<EntityID> entity_ids;

        // maps the EntityID to the start and end index of the bodies in entity_bodies
        // so entity_id has bodies (entity_to_bodies[entity_id.index], entity_to_bodies[entity_id.index]] in entity_bodies
        std::vector<uint32_t> entity_to_bodies {{0}};
        // all the bodies of the entities, pointed to by entity_to_bodies
        std::vector<BodyID> entity_bodies;
    };
}
