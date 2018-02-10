#pragma once

#include <Conventions\EntityID.h>
#include <Conventions\EntityTemplateID.h>
#include <Utilities\IndexedHelp.h>
#include <Utilities\VectorHelper.h>


#include <vector>

namespace Logic{

    struct ItemContainer
    {
        std::vector<EntityID> entities;
        std::vector<unsigned> entity_to_data;

        std::vector<EntityTemplateID> equipped_template_ids;
        std::vector<EntityTemplateID> dropped_template_ids;

        void RemoveEntity( EntityID const item_entity );
        void RemoveEntities( Range<EntityID const *> item_entities );
        void AddToItems( EntityID const item_entity, EntityTemplateID equipped_template, EntityTemplateID dropped_template );
        bool EntityIsItem( EntityID id ) const;
    };
}