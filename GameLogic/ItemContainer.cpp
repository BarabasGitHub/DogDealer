#include "ItemContainer.h"

#include <Utilities\VectorHelper.h>
#include <Utilities\ContainerHelpers.h>

namespace Logic
{
    void ItemContainer::RemoveEntity(EntityID const item_entity)
    {
        RemoveEntities(CreateRange(&item_entity, 1));
    }


    void ItemContainer::RemoveEntities( Range<EntityID const *> item_entities )
    {
        RemoveIDsWithSwapAndPrune(item_entities, entity_to_data, entities, dropped_template_ids, equipped_template_ids);
    }


    // Store the input entity in the ItemContainer, provided that it is not stored already
    void ItemContainer::AddToItems( EntityID const item_entity, EntityTemplateID equipped_template, EntityTemplateID dropped_template )
    {
        assert(GetOptional(entity_to_data, item_entity.index) == c_invalid_index);

        auto index = uint32_t(entities.size());
        entities.emplace_back(item_entity);
        dropped_template_ids.emplace_back( dropped_template );
        equipped_template_ids.push_back( equipped_template );

        AddIndexToIndices(entity_to_data, item_entity.index, index );
    }


    bool ItemContainer::EntityIsItem( EntityID id ) const
    {
        return GetOptional( entity_to_data, id.index ) < entities.size();
    }
}