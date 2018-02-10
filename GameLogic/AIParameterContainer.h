#pragma once

#include <Conventions\EntityID.h>
#include <Conventions\Orientation.h>

#include <vector>

namespace Logic{

    struct AIParameterContainer
    {
        // Parallel vectors for entities moving towards other entities.
        // Used for combat movement and non-aggressive following
        struct Following
        {
            std::vector<EntityID> m_entities;
            std::vector<EntityID> m_target_entities;

            // Get no closer or further than
            std::vector<float> m_minimum_distances;
            std::vector<float> m_maximum_distances;

			// Index of the node the entity is currently walking towards.
			// The index is relative to the path start
			std::vector<unsigned> m_next_node_indices;

			std::vector<std::vector<Math::Float2>> m_paths;
        };
        Following m_following;
        
        // Parallel vectors for attacking other entities when in range.
        // Entails no movement.
        struct Aggression
        {
            std::vector<EntityID> m_entities;
            std::vector<EntityID> m_target_entities;

            // Only attack when closer than
            std::vector<float> m_maximum_distances;
        };
        Aggression m_aggression;
        

        // Temporary container for walking entities, separate from Following AI
        // TODO: Outsource the pathfinding functionality from following AI 
        //       and merge with this
        struct navigating
        {
            std::vector<EntityID> m_entities;    
            std::vector<Math::Float3> m_target_positions;

			// Index of the node the entity is currently walking towards.
			// The index is relative to the path start
			std::vector<unsigned> m_next_node_indices;

			std::vector<std::vector<Math::Float2>> m_paths;
        };
        navigating m_navigating;        


        // These entities resist basic forces and attempt to stay in position
        std::vector<EntityID> m_ai_passive_entities;  


        void AddAggressionTarget(EntityID const aggressor_entity, 
                                EntityID const target_entity, 
                                float maximum_combat_distance);
        
        void AddFollowingTarget(EntityID const following_entity, 
                                EntityID const target_entity, 
                                float following_minimum_distance,
                                float following_maximum_distance);
        /*
        void AddNavigationPath(EntityID const entity,
                            std::vector<Math::Float2> & path);
							*/

        void AddPatrollingTarget(EntityID const entity_id,
                                 Math::Float3 const target);
                                
        
        void RemoveFollowingAI(EntityID const entity_id);


        void AddPassiveAIEntity(EntityID const entity);


		// Remove all entries for the input entity
		void RemoveEntity(EntityID const entity_id);
		void RemoveEntityFromActiveAIRoles(EntityID const entity_id);

        void RemovePassiveAI(EntityID const entity);

		/*
		void RemoveEntityFromNavigation(EntityID const entity_id);
		*/ 

		void RemoveEntityFromPatrolling(EntityID const entity_id);

		void RemoveEntityAsAggressor(EntityID const entity_id);
		void RemoveEntityFromAggression(EntityID const entity_id);

		void RemoveEntityAsFollower(EntityID const entity_id);
		void RemoveEntityFromFollowing(EntityID const entity_id);
    };
}
