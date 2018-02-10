#pragma once
#include <Conventions\EntityID.h>

#include <vector>
namespace Logic
{
	struct AnimatingStateMachineContainer
	{	
		std::vector<EntityID> entity_ids;
		std::vector<unsigned> entity_to_data;

		// Use very simple implicit turdlery because I'm lazy
		std::vector<uint8_t> entity_is_stationary;
		std::vector<uint8_t> entity_is_turning;

		void AddEntity(EntityID const entity_id);
		void RemoveEntity(EntityID const entity_id);
	};
}