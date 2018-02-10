#pragma once

#include <Conventions\Velocity.h>
#include <Conventions\EntityID.h>

#include <Utilities\Range.h>

namespace Physics
{

    struct MovingEntities
    {

        std::vector<EntityID> entity_ids;
        std::vector<uint32_t> entity_to_element;

        std::vector<Velocity> velocities;
        std::vector<Math::Float3> angular_velocities;

        void AddComponent( EntityID entity_id, Velocity velocity, Math::Float3 angular_velocity );

        void RemoveEntities( Range<EntityID const *> entity_ids_to_be_removed );
    };
}