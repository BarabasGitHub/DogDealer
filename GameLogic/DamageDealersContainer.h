#pragma once

#include <Conventions\EntityID.h>
#include <Utilities\Range.h>
#include <vector>

namespace Logic
{
    struct DamageDealersContainer
    {
        std::vector<uint32_t> entity_to_data;
        std::vector<EntityID> entities;
        std::vector<float> damage_values;
    };


    void Add(EntityID damage_dealer, float damage, DamageDealersContainer & self);
    void Remove(Range<EntityID const*> entities_to_be_removed, DamageDealersContainer & self);
}