#pragma once
#include "MovementSystem.h"

#include <Math\MathFunctions.h>

#include <Utilities\ParallelIterator.h>
#include <Utilities\Range.h>
#include <Utilities\VectorHelper.h>

#include <algorithm>

namespace Logic
{
    unsigned SeparateIdleFromMovingEntities( Range<EntityID *> entities, IndexedVelocities const & velocities, float speed_threshold )
    {
        auto partition_point = std::partition( begin( entities ), end( entities ),
                                               [&velocities, speed_threshold]
        ( EntityID a )
        {
            auto velocity = GetOptionalVelocity( velocities, a, 0 );
            auto speed = Norm( velocity );
            return speed < speed_threshold;
        } );

        return unsigned( partition_point - begin( entities ) );
    }


    uint32_t ProcessMovement( Range<EntityID const *> new_entities, std::vector<EntityID> & entities, unsigned const moving_entities_start_index,
                              IndexedVelocities const & velocities, float const start_moving_speed_threshold, float const stop_moving_speed_threshold,
                              Range<EntityID const *> & now_idle_entities, Range<EntityID const *> & now_moving_entities )
    {
        // get the existing idle entities
        auto idle_entities = CreateRange( entities, 0, moving_entities_start_index );

        // separate out the moving entities
        auto now_moving_entities_start_index = SeparateIdleFromMovingEntities( idle_entities, velocities, start_moving_speed_threshold );

        // store the number of now moving entities
        auto now_moving_entities_count = moving_entities_start_index - now_moving_entities_start_index;

        // get the entities that were moving
        auto old_moving_entities = CreateRange( entities, moving_entities_start_index, Size(entities));

        // separate out the idle entities
        auto now_idle_entities_count = SeparateIdleFromMovingEntities( old_moving_entities, velocities, stop_moving_speed_threshold );

        // adjust the index, so it is relative to the whole vector again
        auto now_idle_entities_end_index = now_idle_entities_count + moving_entities_start_index;

        // swap the new moving and idle entities
        std::rotate( begin( entities ) + now_moving_entities_start_index,
                     begin( entities ) + moving_entities_start_index,
                     begin( entities ) + now_idle_entities_end_index );

        // store where the now idle entities start
        auto now_idle_entities_start_index = now_moving_entities_start_index;

        // update the moving entities start and end index
        now_moving_entities_start_index += now_idle_entities_count;
        // now_idle_entities_end_index = now_moving_entities_start_index; // not used

        // insert the new entities, separating the idle and moving entities automatically
        auto updated_moving_entities_start_index = AddNewEntities( new_entities, entities, now_moving_entities_start_index, velocities, start_moving_speed_threshold );

        // count how many idle entities there are among the new entities
        auto new_idle_entities_count = updated_moving_entities_start_index - now_moving_entities_start_index;

        // count how many moving entities there are among the new entities
        auto new_moving_entities_count = uint32_t( Size( new_entities ) ) - new_idle_entities_count;

        // update the count, start index and end index of the now moving entities
        now_moving_entities_start_index = updated_moving_entities_start_index;
        now_moving_entities_count += new_moving_entities_count;
        now_idle_entities_end_index = now_moving_entities_start_index;

        // get the moving now entities
        now_moving_entities = CreateRange( entities.data() + now_moving_entities_start_index, now_moving_entities_count );

        // get all new idle entities, both the ones that are newly added as the ones that were moving before
        now_idle_entities = CreateRange( entities, now_idle_entities_start_index, now_idle_entities_end_index );

        return now_moving_entities_start_index;
    }


    uint32_t AddNewEntities( Range<EntityID const *> new_entities, std::vector<EntityID> & existing_entities, uint32_t moving_entities_start_index,
                             IndexedVelocities const & velocities, float const speed_threshold )
    {
        auto insertion_point_iterator = begin( existing_entities ) + moving_entities_start_index;
        existing_entities.insert( insertion_point_iterator, begin( new_entities ), end( new_entities ) );

        // recalculate the starting pointer, both because we need an actual pointer and because the vector could have reallocated
        auto inserted_entities = CreateRange( existing_entities.data() + moving_entities_start_index, Size( new_entities ) );

        auto moving_index = SeparateIdleFromMovingEntities( inserted_entities, velocities, speed_threshold );

        // adjust the index because it is relative to the inserted_entities
        return moving_index + moving_entities_start_index;
    }

    void MovementSystem::RemoveEntity( EntityID const & entity_id )
    {
        auto found = std::find( begin( idle_and_moving_entities ), end( idle_and_moving_entities ), entity_id );
        if( found != end( idle_and_moving_entities ) )
        {
            auto i = uint32_t( found - begin( idle_and_moving_entities ) );
            // Check with what kind of entity to swap for removal
            bool moving_entity = ( i >= moving_entities_start_index );

            // If moving, swap with last moving entry, otherwise with last idle entry
            auto swap_index = moving_entity ? uint32_t( idle_and_moving_entities.size() ) : moving_entities_start_index;
            swap_index -= 1;

            // Overwite to-be-removed entry
            idle_and_moving_entities[i] = idle_and_moving_entities[swap_index];

            // If entity was idle, the original last idle entry
            // used to overwrite it is now a duplicate.
            // Remove it by overwriting it in turn with the last moving
            // entity entry and reducing the count of idle entities
            if( !moving_entity )
            {
                SwapAndPrune( swap_index, idle_and_moving_entities );

                // Shift back start index of moving entities
                --moving_entities_start_index;
            }
            else
            {
                // remove the last moving entry which we used to overwrite the to-be-removed entry
                idle_and_moving_entities.resize( idle_and_moving_entities.size() - 1 );
            }
        }
    }
}