#include "MovingEntities.h"

#include <Utilities\IndexedHelp.h>
#include <Utilities\VectorHelper.h>

void Physics::MovingEntities::AddComponent( EntityID entity_id, Velocity velocity, Math::Float3 angular_velocity )
{
    auto index = uint32_t( entity_ids.size() );
    entity_ids.emplace_back( entity_id );
    velocities.emplace_back( velocity );
    angular_velocities.emplace_back( angular_velocity );

    AddIndexToIndices( entity_to_element, entity_id.index, index );
}


void Physics::MovingEntities::RemoveEntities( Range<EntityID const *> entity_ids_to_be_removed )
{
    auto indices = RemoveIndices( entity_to_element, entity_ids_to_be_removed );

    std::sort( begin( indices ), end( indices ) );

    RemoveEntries( entity_ids, indices );
    RemoveEntries( velocities, indices );
    RemoveEntries( angular_velocities, indices );
}