#pragma once
#include <Conventions\Orientation.h>

#include <vector>

namespace Logic{

    struct ItemContainer;
    struct EntityWieldingAbilities;

    struct WeaponWieldingParameters
    {
        EntityID wielder_entity_id = c_invalid_entity_id;
        EntityID wielded_entity_id;
        unsigned bone_index;
        Orientation weapon_offset;
    };

    void PickUpNearestItems(std::vector<EntityID> const & triggered_entities,
                            IndexedOrientations const & indexed_orientations,
                            ItemContainer const & item_container,
                            EntityWieldingAbilities const & wielding_abilities,
                            std::vector<EntityID> & picking_entities,
                            std::vector<EntityID> & picked_up_entities);


    void DropItems( std::vector<EntityID> const & drop_triggered_entities, EntityWieldingAbilities & wielding_abilities, std::vector<EntityID> & dropped_items );
}