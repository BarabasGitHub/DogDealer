#pragma once
#include <vector>
#include <Conventions\EntityID.h>
#include <Conventions\EntityTemplateID.h>

#include <Utilities\Range.h>

namespace Logic
{
    struct DeadEntityComponents
    {
        std::vector<EntityID> entity_ids;
        std::vector<EntityTemplateID> template_ids;

        std::vector<uint32_t> entity_to_data;

    };

    void Add( EntityID entity, EntityTemplateID template_id, DeadEntityComponents & self );
    void Remove( EntityID id, DeadEntityComponents & self );
    void Remove( Range<EntityID const *> entities, DeadEntityComponents & self );
}