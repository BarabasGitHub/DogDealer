#include "AIParameterContainer.h"

#include <Utilities\VectorHelper.h>
#include <Utilities\IndexedHelp.h>

using namespace Logic;

// Remove all aggression entries where the input entity is aggressor or target
void AIParameterContainer::RemoveEntity( EntityID const entity_id )
{
	RemoveEntityFromAggression(entity_id);

	RemoveEntityFromFollowing(entity_id);

	RemoveEntityFromPatrolling(entity_id);
	
	//RemoveEntityFromNavigation(entity_id);	
  
    RemovePassiveAI(entity_id);
}

void AIParameterContainer::RemoveEntityFromActiveAIRoles(EntityID const entity_id)
{
	RemoveEntityAsAggressor(entity_id);

	RemoveEntityAsFollower(entity_id);

	RemoveEntityFromPatrolling(entity_id);

	//RemoveEntityFromNavigation(entity_id);

	RemovePassiveAI(entity_id);
}

void AIParameterContainer::AddAggressionTarget(EntityID const aggressor_entity, 
                                            EntityID const target_entity, 
                                            float maximum_combat_distance)
{
    m_aggression.m_entities.push_back( aggressor_entity );
    m_aggression.m_target_entities.push_back( target_entity );

    m_aggression.m_maximum_distances.push_back( maximum_combat_distance );
}
        
void AIParameterContainer::AddFollowingTarget(EntityID const following_entity, 
                                            EntityID const target_entity, 
                                            float following_minimum_distance,
                                            float following_maximum_distance)
{
    // Only allow following a single entity at a time
    assert(std::find(begin(m_following.m_entities), end(m_following.m_entities), following_entity) == end(m_following.m_entities));
    
    m_following.m_entities.push_back( following_entity );
    m_following.m_target_entities.push_back( target_entity );

    m_following.m_minimum_distances.push_back( following_minimum_distance );
    m_following.m_maximum_distances.push_back( following_maximum_distance );
	
	// Store yet invalid path index
	m_following.m_next_node_indices.push_back(unsigned(-1));
	m_following.m_paths.push_back(std::vector<Math::Float2>());
}

/*
void AIParameterContainer::AddNavigationPath(EntityID const entity,
                                            std::vector<Math::Float2> & path)
{       
    assert(std::find(begin(m_navigation.m_entities), end(m_navigation.m_entities), entity) == end(m_navigation.m_entities)
		&& "Attempting to add duplicate entry for navigation AI");

    auto index = uint32_t( m_navigation.m_entities.size() );
    AddIndexToIndices( m_navigation.m_entity_to_data, entity.index, index );

    m_navigation.m_entities.push_back(entity);
    m_navigation.m_paths.push_back(path);
    
    // begin by walking towards the first path node
    m_navigation.m_next_node_indices.push_back(0);  
}
*/

void AIParameterContainer::AddPassiveAIEntity(EntityID const entity)
{
    m_ai_passive_entities.push_back(entity);
}



void AIParameterContainer::AddPatrollingTarget(EntityID const entity_id, Math::Float3 const target)
{
    m_navigating.m_entities.push_back(entity_id);
	m_navigating.m_target_positions.push_back(target);
	
	// Store yet invalid path index
	m_navigating.m_next_node_indices.push_back(unsigned(-1));
	m_navigating.m_paths.push_back(std::vector<Math::Float2>());
}



void AIParameterContainer::RemoveFollowingAI(EntityID const entity_id)
{
    auto entry_index = 0u;

    // Iterate over all passive entries
    while (entry_index < m_following.m_entities.size())
    {
        auto current_entity = m_following.m_entities[entry_index];

        // Check whether to-be-removed entity is involved
        if (current_entity == entity_id)
        {
            // Remove entry by swapping with vector end and resizing
            // and inspect swapped entry at same index in next iteration
            SwapAndPrune(entry_index, 
						 m_following.m_entities, 
						 m_following.m_maximum_distances, 
						 m_following.m_minimum_distances, 
						 m_following.m_target_entities,
						 m_following.m_next_node_indices,
						 m_following.m_paths);
        }
        else
        {
            // Otherwise inspect next entry
            ++entry_index;
        }
    }
}

/*
// TODO: This one is structurally awful since it works on multiple containers
//	     which depend on the navigation simultaneously
void AIParameterContainer::RemoveEntityFromNavigation(EntityID const entity_id)
{
    auto entry_index = 0u;

    // Iterate over all passive entries
    while (entry_index < m_navigation.m_entities.size())
    {
        auto current_entity = m_navigation.m_entities[entry_index];

        // Check whether to-be-removed entity is involved
        if (current_entity == entity_id)
        {

			if (entry_index != m_navigation.m_entities.size() - 1)
			{
				// Update external references in following or patrolling AI:
				// Find the entry that references the last navigation index
				unsigned max_path_index = unsigned(-1);
				unsigned max_path_user_index = unsigned(-1);
				bool max_path_user_is_following_ai = true;

				// Following
				for (auto i = 0; i < m_following.m_entities.size(); i++)
				{
					// Get navigation index used by current following entity
					auto navigation_index = m_following.m_path_indices[i];

					// Ignore invalid entries
					if (navigation_index == unsigned(-1)) continue;

					if (navigation_index > max_path_index || max_path_index == unsigned(-1))
					{
						max_path_index = navigation_index;
						max_path_user_index = i;
					}
				}

				// Patrolling
				for (auto i = 0; i < m_patrolling.m_entities.size(); i++)
				{
					// Get navigation index used by current following entity
					auto navigation_index = m_patrolling.m_path_indices[i];

					// Ignore invalid entries
					if (navigation_index == unsigned(-1)) continue;

					if (navigation_index > max_path_index || max_path_index == unsigned(-1))
					{
						max_path_index = navigation_index;
						max_path_user_index = i;

						max_path_user_is_following_ai = false;
					}
				}

				// This may occur when all existing following or patrolling entities
				// have no valid path yet
				EntityID max_user_entity = c_invalid_entity_id;
				if (max_path_user_index != unsigned(-1))
				{
					// Update the user of the to-be-moved entry to reference the one to be freed
					if (max_path_user_is_following_ai)
					{
						m_following.m_path_indices[max_path_user_index] = entry_index;
						max_user_entity = m_following.m_entities[max_path_user_index];
					}
					else
					{
						m_patrolling.m_path_indices[max_path_user_index] = entry_index;
						max_user_entity = m_patrolling.m_entities[max_path_user_index];
					}
				} 

				m_navigation.m_entity_to_data[max_user_entity.index] = entry_index;
			}
		
			// Remove entry by swapping with vector end and resizing
			// and inspect swapped entry at same index in next iteration
			SwapAndPrune(entry_index, m_navigation.m_entities, m_navigation.m_next_node_indices, m_navigation.m_paths);				

			m_navigation.m_entity_to_data[entity_id.index] = unsigned(-1);
        }
        else
        {
            // Otherwise inspect next entry
            ++entry_index;
        }
    }	
}
*/

void AIParameterContainer::RemovePassiveAI(EntityID const entity_id)
{
    auto entry_index = 0u;

    // Iterate over all passive entries
    while (entry_index < m_ai_passive_entities.size())
    {
        auto current_entity = m_ai_passive_entities[entry_index];

        // Check whether to-be-removed entity is involved
        if (current_entity == entity_id)
        {
            // Remove entry by swapping with vector end and resizing
            // and inspect swapped entry at same index in next iteration
            SwapAndPrune(entry_index, m_ai_passive_entities);
        }
        else
        {
            // Otherwise inspect next entry
            ++entry_index;
        }
    }
}


void AIParameterContainer::RemoveEntityFromPatrolling(EntityID const entity_id)
{
	auto entry_index = 0;
	while (entry_index < m_navigating.m_entities.size())
	{
		auto current_entity = m_navigating.m_entities[entry_index];
		// Check whether to-be-removed entity is involved
		if ((current_entity == entity_id))
		{
			// Remove entry by swapping with vector end and resizing
			// and inspect swapped entry at same index in next iteration
			SwapAndPrune(entry_index, 
						 m_navigating.m_entities, 
						 m_navigating.m_target_positions,
						 m_navigating.m_next_node_indices,
						 m_navigating.m_paths);
		}
		else
		{
			// Otherwise inspect next entry
			++entry_index;
		}
	}
}

// Remove all AI aggression entries where input entity is aggressor
void AIParameterContainer::RemoveEntityAsAggressor(EntityID const entity_id)
{
	auto entry_index = 0u;
	while (entry_index < m_aggression.m_entities.size())
	{
		auto current_aggressor = m_aggression.m_entities[entry_index];

		// Check whether to-be-removed entity is involved
		if (current_aggressor == entity_id)
		{
			// Remove entry by swapping with vector end and resizing
			// and inspect swapped entry at same index in next iteration
			SwapAndPrune(entry_index, m_aggression.m_entities, m_aggression.m_target_entities, m_aggression.m_maximum_distances);
		}
		else
		{
			// Otherwise inspect next entry
			++entry_index;
		}
	}
}


// Remove all AI aggression entries where input entity is either target or aggressor
void AIParameterContainer::RemoveEntityFromAggression(EntityID const entity_id)
{
	auto entry_index = 0u;
	while (entry_index < m_aggression.m_entities.size())
	{
		auto current_aggressor = m_aggression.m_entities[entry_index];
		auto current_target = m_aggression.m_target_entities[entry_index];

		// Check whether to-be-removed entity is involved
		if ((current_aggressor == entity_id)
			|| (current_target == entity_id))
		{
			// Remove entry by swapping with vector end and resizing
			// and inspect swapped entry at same index in next iteration
			SwapAndPrune(entry_index, m_aggression.m_entities, m_aggression.m_target_entities, m_aggression.m_maximum_distances);
		}
		else
		{
			// Otherwise inspect next entry
			++entry_index;
		}
	}
}


void AIParameterContainer::RemoveEntityAsFollower(EntityID const entity_id)
{
	auto entry_index = 0;
	while (entry_index < m_following.m_entities.size())
	{
		auto current_follower = m_following.m_entities[entry_index];
		// Check whether to-be-removed entity is involved
		if (current_follower == entity_id)
		{
			// Remove entry by swapping with vector end and resizing
			// and inspect swapped entry at same index in next iteration
			SwapAndPrune(entry_index, 
						 m_following.m_entities, 
						 m_following.m_target_entities, 	
						 m_following.m_minimum_distances, 
						 m_following.m_maximum_distances,
						 m_following.m_next_node_indices,
						 m_following.m_paths);
		}
		else
		{
			// Otherwise inspect next entry
			++entry_index;
		}
	}
}

void AIParameterContainer::RemoveEntityFromFollowing(EntityID const entity_id)
{
	auto entry_index = 0;
	while (entry_index < m_following.m_entities.size())
	{
		auto current_follower = m_following.m_entities[entry_index];
		auto current_target = m_following.m_target_entities[entry_index];

		// Check whether to-be-removed entity is involved
		if ((current_follower == entity_id)
			|| (current_target == entity_id))
		{
			// Remove entry by swapping with vector end and resizing
			// and inspect swapped entry at same index in next iteration
			SwapAndPrune(entry_index, 
						 m_following.m_entities, 
						 m_following.m_target_entities, 						 
						 m_following.m_minimum_distances, 
						 m_following.m_maximum_distances,
						 m_following.m_next_node_indices,
						 m_following.m_paths);
		}
		else
		{
			// Otherwise inspect next entry
			++entry_index;
		}
	}
}