#include "AnimatingStateMachineContainer.h"

#include <Utilities/IndexedHelp.h>
#include <Utilities/VectorHelper.h>

namespace Logic
{
	void AnimatingStateMachineContainer::AddEntity(EntityID const entity_id)
	{
		auto const index = unsigned(entity_ids.size());
		AddIndexToIndices(entity_to_data, entity_id.index, index);

		entity_is_stationary.push_back(true); 
		entity_is_turning.push_back(false);
		entity_ids.push_back(entity_id);
	}


	void AnimatingStateMachineContainer::RemoveEntity(EntityID const entity_id)
	{
		auto entry_index = GetOptional(entity_to_data, entity_id.index);
		if ( entry_index != unsigned(-1))
		{
			SwapAndPrune(entry_index, entity_ids, entity_is_stationary, entity_is_turning);
			entity_to_data[entity_id.index] = unsigned(-1);
		}

		// Update user of previously last entry to refer to overwritten entry
		auto last_entry_index = entity_ids.size();
		for (auto i = 0; i < entity_to_data.size(); i++)
		{
			if (entity_to_data[i] == last_entry_index)
			{
				entity_to_data[i] = entry_index;
			}
		}		
	}
}
