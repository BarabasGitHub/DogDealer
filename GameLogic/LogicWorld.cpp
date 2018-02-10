#pragma once
#include "LogicWorld.h"

#include "ResourceManager.h"

#include "AnimatingSystem.h"
#include "AnimatingTriggers.h"

#include "ControllerAlgorithms.h"
#include "AbilityAlgorithms.h"
#include "DamageCalculation.h"
#include "ProjectileHitRegistration.h"
#include "PlayerControllerSystem.h"
#include "AIControllerSystem.h"
#include "MeleeSystem.h"
#include "MeleeHitRegistration.h"
#include "ItemSystem.h"

#include <Math\MathFunctions.h>
#include <Math\VectorAlgorithms.h>

#include <Utilities\DefaultContainerFunctions.h>
#include <Utilities\IntegerRange.h>
#include <Utilities\VectorHelper.h>
#include <Utilities\StdVectorFunctions.h>
#include <Utilities\Logger.h>
#include <Utilities\DogDealerException.h>

#include <Conventions\OrientationFunctions.h>

#include <Conventions\EntityIDGenerator.h>

#include <Input\GameInput.h>

// Temporarily for striking:
#include <random>
#include <time.h>

using namespace Logic;


float const c_start_moving_speed_threshold = 0.5f;
float const c_stop_moving_speed_threshold = 0.15f;


float const c_replace_dying_body_time = 5.5f; // 2.5 was for fridge
float const c_play_dead_animation_time = 1.5f;


LogicWorld::LogicWorld()
{
    // Hardcoded scampage
    //m_resource_manager.ProvideNavigationMesh("navigation_castle", m_navmesh_container);
	//m_resource_manager.ProvideNavigationMesh("navigation_multitower", m_navmesh_container);
	//m_resource_manager.ProvideNavigationMesh("navigation_100x100m_quad", m_navmesh_container);
}


LogicWorld::~LogicWorld()
{ }


void LogicWorld::CreateMobileComponent( EntityID id, MobileComponentDescription const & description)
{
    m_property_container.AddComponent( id,
                                    description.motion_power,
                                    description.rotation_power,
                                    description.motion_bias_positive,
                                    description.motion_bias_negative,
                                    description.target_speed);

    // Store attachment points for entity
    if (!description.weapon_attachment_points.empty())
    {
        // For now: Only use the first attachment point
        m_attachment_point_container.AddEntity(id, description.weapon_attachment_points.front());
    }
}


void LogicWorld::CreateItemComponent( EntityID id, EntityTemplateID equipped_template, EntityTemplateID dropped_template )
{
    m_item_container.AddToItems( id, equipped_template, dropped_template );
}


void LogicWorld::CreateDamageDealerComponent( EntityID id, DamageDealerComponentDescription description )
{
    Add(id, description.damage_value, m_damage_dealers_container);
}


void LogicWorld::RemoveEntities(Range<EntityID const *> entity_ids)
{
    // Remove projectiles
    m_projectile_container.RemoveProjectiles(entity_ids);

    // Loop over entities
    for (auto& entity_id : entity_ids)
    {
		// Remove animations
		RemoveEntityAnimations(entity_id, m_entity_animations);

		m_animating_state_machine_container.RemoveEntity(entity_id);

		// Free camera if target was input entity
		if (m_camera.m_target_entity == entity_id)
        {
            m_camera.ClearTarget();
        }

        Remove( entity_id, m_dead_entity_components);
    }

    // Remove timers for entity
    RemoveEntityTimers(m_entity_dead_timers, entity_ids);
    RemoveEntityTimers(m_entity_perished_timers, entity_ids);
}


void LogicWorld::KillEntities(std::vector<EntityID> const & killed_entities,
							  AnimatingTriggers & animating_triggers)
{
    if (killed_entities.empty()) return;

    Log( [&killed_entities]()
    {
        std::string output = "Killed entities: ";
        for( auto e : killed_entities )
        {
            output += std::to_string( e.index ) + ", ";
        }
        // remove the last ", "
        output.resize( output.size() - 2 );
        return output;
    });

	Append(animating_triggers.killed_entities, killed_entities);

    // Set timers to play the 'Dead' animation for the killed entities
    for (auto& entity : killed_entities)
    {
		m_entity_dead_timers.entity_ids.push_back(entity);
		m_entity_dead_timers.remaining_times.push_back(c_play_dead_animation_time);

        // Set timer for body replacement:
		m_entity_perished_timers.entity_ids.push_back(entity);
		m_entity_perished_timers.remaining_times.push_back(c_replace_dying_body_time);
    }

    SortTimers(m_entity_perished_timers);


    // REMOVING CONTAINER ENTRIES FOR KILLED ENTITY
    // (Some entries need to be kept until the RemoveEntity() call)

	// Remove abilities
	m_entity_wielding_abilities.RemoveEntities(killed_entities);

    // Loop over entities
    for (auto& entity_id : killed_entities)
    {
        // Remove property
        m_property_container.RemoveEntity(entity_id);

        // Remove controllers
        m_controllers.RemoveEntityControllers(entity_id);

		m_ai_parameters.RemoveEntity(entity_id);

        // Remove properties
        m_property_container.RemoveEntity(entity_id);
        m_numeric_property_container.RemoveEntity(entity_id);

        // Remove abilities
        m_entity_throwing_abilities.RemoveEntity(entity_id);

		// Remove movement
		m_movement_system.RemoveEntity(entity_id);

        m_item_container.RemoveEntity( entity_id );
    }

    // Remove throwing timers for entity
    RemoveEntityTimers(m_release_throw_timers, killed_entities);
}


void LogicWorld::SetAmmunition( EntityID id, ThrowProperties properties, EntityTemplateID ammo_template_id )
{
    m_entity_throwing_abilities.Add( id, properties, ammo_template_id );
}


void LogicWorld::SetHitpoints(EntityID const id, float const hitpoints)
{
    m_numeric_property_container.AddComponent(id, hitpoints);
}


bool LogicWorld::HasPositiveHitpoints(EntityID id) const
{
    auto index = GetOptional(m_numeric_property_container.entity_to_property, id.index);
    auto hitpoints = GetOptional<float>(m_numeric_property_container.current_hitpoints, index, 0.f);
    return hitpoints > 0;
}


void LogicWorld::SetAnimations(EntityID entity_id, std::map<AnimationStateType, AnimationTemplateID> animation_map)
{
    // SCAMP / TODO: this should probably happen somewhere else
    m_movement_system.new_entities.push_back(entity_id );
	SetEntityAnimations( m_entity_animations.animation_template_ids , entity_id, move(animation_map));

	m_animating_state_machine_container.AddEntity(entity_id);
}


void LogicWorld::EntityWieldItem(EntityID const wielder_entity_id, EntityID const item_entity_id, AttachmentPoint const attachment_point)
{
    assert( m_item_container.EntityIsItem( item_entity_id ) );

	assert(Size(m_immediate_body_replacements.entity_ids) == Size(m_immediate_body_replacements.template_ids));

	m_immediate_body_replacements.entity_ids.push_back(item_entity_id);

	auto index = m_item_container.entity_to_data[item_entity_id.index];
	auto template_id = m_item_container.equipped_template_ids[index];
	m_immediate_body_replacements.template_ids.push_back(template_id);

    m_entity_wielding_abilities.Add(wielder_entity_id, item_entity_id, attachment_point.bone_index, attachment_point.offset);
}


void LogicWorld::SetAIAggressionTarget(EntityID const ai_entity_id,
                                    EntityID const target_entity_id,
                                    float const maximum_combat_distance)
{
    m_ai_parameters.AddAggressionTarget(ai_entity_id, target_entity_id, maximum_combat_distance);
}


void LogicWorld::SetAIFollowingTarget(EntityID const ai_entity_id,
                                    EntityID const target_entity_id,
                                    float const minimum_following_distance,
                                    float const maximum_following_distance)

{
    // Remove previous passive AI
	m_ai_parameters.RemovePassiveAI(ai_entity_id);

    m_ai_parameters.AddFollowingTarget(ai_entity_id, target_entity_id, minimum_following_distance, maximum_following_distance);
}


void LogicWorld::SetAIPatrollingTarget(EntityID const ai_entity_id, Math::Float3 const target_position)
{
	// Remove previous passive AI
	m_ai_parameters.RemovePassiveAI(ai_entity_id);

	m_ai_parameters.AddPatrollingTarget(ai_entity_id, target_position);

	auto debug = 0;
	debug = 0;
}


void LogicWorld::SetAIToPassive(EntityID const ai_entity_id)
{
	m_ai_parameters.RemoveEntityFromActiveAIRoles(ai_entity_id);

    // Add passive AI
    m_ai_parameters.AddPassiveAIEntity(ai_entity_id);

	auto debug = 0;
	debug = 0;
}


void LogicWorld::KillEntity(EntityID const entity_id)
{
    m_entities_to_be_killed.push_back(entity_id);
}


void LogicWorld::SetAfterDeathComponents( EntityID const entity_id, EntityTemplateID after_death_components_template_id )
{
    Add( entity_id, after_death_components_template_id, m_dead_entity_components );
}


void LogicWorld::CreateControllerComponent(EntityID entity_id)
{
    m_controllers.AddArrowKeyController( entity_id );
}


void LogicWorld::CreateForceControllerComponent( EntityID entity_id )
{
    if(m_property_container.Contains(entity_id))
    {
        m_controllers.AddMotionKeyForceController( entity_id );
    }
    else
    {
        throw DogDealerException("Can't add force controller for entity, because it has no power property.", false);
    }
}


void LogicWorld::FollowCameraRotationkinematic( EntityID entity_id )
{
    m_controllers.AddkinematicCameraRotationController( entity_id );
}


void LogicWorld::FollowCameraRotationWithTorque( EntityID entity_id)
{
    if(m_property_container.Contains(entity_id))
    {
        m_controllers.AddTorqueCameraRotationController( entity_id );
    }
    else
    {
        throw DogDealerException("Can't create camera following controller for entity, because it has no power property.", false);
    }

    // SCAMPAGE: Do this here instead of at spawning, so that the AI-entities are not affected
    m_uninitialized_entities.push_back(entity_id);
}


void LogicWorld::CreateThrowController( EntityID entity_id )
{
    m_controllers.AddThrowController( entity_id );
}


void LogicWorld::CreateStrikeController(EntityID entity_id)
{
    m_controllers.AddStrikeController(entity_id);
}


void LogicWorld::TriggerKeyActions(
    GameInput const & game_input,
    IndexedOrientations const & indexed_orientations,
    // output
    std::vector<EntityID> & moving_ids,
    std::vector<Math::Float2> & movements,
    std::vector<EntityID> & target_angle_entity_ids,
    std::vector<float> & target_z_angles,
    std::vector<EntityID> & kinematic_moving_ids,
    std::vector<Math::Float2> & kinematic_movements,
    std::vector<EntityID> & kinematic_target_angle_entity_ids,
    std::vector<float> & kinematic_target_z_angles,
    std::vector<EntityID> & entities_that_want_to_jump,
    MeleeActionTriggers & melee_action_triggers,
    std::vector<EntityID> & entities_that_want_to_throw,
    std::vector<EntityID> & entities_that_want_to_pick_up_an_item,
    std::vector<EntityID> & entities_that_want_to_drop_an_item
    )
{
    // Player Movement
    auto const & user_movement = game_input.movement;

    GeneratePlayerMovement(m_controllers.m_motion_key_force_controllers, user_movement,
        // output
        moving_ids,
        movements );

    GeneratePlayerMovement( m_controllers.m_arrow_key_controllers, user_movement,
                            // output
                            kinematic_moving_ids,
                            kinematic_movements );

    bool keep_player_orientation = false;
    for(auto game_event : game_input.events)
    {
        switch(game_event)
        {
            case GameInputEvent::Jump:
                Append(entities_that_want_to_jump, m_controllers.m_motion_key_force_controllers);
                Append(entities_that_want_to_jump, m_controllers.m_arrow_key_controllers);
                break;
            case GameInputEvent::LookAround:
                keep_player_orientation = true;
                break;
            case GameInputEvent::DropItem:
                // TODO: Use other controller
                Append( entities_that_want_to_drop_an_item, m_controllers.m_torque_camera_rotation_controllers );
                break;
            case GameInputEvent::GrabItem:
                // TODO: Use more reasonable controller
                Append(entities_that_want_to_pick_up_an_item, m_controllers.m_torque_camera_rotation_controllers);
                break;
            case GameInputEvent::Strike:
                // if we aren't going to block
                if(std::find(begin(game_input.events), end(game_input.events), GameInputEvent::Block) == end(game_input.events))
                {
                    GenerateMeleeStrikes(m_controllers.m_strike_controllers, game_input.attack_direction, melee_action_triggers.strike_readying_entities, melee_action_triggers.strike_readying_directions);
                }
                break;
            case GameInputEvent::ReleaseStrike:
                // if we aren't going to block
                if(std::find(begin(game_input.events), end(game_input.events), GameInputEvent::Block) == end(game_input.events))
                {
                    Append(melee_action_triggers.strike_releasing_entities, m_controllers.m_strike_controllers);
                }
                break;
            case GameInputEvent::Block:
                Append(melee_action_triggers.strike_cancelling_entities, m_controllers.m_strike_controllers);
                Append(melee_action_triggers.block_starting_entities, m_controllers.m_strike_controllers);
                break;
            case GameInputEvent::ReleaseBlock:
                Append(melee_action_triggers.block_cancelling_entities, m_controllers.m_strike_controllers);
                break;
            case GameInputEvent::Throw:
                Append(entities_that_want_to_throw, m_controllers.m_throw_controllers);
                break;
            default:
                break;
        }
    }

    if( keep_player_orientation )
    {
        {
            auto entities = Append(kinematic_target_angle_entity_ids, m_controllers.m_kinematic_camera_rotation_controllers);
            auto z_angles = Grow(target_z_angles, Size(entities));
            GetEntityZAngles(entities, indexed_orientations, z_angles);
        }
        {
            auto entities = Append(target_angle_entity_ids, m_controllers.m_torque_camera_rotation_controllers);
            auto z_angles = Grow(target_z_angles, Size(entities));
            GetEntityZAngles(entities, indexed_orientations, z_angles);
        }
    }
    // otherwise use the camera controllers
    else
    {
        auto const camera_angles = m_camera.m_angles;

        GenerateCameraTargetAngles(
            m_controllers.m_kinematic_camera_rotation_controllers,
            camera_angles.z,
            // output
            kinematic_target_angle_entity_ids,
            kinematic_target_z_angles );

        GenerateCameraTargetAngles(
            m_controllers.m_torque_camera_rotation_controllers,
            camera_angles.z,
            // output
            target_angle_entity_ids,
            target_z_angles );
    }
}


void LogicWorld::TriggerTimerActions(
    const float time_step,
    std::vector<EntityID> & entities_that_released_a_throw,
	std::vector<EntityID> & died_entities,
	std::vector<EntityID> & replace_body_entities,
    std::vector<EntityID> & new_pre_holding_entities,
    std::vector<WielderDirectionType> & new_pre_holding_directions	)
{
    UpdateTimerProgress(time_step, m_release_throw_timers, entities_that_released_a_throw);

	// TODO: Change to event-based setup
    //UpdateTimerProgress(time_step, m_entity_dead_timers, output.animating_instructions.entity_ids, output.animating_instructions.instructions);
	UpdateTimerProgress(time_step, m_entity_dead_timers, died_entities);

    UpdateTimerProgress(time_step, m_entity_perished_timers, replace_body_entities);

	// Process wielding timers and update their states and mess
    UpdateMeleeWielderStates(time_step, m_entity_wielding_abilities, new_pre_holding_entities, new_pre_holding_directions);
}


void LogicWorld::AppendWieldingSelfCollisionPairs(std::vector<EntityPair> & pairs) const
{
    assert(Size(m_entity_wielding_abilities.entities) == Size(m_entity_wielding_abilities.wielded_entities));
    auto size = Size(m_entity_wielding_abilities.entities);
    auto range = Grow(pairs, size);
    for(size_t i = 0; i < size; i++)
    {
        auto current_wielder = m_entity_wielding_abilities.entities[i];
        auto current_wielded = m_entity_wielding_abilities.wielded_entities[i];
        range[i].id1 = current_wielder;
        range[i].id2 = current_wielded;
    }
}


namespace
{

    // After hitting something, interrupt the strike of the wielder and
    // let him play a rebound animation
    void ReboundWieldersStrike(EntityWieldingAbilities & wielding_abilities,
                               EntityID const & wielder_entity)
    {
        auto wielder_data_index = wielding_abilities.entity_to_data[wielder_entity.index];

        // Force wielder to take a short break in between two hits
        wielding_abilities.wielder_states[wielder_data_index] = WielderStateType::Recovering;
        wielding_abilities.wielder_state_times[wielder_data_index] = 0.75f;
    }

    void ApplyEntityForcesFromHits(Range<EntityID const *> target_entities, Range<Math::Float3 const *> velocities, EntityForces & entity_forces);

    // Apply entity forces from hits
    void ApplyEntityForcesFromHits(Range<EntityID const *> target_entities, Range<Math::Float3 const *> velocities, EntityForces & entity_forces)
    {
        assert(Size(target_entities) == Size(velocities));
        Append(entity_forces.entity_ids, target_entities);
        auto entity_forces_range = Grow(entity_forces.forces, Size(velocities));
        auto velocity_float_range = ReinterpretRange<float const>(velocities);
        auto force_float_range = ReinterpretRange<float>(entity_forces_range);
        Math::Multiply(velocity_float_range, 500.f, force_float_range);
    }
}


void LogicWorld::TriggerWeaponHits(
    IndexedVelocities const & indexed_velocities,
    CollisionEvents const & collision_events,
    EntityForces & entity_forces,
	AnimatingTriggers & animating_triggers,
    //EntityAnimatingInstructions & animating_instructions,
    CollisionEvents & weapon_collision_events)
{
    std::vector<EntityID> entities_that_hit_something;
    std::vector<EntityID> items_that_hit_something;
    std::vector<EntityID> target_entities;
    std::vector<Math::Float3> impact_velocities;

    ProcessWeaponCollisionEvents(collision_events,
        m_entity_wielding_abilities,
        m_numeric_property_container,
        //m_entity_animations,
        indexed_velocities,
        animating_triggers,
        target_entities,
        items_that_hit_something,
        entities_that_hit_something,
        impact_velocities,
        weapon_collision_events);

    // Rebound the strikes of the wielders whose weapons connected to targets
    for (auto i = 0u; i < entities_that_hit_something.size(); i++)
    {
        ReboundWieldersStrike(m_entity_wielding_abilities, entities_that_hit_something[i]);

		animating_triggers.strike_rebounding_entities.push_back(entities_that_hit_something[i]);
    }

    ProcessProjectileCollisionEvents(
        collision_events,
        m_projectile_container,
        indexed_velocities,
        target_entities,
        items_that_hit_something,
        entities_that_hit_something,
        impact_velocities,
        weapon_collision_events);

    ApplyEntityForcesFromHits(target_entities, impact_velocities, entity_forces);
}


LogicWorld::Output LogicWorld::UpdateGameLogic(const float time_step,
                                               EntityIDGenerator& entity_id_generator,
                                               GameInput const & game_input,
                                               IndexedOrientations const & indexed_orientations,
                                               IndexedVelocities const & indexed_velocities,
                                               IndexedAbsolutePoses const & indexed_poses,
                                               CollisionEvents const & collision_events,
                                               Output output)
{
	// ##### Perform immediate actions that occurred after the last game tick
    InitializeRecentEntities(output);

	Append(output.entity_component_replacements.entity_ids, m_immediate_body_replacements.entity_ids);
	Append(output.entity_component_replacements.template_ids, m_immediate_body_replacements.template_ids);

	m_immediate_body_replacements.entity_ids.clear();
	m_immediate_body_replacements.template_ids.clear();

	// ##### Update current logic and gather triggers and such things...
    // Keep track of striking actions to be triggered in the MeleeSystem
    MeleeActionTriggers melee_action_triggers;
	AnimatingTriggers animating_triggers;

    std::vector<EntityID> moving_entity_ids;
    std::vector<Math::Float2> movements;
    std::vector<EntityID> target_angle_entity_ids;
    std::vector<float> target_z_angles;
    std::vector<EntityID> kinematic_moving_entity_ids;
    std::vector<Math::Float2> kinematic_movements;
    std::vector<EntityID> kinematic_target_angle_entity_ids;
    std::vector<float> kinematic_target_z_angles;
    std::vector<EntityID> entities_that_want_to_jump;

    std::vector<EntityID> entities_that_want_to_throw;
    std::vector<EntityID> entities_that_want_to_pick_up_an_item;
    std::vector<EntityID> entities_that_want_to_drop_an_item;
    std::vector<EntityID> entities_that_released_a_throw;

	std::vector<EntityID> new_pre_holding_entities;
	std::vector<WielderDirectionType> new_pre_holding_directions;

	std::vector<EntityID> died_entities;
	std::vector<EntityID> perished_replaced_entities;

    TriggerKeyActions( game_input,
                      indexed_orientations,
                       // output
                       moving_entity_ids,
                       movements,
                       target_angle_entity_ids,
                       target_z_angles,
                       kinematic_moving_entity_ids,
                       kinematic_movements,
                       kinematic_target_angle_entity_ids,
                       kinematic_target_z_angles,
                       entities_that_want_to_jump,
                       melee_action_triggers,
                       entities_that_want_to_throw,
                       entities_that_want_to_pick_up_an_item,
                       entities_that_want_to_drop_an_item);

    

	// SCAMP: Any previously generated movement input could generate speed factors yet.
	//		  Assume for now that it is just the player input and set its factor to 1
	std::vector<float> target_speed_factors(movements.size(), 1.0f);
    if(!m_navmesh_container.m_navigation_meshes.empty())
    {
        auto & navigation_mesh = m_navmesh_container.m_navigation_meshes.front();
        UpdateAIControllers(indexed_orientations,
                            indexed_velocities,
                            m_ai_parameters,
                            // output
                            moving_entity_ids,
                            movements,
                            target_angle_entity_ids,
                            target_z_angles,
                            target_speed_factors,
                            melee_action_triggers,
                            navigation_mesh);
    }
    assert(Size(moving_entity_ids) == Size(movements));


    TriggerTimerActions( time_step, entities_that_released_a_throw, died_entities, perished_replaced_entities, new_pre_holding_entities, new_pre_holding_directions);
	Append(animating_triggers.died_entities, died_entities);

    // SCAMP
    std::vector<Math::Float3> throwing_directions(Size(entities_that_released_a_throw), Rotate(Math::Float3(0,0,-1), m_camera.m_rotation));

    // attack related stuff
    {
        SetHoldingAnimations(m_entity_animations, new_pre_holding_entities, new_pre_holding_directions, output.animating_instructions);

        // Only play the throwing events for expired throwers
        ProcessThrowerEntities(entity_id_generator,
                            entities_that_released_a_throw,
                            throwing_directions,
                            m_entity_throwing_abilities.entity_to_data,
                            m_entity_throwing_abilities.ammunition_entities,
                            m_entity_throwing_abilities.properties,
                            indexed_orientations,
                            indexed_velocities,
                            indexed_poses,
                            m_projectile_container,
                            output.entities_to_be_spawned);

        // Filter out all those triggered entities which can not perform the triggered action
        EnforceMeleeTriggerConditions(m_entity_wielding_abilities, melee_action_triggers);

        // Combine those entities cancelling a strike or block
        // to execute the same cancelling action
        std::vector<EntityID> melee_cancelling_entities;
        Append(melee_cancelling_entities, melee_action_triggers.strike_cancelling_entities);
        Append(melee_cancelling_entities, melee_action_triggers.block_cancelling_entities);

        // TODO: remove game_input dependency here!
        ExecuteMeleeTriggerActions(melee_action_triggers, melee_cancelling_entities, m_entity_animations, game_input.attack_direction, m_entity_wielding_abilities, output.animating_instructions);

		Append(animating_triggers.strike_cancelling_entities, melee_cancelling_entities);

        // THROWING:
        // Prune those throwing entities which currently hold a weapon:
        // Remove entity if it has a valid wielder index
        auto const & wielder_data_indices = m_entity_wielding_abilities.entity_to_data;
        auto valid_throwers_end = std::remove_if(entities_that_want_to_throw.begin(), entities_that_want_to_throw.end(),
            [&wielder_data_indices](EntityID const & entity_id)
        {
            return (GetOptional(wielder_data_indices, entity_id.index) != c_invalid_index);
        });

        entities_that_want_to_throw.resize(valid_throwers_end - begin(entities_that_want_to_throw));

        // Get absolute resulting sizes of timer vectors
        auto thrower_count = entities_that_want_to_throw.size();
        auto previous_timer_count = m_release_throw_timers.entity_ids.size();
        auto new_timer_count = previous_timer_count + thrower_count;

        // Create timers for all throwing entities:
        // Set entity ids on timer vectors
        m_release_throw_timers.entity_ids.insert( m_release_throw_timers.entity_ids.end(), entities_that_want_to_throw.begin(), entities_that_want_to_throw.end() );

        // Set remaining times
        m_release_throw_timers.remaining_times.resize( new_timer_count, 1.35f );

		Append(animating_triggers.throwing_entities, entities_that_want_to_throw);

        CollisionEvents weapon_collision_events;
        TriggerWeaponHits( indexed_velocities, collision_events, output.forces, animating_triggers, weapon_collision_events);
        assert(output.animating_instructions.entity_ids.size() == output.animating_instructions.instructions.size());

        std::vector<EntityID> damage_dealing_entities;
        std::vector<EntityID> damage_receiving_entities;

        CalculateHitsAndDamageTargets(
            weapon_collision_events,
            indexed_velocities,
            m_damage_dealers_container.entity_to_data,
            m_damage_dealers_container.damage_values,
            m_numeric_property_container.entity_to_property,
            m_numeric_property_container.current_hitpoints,
            damage_dealing_entities,
            damage_receiving_entities,
            m_entities_to_be_killed);

        // let all entities that die drop their weapon
        Append( entities_that_want_to_drop_an_item, m_entities_to_be_killed );
    }


    // kinematic movement
    {
        UpdateArrowkinematicControllers( kinematic_moving_entity_ids, kinematic_movements, indexed_orientations, time_step, output.positions );
        UpdatekinematicCameraRotationControllers( kinematic_target_angle_entity_ids, kinematic_target_z_angles, output.rotations );
    }

    // 'physically based' movement
    {
        UpdateMotionForceControllers(
            moving_entity_ids,
            movements,
			target_speed_factors,
            indexed_orientations,
            indexed_velocities,
            m_property_container.entity_to_property,
            m_property_container.motion_powers,
            m_property_container.motion_biases_positive,
            m_property_container.motion_biases_negative,
            m_property_container.motion_target_speeds,
            output.velocity_constraints );

        // UpdateCameraRotationTorqueControllers(
        //     target_angle_entity_ids,
        //     indexed_orientations,
        //     m_property_container.torques,
        //     m_property_container.entity_to_property,
        //     target_z_angles, time_step,
        //     output.angular_velocity_constraints );

        UpdateCameraRotationRotationConstraintsControllers(
                target_angle_entity_ids,
                target_z_angles,
                m_property_container.torques,
                m_property_container.entity_to_property,
                // indexed_velocities,
                output.rotation_constraints);


        entities_that_want_to_jump.resize(
            FilterEntitiesWhoHaveFeetTouchingASurface(
                collision_events,
                entities_that_want_to_jump
                )
            );
        // SCAMP SCAMP
        auto jump_forces = m_property_container.motion_powers;
        Math::Multiply(25, jump_forces);
        UpdateJumpControllers(
            entities_that_want_to_jump,
            m_property_container.entity_to_property,
            jump_forces,
            output.forces);
    }

    // velocity based animations
    {
        Range<EntityID const *> now_idle_entities, now_moving_entities;

        m_movement_system.moving_entities_start_index = ProcessMovement(
            m_movement_system.new_entities, m_movement_system.idle_and_moving_entities, m_movement_system.moving_entities_start_index,
            indexed_velocities, c_start_moving_speed_threshold, c_stop_moving_speed_threshold,
            now_idle_entities, now_moving_entities );

        m_movement_system.new_entities.clear();

		Append(animating_triggers.now_idle_entities, now_idle_entities);
		Append(animating_triggers.now_moving_entities, now_moving_entities);
    }

    // drop items
    {
        std::vector<EntityID> dropped_items;
        DropItems( entities_that_want_to_drop_an_item, m_entity_wielding_abilities, dropped_items );
        Append( output.entity_component_replacements.entity_ids, dropped_items );
        for( auto entity : dropped_items )
        {
            auto index = m_item_container.entity_to_data[entity.index];
            output.entity_component_replacements.template_ids.push_back( m_item_container.dropped_template_ids[index] );
        }
    }

    // Pick up items
    {
        //std::vector<WeaponWieldingParameters> result_wielding_parameters;
        std::vector<EntityID> picking_entities, picked_up_entities;
        PickUpNearestItems( entities_that_want_to_pick_up_an_item,
                            indexed_orientations,
                            m_item_container,
                            m_entity_wielding_abilities,
                            picking_entities,
                            picked_up_entities);

        // Set all the successfully picked-up weapons using the output parameters
        for( auto i = 0u; i < picking_entities.size(); i++ )
        {
            // Get current entity and picked weapon
            auto picking_entity = picking_entities[i];
            auto picked_up_entity = picked_up_entities[i];

            output.entity_component_replacements.entity_ids.push_back( picked_up_entity);
            output.entity_component_replacements.template_ids.push_back( m_item_container.equipped_template_ids[m_item_container.entity_to_data[picked_up_entity.index]] );

            // Use first attachment point of picking entity for weapon mounting
            auto entity_data_index = GetOptional(m_attachment_point_container.entity_to_data, picking_entity.index);
            assert(entity_data_index != unsigned(-1) && "Attempting to pick up weapon without valid attachment point");

            auto attachment_point = m_attachment_point_container.attachment_points[entity_data_index];

            m_entity_wielding_abilities.Add(
                picking_entity,
                picked_up_entity,
                attachment_point.bone_index,
                attachment_point.offset);
        }
    }

    // Play dying animation and set dead animation timer for all killed entities
    KillEntities(m_entities_to_be_killed, animating_triggers);

    m_entities_to_be_killed.clear();

    SortTimers(m_entity_dead_timers);
    SortTimers(m_release_throw_timers);

    // get the entity pairs that are not allowed to collide
    AppendWieldingSelfCollisionPairs(output.new_non_colliding_entity_pairs);

	// These have to be generated somewhere...
	GenerateTurningTriggers(indexed_velocities,
							m_animating_state_machine_container,
							animating_triggers.now_turning_entities,
							animating_triggers.no_longer_turning_entities);

	GenerateAnimatingInstructions(animating_triggers, m_entity_animations, m_animating_state_machine_container, output.animating_instructions);


	// ##### Replace bodies for entities that perished and will next be removed
	for (auto& entity : perished_replaced_entities)
	{
		auto dead_body_template_index = GetOptional(m_dead_entity_components.entity_to_data, entity.index);
		if (dead_body_template_index != c_invalid_index)
		{
			auto dead_body_template_id = m_dead_entity_components.template_ids[dead_body_template_index];

			output.entity_component_replacements.entity_ids.push_back(entity);
			output.entity_component_replacements.template_ids.push_back(dead_body_template_id);
		}
	}
	// #####

	// Remove the logic components of the dead entities
	RemoveEntities(perished_replaced_entities);

    return output;
}

// Use the temporary storage of m_uninitialized_entities to set default actions on other worlds
void LogicWorld::InitializeRecentEntities(Output & output)
{
    // Scampage: Set torso tilt animation
    // for any recently spawned entity that may play it:

    // Search animation template map for torso tilting, abort if none exists
    auto animation_template_iterator = m_entity_animations.animation_template_ids.find(AnimationStateType::TorsoTilting);
    if (animation_template_iterator == m_entity_animations.animation_template_ids.end()) return;

    // If found, get the map of torso tilting animation template ids
    auto & torso_tilting_animation_templates = animation_template_iterator->second;

    // Prepare default instruction
    AnimatingInstruction instruction;
    instruction.blend_mode = Animating::BlendMode::Additive;
    instruction.layer_index = 3;

    for (auto & entity_id : m_uninitialized_entities)
    {
        // Check if a torso tilting animation template exists for the current entity
        auto template_iterator = torso_tilting_animation_templates.find(entity_id);
        if (template_iterator == torso_tilting_animation_templates.end()) continue;

        // If a valid torso-tilting animation was found, set it as logic output
        instruction.template_id = template_iterator->second;

        output.animating_instructions.entity_ids.push_back(entity_id);
        output.animating_instructions.instructions.push_back(instruction);
    }

    // Clear uninitialized entities for next game tick
    m_uninitialized_entities.clear();
}


// Manipulate the final orientations for all entities
void LogicWorld::PostProcessOrientations(IndexedOrientations const & orientations, IndexedAbsolutePoses const & poses, EntityPositions& entity_positions, EntityRotations& entity_rotations) const
{
    auto const & wielding_abilities = m_entity_wielding_abilities;
    auto wielder_count = wielding_abilities.entities.size();

    // Allocate enough space in vectors
    entity_positions.entity_ids.reserve(wielder_count);
    entity_rotations.entity_ids.reserve(wielder_count);
    entity_positions.positions.reserve(wielder_count);
    entity_rotations.rotations.reserve(wielder_count);

    // Set wielded items relative to wielders weapon bone
    for (auto i = 0u; i < wielder_count; i++)
    {
        // Get current wielding and wielded entity
        auto& wielding_entity = wielding_abilities.entities[i];
        auto& wielded_entity = wielding_abilities.wielded_entities[i];

        // Get orientations
        auto wielder_orientation_index = orientations.indices[wielding_entity.index];
        auto const & wielder_orientation = orientations.orientations[wielder_orientation_index];

        // Get hand bone position of wielding entity
        auto const bone_index = wielding_abilities.bone_indices[i]; // TODO: Store in entity abilities
        auto pose_index = poses.indices[wielding_entity.index];

        auto bone_state_index = poses.pose_offsets[pose_index] + bone_index;
        auto bone_state = poses.bone_states[bone_state_index];

        auto weapon_offset = wielding_abilities.weapon_offsets[i];

        auto weapon_orientation = ToParentFromLocal(wielder_orientation, bone_state);
        weapon_orientation = ToParentFromLocal(weapon_orientation, weapon_offset);

        // Store entry in EntityPositions and EntityRotations as output
        entity_positions.entity_ids.push_back(wielded_entity);
        entity_rotations.entity_ids.push_back(wielded_entity);

        entity_positions.positions.push_back(weapon_orientation.position);
        entity_rotations.rotations.push_back(weapon_orientation.rotation);
    }
}


void LogicWorld::SetCameraPositionAndAngles( Math::Float3 position, Math::Float3 angles )
{
    m_camera.m_position = position;
    m_camera.SetAngles( angles );
}


void LogicWorld::SetCameraTarget(EntityID entity_id)
{
	//Use pseudo game rules
	auto position_offset = Math::Float3(0.0f, 0.0f, 3.0f);
	auto pivot_offset = Math::Float3(0.0f, 0.0f, 1.5f);

	m_camera.SetTarget(entity_id, position_offset, pivot_offset);
}


void LogicWorld::AdjustAllPositions(Math::Float3 adjustment)
{
    m_camera.m_position += adjustment;
}


void LogicWorld::UpdateCamera(const float time_step, CameraInput const & input, IndexedOrientations const & indexed_orientations)
{
    m_camera.Update(time_step, input, indexed_orientations);
}
