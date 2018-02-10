#pragma once

#include "EntityDescription.h"
#include "EntityTemplates.h"
#include "WorldConfiguration.h"

#include <Conventions\EntityIDGenerator.h>
#include <Input\InputManager.h>
#include <Utilities\Window.h>

#include <Animating\AnimatingWorld.h>
#include <Conventions\EntitySpawnDescription.h>
#include <GameLogic\LogicWorld.h>
#include <Graphics\RenderWorld.h>
#include <Physics\PhysicsWorld.h>
#include <Utilities\SimplexNoise.h>

#include <vector>

struct GameInput;
struct InterfaceInput;

class DogWorld{

public:

    DogWorld();

    void CreateAndShowWindow(HINSTANCE instance, int show_command, std::wstring title = L"");
    void Run();

    void CreateEntities( std::vector<EntityDescription> const & entity_descriptions );

    EntityID CreateTerrain(
        EntityDescription const & description,
        Math::Float3 const & terrain_center,
        float const update_distance,
        Math::Unsigned3 const & terrain_block_count,
        Math::Float3 const & terrain_block_dimensions,
        Math::Unsigned3 const & terrain_block_cube_count,
        std::vector<float> const & degradation_thresholds,
        std::vector<NoiseParameters<Math::Float3>> const & noise_parameters );

    EntityID DogWorld::CreateTerrain(
        EntityDescription const & description,
        Math::Float3 const & terrain_center,
        Math::Unsigned2 patch_dimensions,
        Math::Float2 patch_size,
        uint32_t lod_count,
        std::vector<NoiseParameters<Math::Float2>> const & noise_parameters);

    void SetTerrainGrassTypes(std::vector<Graphics::RenderComponentDescription> const & descriptions, std::vector<std::array<float, Graphics::c_lod_count>> const & densities);

    // the input entities will be reordered
	void RemoveEntities(std::vector<EntityID> & entity_ids);

    EntityID SpawnEntity(EntityTemplateID template_id, Orientation orientation, Velocity velocity );

    EntityID SpawnEntity(EntityDescription const & description, Orientation orientation, Velocity velocity);

    void AddEntityComponents( EntityID const entity_id, EntityDescription const & description, Orientation orientation, Velocity velocity );

    void ReplaceEntityComponents( Range<EntityID const *> entity_ids, Range<EntityDescription const * const *> descriptions );

    EntityID SpawnItem( Logic::ItemComponentDescription description, Orientation orientation, Velocity velocity );

    void EntityWieldItem(EntityID const wielder_entity_id, EntityID const wielded_entity_id, AttachmentPoint const attachment_point);

    void SetAIAgressionTarget(EntityID const ai_entity_id, EntityID const target_entity_id, float const maximum_distance);
    void SetAIFollowingTarget(EntityID const ai_entity_id, EntityID const target_entity_id, float const minimum_following_distance, float const maximum_following_distance);
    void SetAIToPassive(EntityID const ai_entity_id);
	void SetAIPatrollingTarget(EntityID const ai_entity_id, Math::Float3 const target_position);

    void KillEntity(EntityID const entity_id);

    void SetAfterDeathComponents( EntityID const entity_id, EntityTemplateID after_death_components_template_id );

    AnimationTemplateID LoadAnimation( Animating::AnimationDescription animation_description );

    void SetEntityAnimations( EntityID id, std::string const & skeleton, std::map<AnimationStateType, AnimationTemplateID> animations );

    void SetEntityAmmunition( EntityID id, Logic::ThrowProperties properties, EntityTemplateID ammo_template_id );

    void SetEntityHitpoints( EntityID id, float const hitpoints);
    bool HasPositiveHitpoints( EntityID id ) const;

    void CreatePlayerController( EntityID entit_id );

    void CreatePlayerThrowController( EntityID entity_id );

    void CreatePlayerStrikeController( EntityID entity_id );

    void AddLight( Graphics::LightDescription const & light_description, EntityID entity_id );

    void SetCameraPositionAndAngles( Math::Float3 position, Math::Float3 angles );

    void FollowCameraRotation( EntityID entity_id );

	void SetCameraTarget( EntityID entit_id );

    Math::Float3 FindRestingPositionOnEntity( Math::Float3 starting_position, EntityID target_entity ) const;

    Input::WorldConfiguration & InputConfiguration( );
    Input::WorldConfiguration const & InputConfiguration( ) const;

    Logic::WorldConfiguration & LogicConfiguration( );
    Logic::WorldConfiguration const & LogicConfiguration( ) const;

    Graphics::WorldConfiguration & GraphicsConfiguration();
    Graphics::WorldConfiguration const & GraphicsConfiguration() const;

    void SetPhysicsConfiguration(Physics::WorldConfiguration const &);

    void SetGravity( Math::Float3 const & gravity );

    Math::Float3 const & GetGravity();
    Orientation GetEntityOrientation(EntityID const entity_id);
    void GetOrientations(Range<EntityID const*> entity_ids, Range<Orientation *> orientations);


    std::function<void( double const time_step )> m_script_callback;

    bool m_pause_simulation = false;
    bool m_take_single_step = false;

    Orientation m_previous_camera_orientation;

    EntityTemplates m_entity_templates;

    WorldConfiguration m_world_configuration;

private:

    // this one should not exist
    EntityID SpawnEntity(EntityTemplateID template_id, Orientation orientation, Velocity velocity, EntityID id );

    void InputAndScriptUpdate(GameInput & game_input_output, InterfaceInput & interface_input_output);
    CollisionEvents DoGameTick( CollisionEvents collision_events, GameInput const & game_input, InterfaceInput const & interface_input );

    void RemoveReplaceAndSpawnEntities();

    void SetEntityName(EntityID id, std::string name);

    void SetPhysicsDebugVisualization();

    Math::Unsigned3 m_world_reference_position;

    Window m_window;
    InputManager m_input_manager;

    EntityIDGenerator m_entity_id_generator;

	Physics::PhysicsWorld	m_physics_world;
	Graphics::RenderWorld	m_render_world;
	Animating::AnimatingWorld m_animating_world;
    Logic::LogicWorld m_logic_world;

    std::vector<EntityID> m_entity_ids;
    std::vector<std::string> m_entity_names;

    std::vector<EntitySpawn> m_entities_to_be_spawned;
	std::vector<EntityID> m_entities_to_be_pruned;
    EntityComponentReplacements m_entity_component_replacements;
};


inline Input::WorldConfiguration & DogWorld::InputConfiguration() { return m_input_manager.m_configuration; }
inline Input::WorldConfiguration const & DogWorld::InputConfiguration() const { return m_input_manager.m_configuration; }

inline Logic::WorldConfiguration & DogWorld::LogicConfiguration() { return m_logic_world.m_configuration; }
inline Logic::WorldConfiguration const & DogWorld::LogicConfiguration() const { return m_logic_world.m_configuration; }

inline Graphics::WorldConfiguration & DogWorld::GraphicsConfiguration() { return m_render_world.m_world_configuration; }
inline Graphics::WorldConfiguration const & DogWorld::GraphicsConfiguration() const { return m_render_world.m_world_configuration; }
