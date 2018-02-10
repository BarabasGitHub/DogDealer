#include "DamageDealersContainer.h"

#include <Utilities\IndexedHelp.h>
#include <Utilities\ContainerHelpers.h>

using namespace Logic;


void Logic::Add(EntityID damage_dealer, float damage, DamageDealersContainer & self)
{
    auto index = uint32_t(self.entities.size());
    self.entities.push_back(damage_dealer);
    self.damage_values.push_back(damage);

    AddIndexToIndices( self.entity_to_data, damage_dealer.index, index );
}


void Logic::Remove(Range<EntityID const*> entities_to_be_removed, DamageDealersContainer & self)
{
    RemoveIDsWithSwapAndPrune(entities_to_be_removed, self.entity_to_data, self.entities, self.damage_values);
}