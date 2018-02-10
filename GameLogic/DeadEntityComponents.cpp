#include "DeadEntityComponents.h"

#include <Utilities\ContainerHelpers.h>
#include <Utilities\IndexedHelp.h>

using namespace Logic;

void Logic::Add( EntityID entity, EntityTemplateID template_id, DeadEntityComponents & self )
{
    assert( GetOptional( self.entity_to_data, entity.index ) == c_invalid_index && "Entity already exists in container.");
    auto index = uint32_t( self.entity_ids.size() );
    self.entity_ids.push_back( entity );
    self.template_ids.push_back( template_id );
    AddIndexToIndices( self.entity_to_data, entity.index, index );
}


void Logic::Remove( EntityID id, DeadEntityComponents & self )
{
    Remove( CreateRange( &id, 1 ), self );
}


void Logic::Remove( Range<EntityID const *> entities, DeadEntityComponents & self )
{
    RemoveIDsWithSwapAndPrune( entities, self.entity_to_data, self.entity_ids, self.template_ids );
}