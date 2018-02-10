#include "AIControllerSystem.h"

#include "EntityAbilities.h"
#include "EntityAnimations.h"

#include "AINavigation.h"

#include "TimerSystem.h"
#include "AIParameterContainer.h"

#include <Conventions\Velocity.h>

#include <Conventions\Orientation.h>
#include <Conventions\OrientationFunctions.h>

#include <Utilities\VectorHelper.h>
#include <Utilities\StdVectorFunctions.h>

#include <Math\MathFunctions.h>

// For random striking directions
#include <random>
#include <time.h>


namespace Logic{


    // Recalculate paths consisting of only one node or no longer leading close enough
    // to their respective target entities.
    // Create paths for those following ai entities that have none yet.
    void UpdateFollowingAINavigation(IndexedOrientations const & indexed_orientations,
                                    AIParameterContainer & parameters,
                                    NavigationMesh const & navigation_mesh)
    {
        // Recalculate a path if the target entity moved further from its destination
        float const c_maximum_destination_target_offset = 2.0f;

        auto following_entity_count = parameters.m_following.m_entities.size();

        // Get vector of all following ai entity positions
        std::vector<Math::Float3> entity_positions(following_entity_count);
        GetEntityPositions(parameters.m_following.m_entities, indexed_orientations, entity_positions);

        // Get parallel vector of their target entity positions
        std::vector<Math::Float3> target_entity_positions(following_entity_count);
        GetEntityPositions(parameters.m_following.m_target_entities, indexed_orientations, target_entity_positions);

		std::vector<unsigned> pathless_followers;
		     
        // Iterate over all entities until finding the first one with no valid path.
        // Check if the target moved away too far from the last path node, so that
        // the path must be updated,
        for(auto i = 0; i < following_entity_count; i++)
        {
            // Get path for current entity
            auto const & path = parameters.m_following.m_paths[i];

            // Break once the first entity with no path is encountered, skipping to the next loop
			if (path.empty())
			{ 
				pathless_followers.push_back(i);
				continue;
			}

            // Check whether a recalculation is necessary
            bool recalculate = false;

            if(path.size() == 1)
            {
                // Recalculate if the entity is immediately moving towards the target
                recalculate = true;
            }
            else
            {
                // Otherwise check whether the target moved away far enough from the
                // final node of the path to make a recalculation necessary
                auto path_destination = path.back();

                // Get target entity position in 2d
                auto target_position = target_entity_positions[i];
                auto target_position_2d = Math::Float2(target_position.x, target_position.y);

                // Get distance between path destination and target
                auto distance = Math::Norm(path_destination - target_position_2d);

                if(distance > c_maximum_destination_target_offset) recalculate = true;
            }

            // TODO: It might not be necessary to recalculate the entire path
            if(recalculate)
            {
                // Get ai entity
                auto entity = parameters.m_following.m_entities[i];

                // Get ai entity position
                auto entity_position = entity_positions[i];
                auto target_position = target_entity_positions[i];

                auto new_path = FindPath(entity_position, target_position, navigation_mesh);

                // Store the index of the path for the ai entity
                parameters.m_following.m_paths[i] = new_path;
				parameters.m_following.m_next_node_indices[i] = 0;
            }
        }

		// Continue with those entities which have no valid path
		for (auto i : pathless_followers)
		{			       
            auto entity = parameters.m_following.m_entities[i];

            // Get 3D positions for both entities
            auto entity_position = entity_positions[i];
            auto target_entity_position = target_entity_positions[i];

            // Find a path from entity to target
            auto path = FindPath(entity_position, target_entity_position, navigation_mesh);

            // Store the index of the path for the ai entity
            parameters.m_following.m_paths[i] = path;
			parameters.m_following.m_next_node_indices[i] = 0;
        }
    }


	void InitializeNewPatrollingPaths(IndexedOrientations const & indexed_orientations,
									AIParameterContainer & parameters,
									NavigationMesh const & navigation_mesh)
	{
		auto patrolling_entity_count = parameters.m_navigating.m_entities.size();

		// Get vector of all following ai entity positions
		std::vector<Math::Float3> entity_positions(patrolling_entity_count);
		GetEntityPositions(parameters.m_navigating.m_entities, indexed_orientations, entity_positions);

		for (int i = 0; i < patrolling_entity_count; i++)
		{			
			if (parameters.m_navigating.m_paths[i].empty())
			{		
				// Get AI entity and target entity
				auto entity = parameters.m_navigating.m_entities[i];

				// Get 3D positions for both entities
				auto entity_position = entity_positions[i];
				auto target_position = parameters.m_navigating.m_target_positions[i];

				// Find a path from entity to target
				auto path = FindPath(entity_position, target_position, navigation_mesh);

				// Store the index of the path for the ai entity
				parameters.m_navigating.m_paths[i] = path;
				parameters.m_navigating.m_next_node_indices[i] = 0;
			}
		}
	}


    // Pick next node in each path if close enough
    void UpdateAIPathNextNodes(IndexedOrientations const & indexed_orientations,
                               std::vector<EntityID> const & entities,
							   std::vector<std::vector<Math::Float2>> const & paths,
							   std::vector<unsigned> & next_node_indices)
    {
        // A node counts as passed if an entity gets as close as  this
        float const c_node_passing_offset = 0.3f;

        std::vector<Math::Float3> entity_positions(entities.size());
        GetEntityPositions(entities, indexed_orientations, entity_positions);

        // Iterate over navigating entities
        for( auto i = 0; i < entities.size(); i++)
        {
            // Get position and path of current entity
            auto entity_position = entity_positions[i];
            auto const & current_path = paths[i];

            // Get index and position of the node the entity is currently walking towards
            auto& next_node_index = next_node_indices[i];

            auto const next_node_position = current_path[next_node_index];

            // Get offset of entity to node
            auto offset = Math::Norm(Math::Float2(entity_position.x, entity_position.y) - next_node_position);

            // Check if entity has passed the node by getting sufficiently close
            if(offset < c_node_passing_offset)
            {
                // Set successive node as new target for entity
                // only if it is not the last node
                if(next_node_index < current_path.size() - 1) next_node_index++;

                // TODO: Otherwise?
            }
        }
    }


    // Make entities walk towards a path node while keeping the minimum and maximum following
    // distance to their target.
    void ApproachTargetsAlongPathNodes(std::vector<EntityID> const & entities,
        IndexedOrientations const & indexed_orientations,
        std::vector<Math::Float2> const & movement_positions,
        std::vector<Math::Float3> const & target_entity_positions,
        std::vector<float> const & minimum_combat_distances,
        std::vector<float> const & maximum_combat_distances,
        std::vector<EntityID> & entities_output,
        std::vector<Math::Float2> & horizontal_movement_output)
    {
        assert(Size(entities) == Size(movement_positions));

        // add entities to output
        Append(entities_output, entities);

        auto entity_count = Size(entities);
        // Reserve space for output
        auto horizontal_movement_output_range = Grow(horizontal_movement_output, entity_count);

        // Loop over AI-controlled entities
        for (auto i = 0u; i < entity_count; i++)
        {
            // Get entity and desired offset from target
            auto entity = entities[i];
            auto minimum_combat_distance = minimum_combat_distances[i];
            auto maximum_combat_distance = maximum_combat_distances[i];

            // Get entity orientation
            auto orientation_index = indexed_orientations.indices[entity.index];
            auto orientation = indexed_orientations.orientations[orientation_index];

            // Get node and target position
            auto node_position_2D = movement_positions[i];
			auto node_position = Math::Float3(node_position_2D.x, node_position_2D.y, 0.0f);
            auto target_position = target_entity_positions[i];

            // Get vector towards node
            auto movement_vector = node_position - orientation.position;
            movement_vector = InverseRotate(movement_vector, orientation.rotation);

            // Get vector towards target
            auto distance_vector = target_position - orientation.position;
            distance_vector = InverseRotate(distance_vector, orientation.rotation);

            // Walk towards node while keeping distance from target
            auto squared_distance = SquaredNorm(distance_vector);
            Math::Float2 movement;
            if (squared_distance <= (minimum_combat_distance * minimum_combat_distance))
            {
                movement = Normalize(-Math::Float2(movement_vector.x, movement_vector.y));
            }
            else if (squared_distance >= (maximum_combat_distance * maximum_combat_distance))
            {
                movement = Normalize(Math::Float2(movement_vector.x, movement_vector.y));
            }
            else
            {
                movement = 0;
            }

            horizontal_movement_output_range[i] = movement;
        }
    }


	// For each following entity, get its next and second next node position.
	// If no second next node exists, use the value of the original next node
	// as a convention.
	void GetNextTwoPathNodes(std::vector<EntityID> const & entities,
                        std::vector<unsigned> const & next_node_indices,
                        std::vector<std::vector<Math::Float2>> const & paths,
						std::vector<Math::Float2> & next_nodes,
						std::vector<Math::Float2> & second_next_nodes)
	{
		auto following_ai_count = entities.size();
		next_nodes.resize(following_ai_count);
		second_next_nodes.resize(following_ai_count);

		// Get next nodes for all following ai entities
		for (auto i = 0; i < following_ai_count; i++)
		{
			// Get navigation path for entity
			auto& path = paths[i];

			// Get next node position
			auto next_node_index = next_node_indices[i];
			auto node_a = path[next_node_index];

			// Walk directly to the next node
			next_nodes[i] = node_a;

			// Get second next node, if it exists
			if (next_node_index + 1 < path.size())
			{
				auto node_b = path[next_node_index + 1];
				second_next_nodes[i] = node_b;
			}
			else
			{
				// Otherwise, set the second node to the first node as a convention
				second_next_nodes[i] = next_nodes[i];
			}
		}
	}


	// Simply set the next node as target
	void DetermineNextMovementPositions(std::vector<Math::Float2> const & next_nodes,
										std::vector<Math::Float2> & next_movement_positions)
	{
		Append(next_movement_positions, next_nodes);
	}


	// Face towards the next path node. When getting close to it, start facing
	// a mixture of the node and its successor for smooth turning
	void DetermineNextFacingPositions(std::vector<Math::Float3> const & ai_entity_positions,
									std::vector<Math::Float2> const & next_nodes,
									std::vector<Math::Float2> const & second_next_nodes,
									std::vector<Math::Float2> & next_facing_positions)
	{
		auto following_ai_count = ai_entity_positions.size();
		next_facing_positions.resize(following_ai_count);

		// How closely a next node must be for the entity
		// to also start turning a bit towards its successor
		float const c_node_rotation_blending_radius = 0.5f;

		// Highest possible weight the second next node
		// may achieve in the blending
		float const c_node_rotation_blending_maximum = 0.5;

		// Get next nodes for all following ai entities
		for (auto i = 0; i < following_ai_count; i++)
		{
			// Get both path nodes for the current entity
			auto next_node = next_nodes[i];
			auto second_next_node = second_next_nodes[i];

			// If no second node exists (by convention it is then identical to the next node)
			if (next_node == second_next_node)
			{
				// No blending can be done anyway, so
				// face next node directly without blending
				next_facing_positions[i] = next_node;
				continue;
			}

			// Otherwise Get entity position in 2D
			auto entity_position_3D = ai_entity_positions[i];
			auto entity_position = Math::Float2(entity_position_3D.x, entity_position_3D.y);

			// Get 2D distance between entity and next node
			auto distance = Math::Norm(entity_position - next_node);

			// Check if any blending should be done
			if (distance > c_node_rotation_blending_radius)
			{
				// Face next node directly without blending
				next_facing_positions[i] = next_node;
			}
			else
			{
				// Otherwise determine blend weight and apply maximum influence factor for second next node
				auto blend_weight = 1 - (distance / c_node_rotation_blending_radius);
				blend_weight *= c_node_rotation_blending_maximum;

				auto blended_position_x = Math::Lerp(next_node.x, second_next_node.x, blend_weight);
				auto blended_position_y = Math::Lerp(next_node.y, second_next_node.y, blend_weight);

				next_facing_positions[i] = Math::Float2(blended_position_x, blended_position_y);
			}
		}
	}


	// Slow down the entities depending on
	//	-the estimated time until arrival at the next path node
	//  -the angle between the current and next path segment
	void GenerateNextMovementSpeeds(std::vector<Math::Float3> const & ai_entity_positions,
									std::vector<Velocity> const & ai_entity_velocities,
									std::vector<Math::Float2> const & next_nodes,
									std::vector<Math::Float2> const & second_next_nodes,
									// output
									std::vector<float> & speed_factors)
	{
		// Keep all previously defined entries
		auto speed_write_offset = speed_factors.size();

		auto following_ai_count = ai_entity_positions.size();
		speed_factors.resize(speed_write_offset + following_ai_count, 1.0f);

		// Time to arrival at a next node at which a deceleration is first considered
		float const c_travel_time_deceleration_start = 1.0f;

		// The extent to which these calculations may lessen the speed
		float const c_speed_blend_factor = 0.9f; // was 0.75

		for (auto i = 0; i < following_ai_count; i++)
		{
			// Get entity position and velocity
			auto entity_position_3D = ai_entity_positions[i];
			auto entity_position = Math::Float2(entity_position_3D.x, entity_position_3D.y);

			// Get entity velocity and next node position
			auto entity_velocity = ai_entity_velocities[i];
			auto next_node = next_nodes[i];

			// Determine current distance to next node
			auto distance = next_node - entity_position;
			auto distance_3D = Math::Float3(distance.x, distance.y, 0.0f);

			// Approximate time until node is reached
			auto projected_velocity = Math::Dot(entity_velocity, Math::Normalize(distance_3D));

			// If not moving yet, then no deceleration is necessary
			if (projected_velocity == 0.0f)
			{
				speed_factors[speed_write_offset + i] = 1.0f;
				continue;
			}

			auto travel_time = Math::Norm(distance) / projected_velocity;

			// If it will take a longer time, just go full speed
			if (travel_time > c_travel_time_deceleration_start)
			{
				speed_factors[speed_write_offset + i] = 1.0f;
			}
			else
			{
				// Otherwise, check for the need of a sudden turn upon arrival
				// TODO: The result of this check could actually be stored as part of the path

				// Current path segment is the vector between player and next node
				auto current_segment = distance_3D;

				//
				auto second_next_node = second_next_nodes[i];

				// Check if no second segment exists
				// (nodes are then identical by convention)
				if (next_node == second_next_node)
				{
					speed_factors[speed_write_offset + i] = 1.0f;
					continue;
				}

				// Get angle between segments
				auto next_segment = second_next_node - next_node;
				auto segment_angle_z = Math::ZRotationBetweenAxes(current_segment, Math::Float3(next_segment.x, next_segment.y, 0.0f));

				// Convert the angle to a factor between 0 and 1
				auto angle_factor = 1 - std::abs(segment_angle_z) / float(Math::c_PI);

				// Weigh the time until arrival and the angle between the segments
				auto result_speed = travel_time * angle_factor;

				result_speed = Math::Lerp(speed_factors[speed_write_offset + i], result_speed, c_speed_blend_factor);

				result_speed = Math::Clamp(0.0f, 1.0f, result_speed);
				speed_factors[speed_write_offset + i] = result_speed;
			}
		}
	}


	// Store movement and rotation vectors for all AI entities following other entities.
    // Follow the navigation paths while keeping the minimum offset from the target entity.
    void GetFollowingAIActions(IndexedOrientations const & indexed_orientations,
							IndexedVelocities const & indexed_velocities,
                            AIParameterContainer const & parameters,
                            // output
                            std::vector<EntityID> & moving_ai_entity_ids,
                            std::vector<Math::Float2> & ai_movement,
                            std::vector<EntityID> & target_angle_entity_ids,
                            std::vector<float> & target_z_angles,
							std::vector<float> & target_speed_factors)
    {
		// Get positions for all following target entities
		std::vector<Math::Float3> target_entity_positions(parameters.m_following.m_target_entities.size());
		GetEntityPositions(parameters.m_following.m_target_entities, indexed_orientations, target_entity_positions);

		// Get positions for all ai navigation entities
		std::vector<Math::Float3> ai_entity_positions(parameters.m_following.m_entities.size());
		GetEntityPositions(parameters.m_following.m_entities, indexed_orientations, ai_entity_positions);

		// Get velocities for all ai navigation entities
		std::vector<Velocity> ai_entity_velocities(parameters.m_following.m_entities.size());
		GetEntityVelocities(parameters.m_following.m_entities, indexed_velocities, ai_entity_velocities);

		// Get the next two nodes on the current path for all navigating entities
		std::vector<Math::Float2> next_nodes, second_next_nodes;
		
	    GetNextTwoPathNodes( parameters.m_following.m_entities,
                             parameters.m_following.m_next_node_indices,
                             parameters.m_following.m_paths,
                             next_nodes,
                             second_next_nodes);

		// Determine what point to move towards
		std::vector<Math::Float2> next_movement_positions;
		DetermineNextMovementPositions(next_nodes, next_movement_positions);

		// Determine what point to rotate towards
		std::vector<Math::Float2> next_facing_positions;
		DetermineNextFacingPositions(ai_entity_positions, next_nodes, second_next_nodes, next_facing_positions);

		// Determine which speed to move at
		GenerateNextMovementSpeeds(ai_entity_positions, ai_entity_velocities, next_nodes, second_next_nodes, target_speed_factors);

		// Following-specific
        ApproachTargetsAlongPathNodes(
            parameters.m_following.m_entities,
            indexed_orientations,
			next_movement_positions,
            target_entity_positions,
            parameters.m_following.m_minimum_distances,
            parameters.m_following.m_maximum_distances,
            moving_ai_entity_ids,
            ai_movement);

        TurnTowardsTargetPositions(
            parameters.m_following.m_entities,
            indexed_orientations,
			next_facing_positions,
            target_angle_entity_ids,
            target_z_angles);
    }


    void AdvanceAlongPathNodes(std::vector<EntityID> const & entities,
                            IndexedOrientations const & indexed_orientations,
                            std::vector<Math::Float2> const & movement_positions,
                            std::vector<EntityID> & entities_output,
                            std::vector<Math::Float2> & horizontal_movement_output)
    {
        assert(Size(entities) == Size(movement_positions));

        // add entities to output
        Append(entities_output, entities);

        auto entity_count = Size(entities);
        // Reserve space for output
        auto horizontal_movement_output_range = Grow(horizontal_movement_output, entity_count);

        // Loop over AI-controlled entities
        for (auto i = 0u; i < entity_count; i++)
        {
            // Get entity and desired offset from target
            auto entity = entities[i];

            // Get entity orientation
            auto orientation_index = indexed_orientations.indices[entity.index];
            auto orientation = indexed_orientations.orientations[orientation_index];

            // Get node and target position
            auto node_position_2D = movement_positions[i];
			auto node_position = Math::Float3(node_position_2D.x, node_position_2D.y, 0.0f);

            // Get vector towards node
            auto movement_vector = node_position - orientation.position;
            movement_vector = InverseRotate(movement_vector, orientation.rotation);

            horizontal_movement_output_range[i] = Normalize(Math::Float2(movement_vector.x, movement_vector.y));
        }
    }



    // ##### THIS ONE IS SPECIFICALLY FOR NON-FOLLOWING AI ENTITIES:
	// Store movement and rotation vectors for all AI entities following other entities.
    // Follow the navigation paths while keeping the minimum offset from the target entity.
    void GetPatrollingAIActions(IndexedOrientations const & indexed_orientations,
							IndexedVelocities const & indexed_velocities,
                            AIParameterContainer const & parameters,
                            // output
                            std::vector<EntityID> & moving_ai_entity_ids,
                            std::vector<Math::Float2> & ai_movement,
                            std::vector<EntityID> & target_angle_entity_ids,
                            std::vector<float> & target_z_angles,
							std::vector<float> & target_speed_factors)
    {
		// Get positions for all ai navigation entities
		std::vector<Math::Float3> ai_entity_positions(parameters.m_navigating.m_entities.size());
		GetEntityPositions(parameters.m_navigating.m_entities, indexed_orientations, ai_entity_positions);

		// Get velocities for all ai navigation entities
		std::vector<Velocity> ai_entity_velocities(parameters.m_navigating.m_entities.size());
		GetEntityVelocities(parameters.m_navigating.m_entities, indexed_velocities, ai_entity_velocities);

		// Get the next two nodes on the current path for all navigating entities
		std::vector<Math::Float2> next_nodes, second_next_nodes;
	    
        GetNextTwoPathNodes( parameters.m_navigating.m_entities,
                             parameters.m_navigating.m_next_node_indices,
                             parameters.m_navigating.m_paths,
                             next_nodes,
                             second_next_nodes);

		// Determine what point to move towards
		std::vector<Math::Float2> next_movement_positions;
		DetermineNextMovementPositions(next_nodes, next_movement_positions);

		// Determine what point to rotate towards
		std::vector<Math::Float2> next_facing_positions;
		DetermineNextFacingPositions(ai_entity_positions, next_nodes, second_next_nodes, next_facing_positions);

		// Determine which speed to move at
		GenerateNextMovementSpeeds(ai_entity_positions, ai_entity_velocities, next_nodes, second_next_nodes, target_speed_factors);

        AdvanceAlongPathNodes(parameters.m_navigating.m_entities,
                             indexed_orientations,
                             next_movement_positions,
                             moving_ai_entity_ids,
                             ai_movement);

        TurnTowardsTargetPositions(
            parameters.m_navigating.m_entities,
            indexed_orientations,
			next_facing_positions,
            target_angle_entity_ids,
            target_z_angles);
    }



    // Fill MeleeActionTriggers for all AI entities close enough to their target to strike
    void GetAggressionAIActions(IndexedOrientations const & indexed_orientations,
                                AIParameterContainer const & parameters,
                                // output
                                MeleeActionTriggers & melee_action_triggers)
    {
        std::vector<Math::Float3> aggression_target_positions(parameters.m_aggression.m_target_entities.size());
        GetEntityPositions(parameters.m_aggression.m_target_entities, indexed_orientations, aggression_target_positions);

        // Store those entities that are in melee range already
        std::vector<EntityID> melee_entities;
        GetEntitiesWithingTargetDistance(
            parameters.m_aggression.m_entities,
            indexed_orientations,
            aggression_target_positions,
            parameters.m_aggression.m_maximum_distances,
            melee_entities);

        // WIELDING
        Append(melee_action_triggers.strike_readying_entities, melee_entities);

        // Generate random attack directions for all triggered ai entities
        auto needed_attack_directions = unsigned(melee_entities.size());
        AppendRandomAttackDirections(melee_action_triggers.strike_readying_directions, needed_attack_directions);

        // Mark all AI entities for potential releasing of a swing
        Append(melee_action_triggers.strike_releasing_entities, parameters.m_aggression.m_entities);
    }


    void UpdateAIControllers(IndexedOrientations const & indexed_orientations,
						IndexedVelocities const & indexed_velocities,
                        AIParameterContainer & parameters,
                        // output
                        std::vector<EntityID> & moving_ai_entity_ids,
                        std::vector<Math::Float2> & ai_movement,
                        std::vector<EntityID> & target_angle_entity_ids,
                        std::vector<float> & target_z_angles,
						std::vector<float> & target_speed_factors,
                        MeleeActionTriggers & melee_action_triggers,
                        NavigationMesh const & navigation_mesh)
    {
        // Keep all passive AI entities in position
        Append(moving_ai_entity_ids, parameters.m_ai_passive_entities);
        Append(ai_movement, Math::Float2(0,0), parameters.m_ai_passive_entities.size());
        // and facing the same way
        Append(target_angle_entity_ids, parameters.m_ai_passive_entities);
        auto z_angles = Grow(target_z_angles, Size(parameters.m_ai_passive_entities));
        GetEntityZAngles(parameters.m_ai_passive_entities, indexed_orientations, z_angles);
        Append(target_speed_factors, 1.0f, Size(parameters.m_ai_passive_entities));

        // Update following paths and generate un-initialized ones
        UpdateFollowingAINavigation(indexed_orientations, parameters, navigation_mesh);
		UpdateAIPathNextNodes(indexed_orientations,
			parameters.m_following.m_entities,
			parameters.m_following.m_paths,
			parameters.m_following.m_next_node_indices);

		InitializeNewPatrollingPaths(indexed_orientations, parameters, navigation_mesh);
		UpdateAIPathNextNodes(indexed_orientations,
							  parameters.m_navigating.m_entities,
							  parameters.m_navigating.m_paths,
							  parameters.m_navigating.m_next_node_indices);

        // Generate movement output for following AI
        GetFollowingAIActions(indexed_orientations,
							  indexed_velocities,
                              parameters,
                              moving_ai_entity_ids,
                              ai_movement,
                              target_angle_entity_ids,
                              target_z_angles,
							  target_speed_factors);

	
        // #####
        // Do the same thing for patrolling AI
        std::vector<EntityID> target_angle_entity_ids_patrolling;
        std::vector<float> target_z_angles_patrolling;
        std::vector<float> target_speed_factors_patrolling;
        GetPatrollingAIActions(indexed_orientations,
							  indexed_velocities,
                              parameters,
                              moving_ai_entity_ids,
                              ai_movement,
                              target_angle_entity_ids_patrolling,
                              target_z_angles_patrolling,
							  target_speed_factors_patrolling);

        // Merge results
        Append(target_angle_entity_ids, target_angle_entity_ids_patrolling); 
        Append(target_z_angles, target_z_angles_patrolling);
        Append(target_speed_factors, target_speed_factors_patrolling);

        // #####
        // Generate melee actions for aggression AI
        GetAggressionAIActions(indexed_orientations,
                               parameters,
                               melee_action_triggers);

    }


    // Append #count randomly generated attack directions to the input vector
    void AppendRandomAttackDirections(std::vector<Math::Float2>& attack_directions,
                                    unsigned const count)
    {
        // Reserve space for the new directions
        auto current_size = attack_directions.size();
        auto result_size = current_size + count;
        attack_directions.reserve(result_size);

        // Seed random generator
        srand(unsigned(time(NULL)));

        // Generate and push back directions
        for (auto i = 0u; i < count; i++)
        {
            auto direction = rand() % 4 + 1;

            switch (direction)
            {
            case 1:
                attack_directions.push_back({ -1.0f, 0.0f });
                break;
            case 2:
                attack_directions.push_back({ 1.0f, 0.0f });
                break;
            case 3:
                attack_directions.push_back({ 0.0f, -1.0f });
                break;
            case 4:
                attack_directions.push_back({ 0.0f, 1.0f });
                break;
            }
        }
    }





    // Fill the horizontal movement output vector with Float 2
    // parallel to the input entities.
    // Use -1.0, 0.0 or 1.0 for relative movement towards the target entity
    void WalkTowardsTargetPositions(std::vector<EntityID> const & entities,
                                    IndexedOrientations const & indexed_orientations,
                                    std::vector<Math::Float3> const & target_positions,
                                    std::vector<float> const & minimum_combat_distances,
                                    std::vector<float> const & maximum_combat_distances,
                                    std::vector<EntityID> & entities_output,
                                    std::vector<Math::Float2> & horizontal_movement_output )
    {
        assert( Size( entities ) == Size( target_positions ) );

        // Reserve space for output
        auto old_size = Size( horizontal_movement_output );
        horizontal_movement_output.resize( old_size + entities.size());
        auto horizontal_movement_output_range = CreateRange( horizontal_movement_output, old_size, Size(horizontal_movement_output) );
        // add entities to output
        Append( entities_output, entities );

        // Loop over AI-controlled entities
        for (auto i = 0u; i < entities.size(); i++)
        {
            // Get entity and desired offset from target
            auto entity = entities[i];
            auto minimum_combat_distance = minimum_combat_distances[i];
            auto maximum_combat_distance = maximum_combat_distances[i];

            // Get entity orientation
            auto orientation_index = indexed_orientations.indices[entity.index];
            auto orientation = indexed_orientations.orientations[orientation_index];

            // Get target position
            auto target_position = target_positions[i];

            // Get vector towards target
            auto distance_vector = target_position - orientation.position;
            distance_vector = InverseRotate( distance_vector, orientation.rotation );

            auto squared_distance = SquaredNorm(distance_vector);
            Math::Float2 movement;
            if(squared_distance <= (minimum_combat_distance * minimum_combat_distance))
            {
                movement = Normalize( -Math::Float2(distance_vector.x, distance_vector.y) );
            }
            else if(squared_distance >= (maximum_combat_distance * maximum_combat_distance))
            {
                movement = Normalize( Math::Float2(distance_vector.x, distance_vector.y) );
            }
            else
            {
                movement = 0;
            }

            horizontal_movement_output_range[i] = movement;
        }
    }


    void TurnTowardsTargetPositions(std::vector<EntityID> const & entities,
                                    IndexedOrientations const & indexed_orientations,
                                    std::vector<Math::Float2> const & target_positions,
                                    std::vector<EntityID> & entities_output,
                                    std::vector<float> & target_z_angles )
    {
        // Reserve space for output
        auto old_size = Size( target_z_angles );
        target_z_angles.resize(old_size + entities.size());
        auto target_z_angles_range = CreateRange( target_z_angles, old_size, Size(target_z_angles) );
        // add entities to output
        Append( entities_output, entities );

        // Loop over AI-controlled entities
        for (auto i = 0u; i < entities.size(); i++)
        {
            // Get entity
            auto entity = entities[i];

            // Get entity orientation
            auto orientation_index = indexed_orientations.indices[entity.index];
            auto& orientation = indexed_orientations.orientations[orientation_index];

            // Get target position
            auto target_position_2D = target_positions[i];
			auto target_position = Math::Float3(target_position_2D.x, target_position_2D.y, 0.0f);

            // Get vector towards target and needed offset from default rotation
            auto distance_vector = target_position - orientation.position;

			// Problem: When following a path and overshooting a path node,
			//			AI entities may turn backwards to face them with extreme torque
            auto rotational_offset_z = Math::ZRotationBetweenAxes(Math::Float3(0.0f, 1.0f, 0.0f), distance_vector);

            target_z_angles_range[i] = rotational_offset_z;
        }
    }

    // Fill entities_in_range with the ids of all those entities
    // closer to their targets than their maximum distance
    void GetEntitiesWithingTargetDistance(
        std::vector<EntityID> const & entities,
        IndexedOrientations const & indexed_orientations,
        std::vector<Math::Float3> const & target_positions,
        std::vector<float> const & maximum_target_distances,
        std::vector<EntityID> & entities_in_range
        )
    {
        assert( Size( entities ) == Size( target_positions ) );

        auto count = entities.size();
        // Loop over AI-controlled entities
        for (auto i = 0u; i < count; i++)
        {
            // Get entity and maximum distance to target
            auto entity = entities[i];
            auto maximum_distance = maximum_target_distances[i];

            // Get entity orientation
            auto orientation_index = indexed_orientations.indices[entity.index];
            auto orientation = indexed_orientations.orientations[orientation_index];

            // Get target position
            auto target_position = target_positions[i];

            // Get vector towards target
            auto squared_distance = SquaredNorm(target_position - orientation.position);

            if( squared_distance <= (maximum_distance * maximum_distance) )
            {
                entities_in_range.push_back(entity);
            }
        }
    }
}
