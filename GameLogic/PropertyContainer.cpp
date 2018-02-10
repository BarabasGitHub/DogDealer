#include "PropertyContainer.h"

#include <Utilities\IndexedHelp.h>
#include <Utilities\VectorHelper.h>
#include <Utilities\ContainerHelpers.h>

using namespace Logic;

void PropertyContainer::AddComponent(EntityID const entity_id, 
									 float const motion_power,  
									 float const torque, 
									 Math::Float2 const motion_bias_positive, 
									 Math::Float2 const motion_bias_negative, 
									 float const target_speed)
{
    auto const index = unsigned( entity_ids.size() );
	entity_ids.push_back(entity_id);
    AddIndexToIndices( entity_to_property, entity_id.index, index );

	motion_powers.push_back( motion_power );
	torques.push_back( torque );

	motion_biases_positive.push_back(motion_bias_positive);
	motion_biases_negative.push_back(motion_bias_negative);

    motion_target_speeds.push_back(target_speed);
}

// Remove all property entries associated with the input entity
void PropertyContainer::RemoveEntity(EntityID const entity_id)
{
    RemoveEntities(CreateRange(&entity_id, 1));
}


void PropertyContainer::RemoveEntities(Range<EntityID const*> entities_to_be_removed)
{
    RemoveIDsWithSwapAndPrune(entities_to_be_removed, entity_to_property, entity_ids, motion_powers, motion_target_speeds, torques, motion_biases_positive, motion_biases_negative);

}


bool PropertyContainer::Contains(EntityID const id) const
{
    return GetOptional(entity_to_property, id.index) != c_invalid_index;
}

// NUMERIC:
void NumericPropertyContainer::AddComponent(EntityID const entity_id, float const entity_max_hitpoints)
{
    auto const index = unsigned(entity_ids.size());
    entity_ids.push_back(entity_id);
    AddIndexToIndices(entity_to_property, entity_id.index, index);

    maximum_hitpoints.push_back(entity_max_hitpoints);
    current_hitpoints.push_back(entity_max_hitpoints);
}


// Remove all property entries associated with the input entity
void NumericPropertyContainer::RemoveEntity(EntityID const entity_id)
{
    RemoveEntities(CreateRange(&entity_id, 1));
}


void NumericPropertyContainer::RemoveEntities(Range<EntityID const*> entities_to_be_removed)
{
    RemoveIDsWithSwapAndPrune(entities_to_be_removed, entity_to_property, entity_ids, maximum_hitpoints, current_hitpoints);
}