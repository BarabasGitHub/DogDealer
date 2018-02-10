#include "NonCollidingBodies.h"

#include <Utilities\IndexedHelp.h>
#include <Utilities\VectorHelper.h>

using namespace Physics;



void NonCollidingBodies::AddStaticComponent( EntityID entity_id, Orientation orientation )
{
    auto insert_index = kinematic_body_start_index;

    entity_ids.emplace( entity_ids.begin() + insert_index, entity_id );
    orientations.emplace( orientations.begin() + insert_index, orientation );
    previous_orientations.emplace( previous_orientations.begin() + insert_index, orientation );

    for( auto & index : entity_to_element )
    {
        if( index >= insert_index && index != c_invalid_index )
        {
            ++index;
        }
    }

    AddIndexToIndices( entity_to_element, entity_id.index, insert_index );

    assert( orientations.size() == entity_ids.size() );
    assert( previous_orientations.size() == entity_ids.size() );
    ++kinematic_body_start_index;
}


void NonCollidingBodies::AddKinematicComponent( EntityID entity_id, Orientation orientation )
{
    auto insert_index = uint32_t( entity_ids.size() );

    entity_ids.emplace( entity_ids.begin() + insert_index, entity_id );
    orientations.emplace( orientations.begin() + insert_index, orientation );
    previous_orientations.emplace( previous_orientations.begin() + insert_index, orientation );

    for( auto & index : entity_to_element )
    {
        if( index >= insert_index && index != c_invalid_index )
        {
            ++index;
        }
    }

    AddIndexToIndices( entity_to_element, entity_id.index, insert_index );

    assert( orientations.size() == entity_ids.size() );
    assert( previous_orientations.size() == entity_ids.size() );

}



void NonCollidingBodies::RemoveEntities( Range<EntityID const *> entity_ids_to_be_removed )
{
    auto indices = RemoveIndices( entity_to_element, entity_ids_to_be_removed );

    std::sort( begin( indices ), end( indices ) );

    RemoveEntries( entity_ids, indices );
    RemoveEntries( orientations, indices );
    RemoveEntries( previous_orientations, indices );

    auto bound = std::lower_bound( begin( indices ), end( indices ), kinematic_body_start_index );
    kinematic_body_start_index -= uint32_t(bound - begin( indices ));
}