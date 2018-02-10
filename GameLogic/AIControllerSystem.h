#pragma once

#include "Structures.h"

#include <Conventions\Force.h>
#include <Conventions\AnimatingInstructions.h>


struct IndexedOrientations;
struct IndexedVelocities;
struct AIParameterContainer;

namespace Logic{

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
                            NavigationMesh const & navigation_mesh);


    void AppendRandomAttackDirections(std::vector<Math::Float2>& attack_directions, 
                                    unsigned const count);

    void UpdateAIPathNextNodes(IndexedOrientations const & indexed_orientations,
								std::vector<EntityID> const & entities,
								std::vector<std::vector<Math::Float2>> const & paths,
								std::vector<unsigned> & next_node_indices);

    void WalkTowardsTargetPositions(std::vector<EntityID> const & entities,
                                IndexedOrientations const & indexed_orientations,
                                std::vector<Math::Float3> const & target_positions,
                                std::vector<float> const & maximum_combat_distances,
                                std::vector<float> const & minimum_combat_distances,
                                std::vector<EntityID> & entities_output,
                                std::vector<Math::Float2> & horizontal_movement_output );

    void TurnTowardsTargetPositions(std::vector<EntityID> const & entities,
                                IndexedOrientations const & indexed_orientations,
                                std::vector<Math::Float2> const & target_positions,
                                std::vector<EntityID> & entities_output,
                                std::vector<float> & target_z_angles);


    // stores the entities within a certain distance from their target
    void GetEntitiesWithingTargetDistance(std::vector<EntityID> const & entities,
                                    IndexedOrientations const & indexed_orientations,
                                    std::vector<Math::Float3> const & target_positions,
                                    std::vector<float> const & minimum_target_distances,
                                    std::vector<EntityID> & entities_in_range);
}
