#include "ItemSystem.h"

#include "ItemContainer.h"
#include "EntityAbilities.h"

#include <Math\FloatOperators.h>
#include <Math\MathFunctions.h>

// Define maximum possible distance allowing to pick up an item
float const c_maximum_pickup_distance = 2.0f;


namespace
{
    Math::Float3 GetEntityPosition(EntityID const entity,
                                    IndexedOrientations const & indexed_orientations)
    {
        // Get data index and orientation of entity
        auto entity_orientation_index = indexed_orientations.indices[entity.index];
        auto entity_orientation = indexed_orientations.orientations[entity_orientation_index];

        // Return position
        return entity_orientation.position;
    }
}

namespace Logic
{
    // Store a vector of distances of the active entity to the item_entities,
    // sorted parallel to the latter
    void GetItemDistances(IndexedOrientations const & indexed_orientations,
                    EntityID const active_entity,
                    std::vector<EntityID> const & item_entities,
                    std::vector<float> & item_distances)
    {
        // Reserve space for all item distances
        item_distances.reserve(item_entities.size());

        auto entity_position = GetEntityPosition(active_entity, indexed_orientations);

        // Get positions and store distances for all items
        for (auto & item_entity : item_entities)
        {
            // Get position of current entity
            auto item_position = GetEntityPosition(item_entity, indexed_orientations);

            // Calculate distance
            auto distance_vector = item_position - entity_position;
            auto distance = Math::Norm(distance_vector);

            // Store distance for current item
            item_distances.push_back(distance);
        }
    }

    // returns false if it can't find anything
    bool FindNearbyUnusedWeapon(std::vector<EntityID> const & weapon_entities,
                                EntityWieldingAbilities const & wielding_abilities,
                                std::vector<unsigned> & used_item_indices,
                                std::vector<float> & item_distances,
                                EntityID & found_weapon_entity)
    {
        // Try picking up the ever nearest item, excluding those already wielded,
        // until successful or having checked all possible items
        unsigned checked_entities = 0;
        while (checked_entities < item_distances.size())
        {
            // Get closest item distance value and index
            auto min_element = std::min_element(item_distances.begin(), item_distances.end());
            auto min_index = unsigned(std::distance(item_distances.begin(), min_element));

            // Abort checks if closest item is too far away to pick up
            if( *min_element > c_maximum_pickup_distance )
            {
                return false;
            }

            // Get EntityID of found item
            auto found_item_entity = weapon_entities[min_index];

            // Check whether found item is already being used as a weapon
            auto item_weapon_data_index = GetOptional(wielding_abilities.wielded_entity_to_data, found_item_entity.index);

            // Ensure that the weapon was not previously picked up
            auto weapon_picked_up_already = std::count(used_item_indices.begin(), used_item_indices.end(), min_index);

            // Don't use the weapon if it is currently wielded
            // or was picked up by another entity before
            if (item_weapon_data_index == c_invalid_index
                && weapon_picked_up_already == 0)
            {
                // Mark current item for use and skip remaining checks
                found_weapon_entity = weapon_entities[min_index];
                used_item_indices.push_back(min_index);
                return true;
            }

            // If item is already used as a weapon, check the next-closest
            // item instead, and exclude this one from further checks
            item_distances[min_index] = std::numeric_limits<float>::max();
            ++checked_entities;
        }
        return false;
    }

    // When not wielding a weapon, pick up the nearest one within a certain reach
    // which is not wielded by anyone.
    void PickUpNearestItems(std::vector<EntityID> const & triggered_entities,
                            IndexedOrientations const & indexed_orientations,
                            ItemContainer const & item_container,
                            EntityWieldingAbilities const & wielding_abilities,                            
                            std::vector<EntityID> & picking_entities,
                            std::vector<EntityID> & picked_up_entities)
    {
        // Keep track of the items which were already picked up
        std::vector<unsigned> used_item_indices;

        // Iterate over all triggered entities
        for (auto entity : triggered_entities)
        {
            // Skip entity if already wielding a weapon
            if (GetOptional(wielding_abilities.entity_to_data, entity.index) != c_invalid_index) continue;

            // Otherwise get distances to all items:
            std::vector<float> item_distances;
            GetItemDistances(indexed_orientations,
                            entity,
                            item_container.entities,
                            item_distances);

            // Try to find a nearby unused weapon that could be picked up
            EntityID found_weapon_entity;
            if( FindNearbyUnusedWeapon(item_container.entities,
                                    wielding_abilities,
                                    used_item_indices,
                                    item_distances,
                                    found_weapon_entity ) )
            {
                picking_entities.push_back(entity);
                picked_up_entities.push_back(found_weapon_entity);
            }
        }
    }




    void DropItems( std::vector<EntityID> const & drop_triggered_entities, EntityWieldingAbilities & wielding_abilities, std::vector<EntityID> & dropped_items )
    {
        for( auto & dropping_entity : drop_triggered_entities )
        {
            // Get wielder data of entity
            auto data_index = GetOptional( wielding_abilities.entity_to_data, dropping_entity.index );
            // Ignore if entity is not a wielder
            if( data_index != c_invalid_index )
            {
                dropped_items.push_back( wielding_abilities.wielded_entities[data_index] );
                // Otherwise remove wielding ability of entity
                wielding_abilities.RemoveEntity( dropping_entity );
            }
        }
    }
}