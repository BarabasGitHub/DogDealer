#pragma once
#include "DLL.h"

#include "Camera.h"
#include "Configuration.h"
#include "Controllers.h"
#include "EntityAbilities.h"
#include "EntityAnimations.h"
#include "EntityComponentReplacements.h"
#include "ResourceDescriptions.h"
#include "DeadEntityComponents.h"
#include "DamageDealersContainer.h"

#include "ResourceManager.h"

#include "MovementSystem.h"
#include "TimerSystem.h"

#include "AIParameterContainer.h"
#include "NavigationMeshContainer.h"

#include "ItemContainer.h"
#include "ProjectileContainer.h"
#include "PropertyContainer.h"
#include "AttachmentPointContainer.h"

#include "AnimatingStateMachineContainer.h"

#include <Conventions\AnimatingInstructions.h>
#include <Conventions\CollisionEvent.h>
#include <Conventions\EntitySpawnDescription.h>
#include <Conventions\EntityIDGenerator.h>
#include <Conventions\Force.h>
#include <Conventions\Orientation.h>
#include <Conventions\PoseInfo.h>
#include <Conventions\Velocity.h>
#include <Conventions\RotationConstraints.h>
#include <Conventions\VelocityConstraints.h>

#include <Math\FloatMatrixTypes.h>

struct GameInput;

namespace Logic{

    struct MeleeActionTriggers;
	struct AnimatingTriggers;

	class GAMELOGIC_DLL LogicWorld{

	public:

        LogicWorld();
        ~LogicWorld();

        void CreateMobileComponent( EntityID id, MobileComponentDescription const & description );
        void CreateItemComponent( EntityID id, EntityTemplateID equipped_template, EntityTemplateID dropped_template );
        void CreateDamageDealerComponent( EntityID id, DamageDealerComponentDescription description );

        void RemoveEntities( Range<EntityID const *> entity_ids );
        void KillEntities(std::vector<EntityID> const & entity_ids, AnimatingTriggers & animating_triggers);

		void SetAmmunition( EntityID id, ThrowProperties properties, EntityTemplateID ammo_template_id );
        void SetHitpoints(EntityID const id, float const hitpoints);
        bool HasPositiveHitpoints(EntityID id) const;

        // this just adds the entity id and map to the system, does not check for existing entity
        void SetAnimations( EntityID id, std::map<AnimationStateType, AnimationTemplateID> animation_map );

        void EntityWieldItem(EntityID const wielder_entity_id, EntityID const item_entity_id, AttachmentPoint const attachment_point);

        void SetAIAggressionTarget(EntityID const ai_entity_id, EntityID const target_entity_id, float const maximum_combat_distance);
        void SetAIFollowingTarget(EntityID const ai_entity_id, EntityID const target_entity_id, float const minimum_following_distance, float const maximum_following_distance);
        void SetAIToPassive(EntityID const ai_entity_id);

		void SetAIPatrollingTarget(EntityID const ai_entity_id, Math::Float3 const target_position);

        void KillEntity(EntityID const entity_id);

        void SetAfterDeathComponents( EntityID const entity_id, EntityTemplateID after_death_components_template_id );

		void CreateControllerComponent(EntityID entity_id);
        void CreateForceControllerComponent( EntityID entity_id );
        void FollowCameraRotationWithTorque( EntityID entity_id );
        void FollowCameraRotationkinematic( EntityID entity_id );

        void CreateThrowController( EntityID entity_id );
        void CreateStrikeController( EntityID entity_id );

        struct Output
        {
            EntityPositions positions;
            EntityRotations rotations;
            EntityForces forces;
            EntityTorques torques;

            RotationConstraints rotation_constraints;

            VelocityConstraints velocity_constraints;
            AngularVelocityConstraints angular_velocity_constraints;

            std::vector<EntitySpawn> entities_to_be_spawned;
            std::vector<EntityID> entities_to_be_pruned;

            EntityComponentReplacements entity_component_replacements;
            EntityAnimatingInstructions animating_instructions;

            // entity pairs that should be ignored by the collision detection
            std::vector<EntityPair> new_non_colliding_entity_pairs;
            std::vector<EntityPair> expired_non_colliding_entity_pairs;
        };

        Output UpdateGameLogic(
            const float time_step,
            EntityIDGenerator& entity_id_generator,
            GameInput const & game_input,
            IndexedOrientations const & indexed_orientations,
            IndexedVelocities const & indexed_velocities,
            IndexedAbsolutePoses const & indexed_poses,
            CollisionEvents const & collision_events,
            Output = {});

        void InitializeRecentEntities(Output & output);

        void AppendWieldingSelfCollisionPairs(std::vector<EntityPair> & pairs) const;

        void PostProcessOrientations(IndexedOrientations const & orientations, IndexedAbsolutePoses const & poses, EntityPositions& entity_positions, EntityRotations& entity_rotations) const;

        void SetCameraPositionAndAngles( Math::Float3 position, Math::Float3 angles );
		void SetCameraTarget(EntityID entity_id);

        void TriggerWeaponHits(
            IndexedVelocities const & indexed_velocities,
            CollisionEvents const & collision_events,
            EntityForces& entity_forces,
			AnimatingTriggers & animating_triggers,
            CollisionEvents & output_weapon_collisions);

        void TriggerKeyActions(GameInput const & game_input,
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
                                std::vector<EntityID> & entities_that_want_to_drop_an_item);

        void TriggerTimerActions(
			const float time_step,
			std::vector<EntityID> & entities_that_released_a_throw,
			std::vector<EntityID> & died_entities,
			std::vector<EntityID> & replace_body_entities,
			std::vector<EntityID> & new_pre_holding_entities,
			std::vector<WielderDirectionType> & new_pre_holding_directions);


        void AdjustAllPositions(Math::Float3 adjustment);

        void UpdateCamera(const float time_step, CameraInput const & input, IndexedOrientations const & indexed_orientations);

    public:
        WorldConfiguration m_configuration;
        Camera	m_camera;
	private:

        std::vector<EntityID> m_uninitialized_entities;
        std::vector<EntityID> m_entities_to_be_killed;

		struct ImmediateBodyReplacements
		{
			std::vector<EntityID> entity_ids;
			std::vector<EntityTemplateID> template_ids;
		};
		ImmediateBodyReplacements m_immediate_body_replacements;


        ResourceManager	m_resource_manager;

		Controllers m_controllers;
        PropertyContainer m_property_container;
        NumericPropertyContainer m_numeric_property_container;

        EntityThrowingAbilities m_entity_throwing_abilities;
        EntityWieldingAbilities m_entity_wielding_abilities;
        DeadEntityComponents m_dead_entity_components;

		MovementSystem m_movement_system;
        EntityAnimations m_entity_animations;

        ReleaseThrowTimers m_release_throw_timers;
        EntityDeadTimers m_entity_dead_timers;
        EntityPerishedTimers m_entity_perished_timers;

        ProjectileContainer m_projectile_container;
        ItemContainer m_item_container;
        AttachmentPointContainer m_attachment_point_container;
        DamageDealersContainer m_damage_dealers_container;

        AIParameterContainer m_ai_parameters;
        NavigationMeshContainer m_navmesh_container;

		AnimatingStateMachineContainer m_animating_state_machine_container;
	};
}
