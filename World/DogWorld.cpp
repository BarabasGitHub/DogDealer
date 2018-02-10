#pragma once

#include "DogWorld.h"

#include <Conventions\VelocityConstraints.h>
#include <Conventions\PerspectiveViewFunctions.h>
#include <BoundingShapes\AxisAlignedBoxFunctions.h>
#include <BoundingShapes\RayFunctions.h>
#include <Math\Conversions.h>
#include <Math\MathFunctions.h>

#include <Utilities\DogDealerException.h>
#include <Utilities\HRTimer.h>
#include <Utilities\Logger.h>
#include <Utilities\Range.h>
#include <Utilities\SimplexNoise.h>
#include <Utilities\VectorHelper.h>

#include <Input\InputTranslator.h>
#include <Input\GameInput.h>
#include <Input\InterfaceInput.h>

#include <thread>

namespace
{
    auto const c_move_reference_point_threshold = 1024;
    // auto const c_move_reference_point_threshold = 8;
}



DogWorld::DogWorld()
{
    m_world_reference_position = 0;
    m_world_configuration.time_step = 1 / 60.f;
    ClearLogFile();
    Log( "Hello DogWorld!" );
}


void DogWorld::CreateAndShowWindow( HINSTANCE instance, int show_command, std::wstring title )
{
    m_window.SetWindowTitle( std::move(title) );

    using namespace std::placeholders;

    // register the input manager before creating the window so it also receive any create messages
    m_window.RegisterForProcessingMessages( std::bind( std::mem_fn( &InputManager::WindowProcess ), &m_input_manager, _1, _2, _3, _4 ) );

    m_window.Register( instance );
    if( !m_window.ShowWindowAsync( show_command ) ) throw WindowsException( HRESULT_FROM_WIN32( GetLastError() ) );

    m_render_world.DefaultInitialize( m_window.GetWindowHandle() );

    InputManager::RegisterInputDevices( m_window.GetWindowHandle() );
}


void DogWorld::CreateEntities( std::vector<EntityDescription> const & entity_descriptions )
{
    // Create RenderComponents
    for( auto const & description : entity_descriptions )
    {
        Orientation orientation = { Float3FromUnsigned3(m_world_reference_position), Math::Identity() };
        SpawnEntity( description, orientation, 0 );
    }
}


namespace
{
    // remove entities from 'whole collection' that are in 'to be removed'
    void FilterEntities( std::vector<EntityID> & whole_collection, std::vector<EntityID> & to_be_removed )
    {
        auto entity_index_less = []( EntityID const & a, EntityID const & b ) { return a.index < b.index; };
        // sort the to be removed entities
        std::sort( begin( to_be_removed ), end( to_be_removed ), entity_index_less );
        whole_collection.erase( std::remove_if( begin( whole_collection ), end( whole_collection ),
            [to_be_removed, entity_index_less]( EntityID const & id )
        {
            return std::binary_search( begin( to_be_removed ), end( to_be_removed ), id, entity_index_less );
        } ), end( whole_collection ) );
    }
}


// Call on all worlds to create their components for the given entity
void DogWorld::RemoveEntities( std::vector<EntityID> & entity_ids )
{
    m_logic_world.RemoveEntities( entity_ids );

    m_render_world.RemoveEntities( entity_ids );

    m_animating_world.RemoveEntities(entity_ids);

    m_physics_world.RemoveEntities(entity_ids);

    FilterEntities( m_entity_ids, entity_ids );

    m_entity_id_generator.Remove( entity_ids );
}

void DogWorld::SetEntityName(EntityID id, std::string name)
{
    Log([&](){return "Entity: " + name + " assigned id: " + std::to_string(id.index);});
    m_entity_names.resize(std::max<size_t>(id.index + 1, m_entity_names.size()));
    m_entity_names[id.index] = move(name);
}


EntityID DogWorld::SpawnEntity( EntityTemplateID template_id, Orientation orientation, Velocity velocity )
{
    auto const & description = m_entity_templates.descriptions[template_id.index];

    auto entity_id = m_entity_id_generator.NewID();
    m_entity_ids.push_back( entity_id );

    AddEntityComponents( entity_id, description, orientation, velocity );

    SetEntityName(entity_id, description.name);

    return entity_id;
}


EntityID DogWorld::SpawnEntity( EntityTemplateID template_id, Orientation orientation, Velocity velocity, EntityID entity_id )
{
    auto const & description = m_entity_templates.descriptions[template_id.index];

    assert(std::find(begin(m_entity_ids), end(m_entity_ids), entity_id) == end(m_entity_ids));
    m_entity_ids.push_back( entity_id );

    AddEntityComponents( entity_id, description, orientation, velocity );

    SetEntityName(entity_id, description.name);

    return entity_id;
}


EntityID DogWorld::SpawnEntity( EntityDescription const & description, Orientation orientation, Velocity velocity )
{
    auto entity_id = m_entity_id_generator.NewID();

    m_entity_ids.push_back( entity_id );

    AddEntityComponents( entity_id, description, orientation, velocity );

    SetEntityName(entity_id, description.name);

    return entity_id;
}


void DogWorld::AddEntityComponents( EntityID const entity_id, EntityDescription const & description, Orientation orientation, Velocity velocity )
{
    orientation.position -= Float3FromUnsigned3(m_world_reference_position);

    if( description.logic_mobile_component_desc != nullptr )
    {
        if (description.logic_mobile_component_desc->motion_power != 0)
        {
            m_logic_world.CreateMobileComponent( entity_id, *description.logic_mobile_component_desc );
        }
    }

    if( description.logic_damage_dealer_component_description != nullptr )
    {
        m_logic_world.CreateDamageDealerComponent( entity_id, *description.logic_damage_dealer_component_description );
    }

    for( auto & rc_description : description.render_component_desc )
    {
        m_render_world.CreateRenderComponent( rc_description, entity_id );
    }

    // Create animating component if defined
    if( description.animating_component_desc != nullptr )
    {
        std::map<AnimationStateType, AnimationTemplateID> animations;
        for( auto & animation : description.animating_component_desc->animations )
        {
            auto animation_id = LoadAnimation( animation.second );
            animations.emplace( animation.first, animation_id );
        }

        SetEntityAnimations( entity_id, description.animating_component_desc->skeleton, animations );

    }

    if(description.physics_component_desc)
    {
        if(description.physics_component_desc->body_type != Physics::BodyType::Rigid || Size(description.physics_component_desc->bodies) == 1)
        {
            for(auto & pc_description : description.physics_component_desc->bodies)
            {
                switch(description.physics_component_desc->body_type)
                {
                    case Physics::BodyType::Static:
                        ;
                        {
                            if(pc_description.collision_file.empty())
                            {
                                m_physics_world.CreateStaticNonCollidingBodyComponent(entity_id, orientation);
                            }
                            else
                            {
                                m_physics_world.CreateStaticBodyComponent(entity_id, pc_description.collision_file, orientation, pc_description.bounciness, pc_description.friction_factor);
                            }
                            break;
                        }
                    case Physics::BodyType::Kinematic:
                        ;
                        {
                            if(pc_description.collision_file.empty())
                            {
                                m_physics_world.CreateKinematicNonCollidingBodyComponent(entity_id, orientation);
                            }
                            else
                            {
                                m_physics_world.CreateKinematicBodyComponent(entity_id, pc_description.collision_file, orientation, pc_description.bounciness, pc_description.friction_factor);
                            }
                            break;
                        }
                    case Physics::BodyType::Rigid:
                        ;
                        {
                            assert(!pc_description.collision_file.empty());
                            auto const mass = pc_description.mass;
                            Physics::Movement movement = {velocity * mass, 0};
                            m_physics_world.CreateRigidBodyComponent(entity_id, pc_description.collision_file, mass, orientation, movement, pc_description.bounciness, pc_description.friction_factor, pc_description.lock_rotation);
                            break;
                        }
                    default:
                        assert(false);
                }
            }
        }
        else
        {
            float mass = 0;
            for(auto const & bd : description.physics_component_desc->bodies)
            {
                mass += bd.mass;
            }
            Physics::Movement movement = {velocity * mass, 0};
            m_physics_world.CreateMultiRigidBodyComponent(
                entity_id,
                orientation,
                movement,
                description.physics_component_desc->bodies,
                description.physics_component_desc->connections
                );
        }
    }

}


void DogWorld::ReplaceEntityComponents( Range<EntityID const *> entity_ids, Range<EntityDescription const * const *> descriptions )
{
    assert( Size( entity_ids ) == Size( descriptions ) );


    for(auto i = 0u; i < Size(entity_ids); ++i)
    {
        auto entity_id = entity_ids[i];
        auto & description = descriptions[i][0];

        // only replace if specified
        if(description.logic_mobile_component_desc != nullptr)
        {
            m_logic_world.RemoveEntities(CreateRange(&entity_id, 1));

            if(description.logic_mobile_component_desc->motion_power != 0)
            {
                m_logic_world.CreateMobileComponent(entity_id, *description.logic_mobile_component_desc);
            }
        }
        if(description.logic_damage_dealer_component_description != nullptr)
        {
            m_logic_world.CreateDamageDealerComponent(entity_id, *description.logic_damage_dealer_component_description);
        }

        m_render_world.ReplaceRenderComponents(description.render_component_desc, entity_id);

        // not sure what to do in this case.
        assert(description.animating_component_desc == nullptr);
        //{
        //    std::map<AnimationStateType, AnimationTemplateID> animations;
        //    for( auto & animation : description.animating_component_desc->animations )
        //    {
        //        auto animation_id = LoadAnimation( animation.second );
        //        animations.emplace( animation.first, animation_id );
        //    }

        //    SetEntityAnimations( entity_id, description.animating_component_desc->skeleton, animations );

        //}

        if(description.physics_component_desc != nullptr)
        {
            for(auto & pc_description : description.physics_component_desc->bodies)
            {
                //auto pc_description = First(description.physics_component_desc->bodies);
                switch(description.physics_component_desc->body_type)
                {
                    case Physics::BodyType::Static:
                        ;
                        {
                            if(pc_description.collision_file.empty())
                            {
                                assert(!"Cannot currently replace with non colliding body.");
                                //m_physics_world.ReplaceWithStaticNonCollidingBodyComponent( entity_id, orientation );
                            }
                            else
                            {
                                m_physics_world.ReplaceWithStaticBodyComponent(entity_id, pc_description.collision_file, pc_description.bounciness, pc_description.friction_factor);
                            }
                            break;
                        }
                    case Physics::BodyType::Kinematic:
                        ;
                        {
                            if(pc_description.collision_file.empty())
                            {
                                assert(!"Cannot currently replace with non colliding body.");
                                //m_physics_world.ReplaceWithKinematicNonCollidingBodyComponent( entity_id, orientation );
                            }
                            else
                            {
                                m_physics_world.ReplaceWithKinematicBodyComponent(entity_id, pc_description.collision_file, pc_description.bounciness, pc_description.friction_factor);
                            }
                            break;
                        }
                    case Physics::BodyType::Rigid:
                        ;
                        {
                            assert(!pc_description.collision_file.empty());
                            auto const mass = pc_description.mass;
                            m_physics_world.ReplaceWithRigidBodyComponent(entity_id, pc_description.collision_file, mass, pc_description.bounciness, pc_description.friction_factor, pc_description.lock_rotation);
                            break;
                        }
                }
            }
        }
    }
}


EntityID DogWorld::SpawnItem( Logic::ItemComponentDescription description, Orientation const orientation, Velocity const velocity )
{
    auto entity_id = SpawnEntity(description.dropped_template, orientation, velocity);
    SetEntityName(entity_id, description.name);
    m_logic_world.CreateItemComponent( entity_id, description.equipped_template, description.dropped_template );
    return entity_id;
}


void DogWorld::EntityWieldItem( EntityID const wielder_entity_id, EntityID const wielded_entity_id, AttachmentPoint const attachment_point)
{
    // Ensure that the bone index is valid for the wielder entity
    auto skeleton_bone_count = m_animating_world.GetBoneCountOfEntity(wielder_entity_id);
    if(attachment_point.bone_index >= skeleton_bone_count)
    {
        throw DogDealerException("Weapon mounted to invalid bone!", false);
    }

    m_logic_world.EntityWieldItem( wielder_entity_id, wielded_entity_id, attachment_point);
}


void DogWorld::SetAIAgressionTarget(EntityID const ai_entity_id, EntityID const target_entity_id, float const maximum_distance)
{
    m_logic_world.SetAIAggressionTarget(ai_entity_id, target_entity_id, maximum_distance);
}


void DogWorld::SetAIFollowingTarget(EntityID const ai_entity_id, EntityID const target_entity_id, float const minimum_following_distance, float const maximum_following_distance)
{
    m_logic_world.SetAIFollowingTarget(ai_entity_id, target_entity_id, minimum_following_distance, maximum_following_distance);
}


void DogWorld::SetAIToPassive(EntityID const ai_entity_id)
{
    m_logic_world.SetAIToPassive(ai_entity_id);
}


void DogWorld::SetAIPatrollingTarget(EntityID const ai_entity_id, Math::Float3 const target_position)
{
	m_logic_world.SetAIPatrollingTarget(ai_entity_id, target_position);
}


void DogWorld::KillEntity(EntityID const entity_id)
{
    m_logic_world.KillEntity(entity_id);
}


void DogWorld::SetAfterDeathComponents( EntityID const entity_id, EntityTemplateID after_death_components_template_id )
{
    m_logic_world.SetAfterDeathComponents( entity_id, after_death_components_template_id );
}


EntityID DogWorld::CreateTerrain(EntityDescription const & description,
                                Math::Float3 const & terrain_center,
                                float const update_distance,
                                Math::Unsigned3 const & terrain_block_count,
                                Math::Float3 const & terrain_block_dimensions,
                                Math::Unsigned3 const & terrain_block_cube_count,
                                std::vector<float> const & degradation_thresholds,
                                std::vector<NoiseParameters<Math::Float3>> const & noise_parameters)
{
    auto entity_id = m_entity_id_generator.NewID();

    m_entity_ids.push_back( entity_id );

    auto orientation = Orientation();
    orientation.position = terrain_center - Float3FromUnsigned3(m_world_reference_position);
    orientation.rotation = Math::Identity();

    auto sample_function = [noise_parameters]( Math::Float3 position, float& density, Math::Float3& gradient )
    {
        density = SimplexNoise<Math::Float3>( noise_parameters, position, gradient );
        density -= position.z;
        gradient.z -= 1;
    };

    // Create RenderComponent
    assert( description.render_component_desc.size() == 1 );
    m_render_world.CreateTerrainRenderComponent( description.render_component_desc[0], orientation.position, update_distance, terrain_block_count, terrain_block_dimensions, terrain_block_cube_count, degradation_thresholds, sample_function, entity_id );

    BoundingShapes::AxisAlignedBox box;
    box.center = 0;
    //box.extent = Float3FromUnsigned3( total_size ) / 2;
    box.extent = Math::Float3(1e30f); // totally out of the blue magic large value
    box = BoundingShapes::TransformByOrientation(box, orientation);
    m_physics_world.CreateStaticDensityBodyComponent(entity_id, box, sample_function, orientation, description.physics_component_desc->bodies[0].bounciness, description.physics_component_desc->bodies[0].friction_factor);

    return entity_id;
}



EntityID DogWorld::CreateTerrain(EntityDescription const & description,
                                Math::Float3 const & terrain_center,
                                Math::Unsigned2 patch_dimensions,
                                Math::Float2 patch_size,
                                uint32_t lod_count,
                                std::vector<NoiseParameters<Math::Float2>> const & noise_parameters)
{
    if(Math::Even(patch_dimensions.x) || Math::Even(patch_dimensions.y))
    {
        throw DogDealerException("The 2D terrain requires odd patch dimensions.", false);
    }
    auto entity_id = m_entity_id_generator.NewID();

    m_entity_ids.push_back( entity_id );

    auto orientation = Orientation();
    orientation.position = terrain_center - Float3FromUnsigned3(m_world_reference_position);
    orientation.rotation = Math::Identity();

    auto sample_function_3d = [noise_parameters]( Math::Float3 position, float& density, Math::Float3& gradient3d )
    {
        Math::Float2 gradient;
        auto height = SimplexNoise<Math::Float2>( noise_parameters, Math::Float2(position.x, position.y), gradient );
        density = height - position.z;
        gradient3d = {gradient.x, gradient.y, -1.f};
    };

    auto sample_function_2d = [noise_parameters]( Math::Float2 position, float& density, Math::Float2& gradient )
    {
        density = SimplexNoise<Math::Float2>( noise_parameters, Math::Float2(position.x, position.y), gradient );
    };

    // Create RenderComponent
    assert( description.render_component_desc.size() == 1 );
    std::vector<float> lod_distances(lod_count, Math::Norm(patch_size) * 4);
    for( auto i = 0u; i < lod_count; ++i )
    {
        lod_distances[i] *= float(1 << i);
    }
    m_render_world.CreateTerrainRenderComponent( description.render_component_desc[0], orientation.position, patch_dimensions, patch_size, lod_distances, sample_function_2d, entity_id );


    BoundingShapes::AxisAlignedBox box;
    box.center = 0;
    //box.extent = Float3FromUnsigned3( total_size ) / 2;
    box.extent = Math::Float3(1e30f); // totally out of the blue magic large value
    box = BoundingShapes::TransformByOrientation(box, orientation);
    m_physics_world.CreateStaticDensityBodyComponent(
        entity_id,
        box,
        sample_function_3d,
        orientation,
        description.physics_component_desc->bodies[0].bounciness,
        description.physics_component_desc->bodies[0].friction_factor
        );

    return entity_id;
}


void DogWorld::SetTerrainGrassTypes(std::vector<Graphics::RenderComponentDescription> const & descriptions, std::vector<std::array<float, Graphics::c_lod_count>> const & densities)
{
    m_render_world.SetTerrainGrassTypes(descriptions, densities);
}


AnimationTemplateID DogWorld::LoadAnimation( Animating::AnimationDescription animation_description )
{
    Animating::AnimationTemplate animation_template;

    animation_template.blend_time = animation_description.blend_time;
    animation_template.bone_masks = animation_description.bone_masks;
    animation_template.cyclic = animation_description.cyclic;
    animation_template.duration = animation_description.duration;
    animation_template.priorities = animation_description.priorities;

    animation_template.external_parameter_type = animation_description.external_parameter_type;
    animation_template.external_parameter_factor = animation_description.external_parameter_factor;

    if( !animation_description.circle_blender.nodes.empty() )
    {
        animation_template.blender_id = m_animating_world.ProvideAnimationBlender( animation_description.circle_blender );
    }
    else
    {
        animation_template.blender_id.index = c_invalid_entity_id.index;
    }
    uint32_t frame_count = 0;
    for( auto & sequence : animation_description.sequences )
    {
        uint32_t new_frame_count;
        animation_template.sequence_ids.push_back( m_animating_world.ProvideSequence( sequence, new_frame_count ) );
        //assert(frame_count == 0 || new_frame_count == frame_count);
        frame_count = new_frame_count;
    }


    return m_animating_world.m_animation_templates.Add( animation_template );
}


void DogWorld::SetEntityAnimations( EntityID id, std::string const & skeleton, std::map<AnimationStateType, AnimationTemplateID> animations )
{
    m_animating_world.CreateAnimatingComponent( skeleton, id );
    m_logic_world.SetAnimations( id, move( animations ) );
}


void DogWorld::SetEntityAmmunition( EntityID id, Logic::ThrowProperties properties, EntityTemplateID ammo_template_id )
{
    m_logic_world.SetAmmunition( id, properties, ammo_template_id );
}


void DogWorld::SetEntityHitpoints(EntityID id, float const hitpoints)
{
    m_logic_world.SetHitpoints( id, hitpoints );
}


bool DogWorld::HasPositiveHitpoints( EntityID id ) const
{
    return m_logic_world.HasPositiveHitpoints(id);
}


void DogWorld::AddLight( Graphics::LightDescription const & light_description, EntityID entity_id )
{
    m_render_world.CreateLight( light_description, entity_id );
}


void DogWorld::CreatePlayerController( EntityID entity_id )
{
    if( m_physics_world.HasRigidBodyComponent( entity_id ) )
    {
        m_logic_world.CreateForceControllerComponent( entity_id );
    }
    else
    {
        m_logic_world.CreateControllerComponent( entity_id );
    }
}


void DogWorld::SetCameraPositionAndAngles( Math::Float3 position, Math::Float3 angles )
{
    m_logic_world.SetCameraPositionAndAngles( position - Float3FromUnsigned3(m_world_reference_position), angles );
}


void DogWorld::FollowCameraRotation( EntityID entity_id )
{
    if( m_physics_world.HasRigidBodyComponent( entity_id ) )
    {
        m_logic_world.FollowCameraRotationWithTorque( entity_id );
    }
    else
    {
        m_logic_world.FollowCameraRotationkinematic( entity_id );
    }
}


void DogWorld::CreatePlayerThrowController( EntityID entity_id )
{
    m_logic_world.CreateThrowController( entity_id );
}


void DogWorld::CreatePlayerStrikeController( EntityID entity_id )
{
    m_logic_world.CreateStrikeController( entity_id );
}


namespace
{
    void SleepFor( double seconds )
    {
        using namespace std::chrono;
        std::this_thread::sleep_for( duration_cast<nanoseconds>( duration<double>( seconds ) ) ); // so we don't waste cpu cycles on querying the time
    }
}

void DogWorld::Run()
{
    auto timer = HRTimer(); // used to be StopWatch, we can switch again if MS updates their timers in the chrono header.
    timer.Start();

    double accumulated_time = 0;

    Math::Unsigned2 window_size;

    m_previous_camera_orientation = { m_logic_world.m_camera.m_position, m_logic_world.m_camera.m_rotation };
    CollisionEvents collision_events;
    GameInput game_input;
    InterfaceInput interface_input;

    size_t game_tick_counter = 0;
    while( !m_input_manager.ShouldQuit() )
    {
        auto const time_step = m_world_configuration.time_step;
        while( m_take_single_step && !m_input_manager.ShouldQuit() )
        {
            m_pause_simulation = true;
            InputAndScriptUpdate(game_input, interface_input);
            SleepFor( time_step );
            if( !m_pause_simulation )
            {
                break;
            }
        }
        // if the extra time from the previous iteration is too large, throw away some game time (slows down game-time)
        // not sure what good values are here.
        if( accumulated_time > time_step )
        {
            auto skip_steps = std::floor(2 * accumulated_time / time_step);
            auto skip_time = (skip_steps / 2) * time_step;
            accumulated_time -= skip_time;
            Log( [skip_time](){ return "Skipped " + std::to_string(skip_time * 1000) + " milliseconds of game time."; } );
            continue;
        }

        Log([game_tick_counter](){ return "Game tick number: " + std::to_string(game_tick_counter); });

        RemoveReplaceAndSpawnEntities();

        InputAndScriptUpdate(game_input, interface_input);

        collision_events = DoGameTick( std::move( collision_events ), game_input, interface_input );

        // add to it the extra time from the previous iteration
        if( !m_input_manager.WindowIsHidden() )
        {
            auto const new_window_size = m_input_manager.GetWindowSize();
            if( !Equal( window_size, new_window_size ) )
            {
                window_size = new_window_size;
                m_render_world.Resize();
                m_logic_world.m_camera.m_perspective_view.aspect_ratio = float( new_window_size.x ) / float( new_window_size.y );
            }
            Orientation camera_orientation = m_logic_world.m_camera.GetOrientation();

            // TODO: use some other condition maybe?
            if(m_render_world.m_world_configuration.render_external_debug_components)
            {
                SetPhysicsDebugVisualization();
            }

            // get the time it took to do all non-render updates and start there with rendering
            timer.Stop();
            auto const start_render_time = accumulated_time + timer.GetSeconds();
            if( !m_pause_simulation )
            {
                auto const & orientations = m_physics_world.GetOrientations();
                auto const & poses = m_animating_world.GetIndexedOffsetPoses();
                m_render_world.RenderFor( start_render_time, time_step, orientations, poses, camera_orientation, m_previous_camera_orientation, m_logic_world.m_camera.m_perspective_view );
            }
            else
            {
                auto orientations = m_physics_world.GetOrientations();
                orientations.previous_orientations = orientations.orientations;
                auto poses = m_animating_world.GetIndexedOffsetPoses();
                poses.previous_bone_states = poses.bone_states;
                m_render_world.RenderFor( start_render_time, time_step, orientations, poses, camera_orientation, m_previous_camera_orientation, m_logic_world.m_camera.m_perspective_view );
            }
            m_previous_camera_orientation = camera_orientation;
        }
        else
        {
            timer.Stop();
            SleepFor(time_step - (accumulated_time + timer.GetSeconds()));
        }

        // get the time it took for all updates and rendering and add it to the accumulated time
        timer.Stop();
        accumulated_time += timer.GetSeconds();
        timer.Start();
        // subtract the time step, so we're left with only the time we spend more than we planned
        accumulated_time -= time_step;
        game_tick_counter += 1;
    }
}


void DogWorld::SetPhysicsDebugVisualization()
{
    std::vector<BoundingShapes::OrientedBox> physics_boxes;
    std::vector<EntityID> physics_boxes_entities;
    m_physics_world.GetAllOrientedBoxes(physics_boxes, physics_boxes_entities);
    m_render_world.CreateDebugRenderComponents(physics_boxes_entities, physics_boxes);
    std::vector<BoundingShapes::Sphere> physics_spheres;
    std::vector<EntityID> physics_spheres_entities;
    m_physics_world.GetAllSpheres(physics_spheres, physics_spheres_entities);
    m_render_world.CreateDebugRenderComponents(physics_spheres_entities, physics_spheres);
}


void DogWorld::RemoveReplaceAndSpawnEntities()
{
    // remove
    if( !m_entities_to_be_pruned.empty() )
    {
        RemoveEntities( m_entities_to_be_pruned );

        m_entities_to_be_pruned.clear();
    }

    // replace
    if(!m_entity_component_replacements.entity_ids.empty())
    {
        std::vector<EntityDescription const *> replacement_descriptions;
        replacement_descriptions.reserve(m_entity_component_replacements.template_ids.size());
        for( auto id : m_entity_component_replacements.template_ids )
        {
            replacement_descriptions.push_back(&m_entity_templates.descriptions[id.index]);
        }

        ReplaceEntityComponents(m_entity_component_replacements.entity_ids, replacement_descriptions);
        m_entity_component_replacements.entity_ids.clear();
        m_entity_component_replacements.template_ids.clear();
    }

    // spawn new entities
    for( auto const & data : m_entities_to_be_spawned )
    {
        SpawnEntity( data.template_id, data.orientation, data.velocity, data.entity_id );
    }
    m_entities_to_be_spawned.clear();
}


void DogWorld::InputAndScriptUpdate(GameInput & game_input_output, InterfaceInput & interface_input_output)
{
    game_input_output.events.clear();
    interface_input_output.events.clear();
    m_input_manager.ProcessInput();
    if(m_input_manager.IsMouseVisible())
    {
        TranslateInputToGameInputIgnoreMouse(m_input_manager, m_input_manager.m_configuration.keys, game_input_output);
        TranslateInputToInterfaceInput(m_input_manager, m_input_manager.m_configuration.keys, interface_input_output);
    }
    else
    {
        TranslateInputToGameInput(m_input_manager, m_input_manager.m_configuration.keys, game_input_output);
        TranslateInputToInterfaceInputIgnoreMouseKeys(m_input_manager, m_input_manager.m_configuration.keys, interface_input_output);
    }    
    for(auto interface_event : interface_input_output.events)
    {
        switch(interface_event)
        {
            case InterfaceInputEvent::SwitchPauseSimulation:
                m_pause_simulation = !m_pause_simulation;
                break;
            case InterfaceInputEvent::SingleSimulationStep:
                m_take_single_step = !m_take_single_step;
                m_pause_simulation = m_take_single_step;
                break;
            case InterfaceInputEvent::SwitchControlMode:
            {
                auto new_state = !m_logic_world.m_camera.m_angle_control;
                m_input_manager.SetMouseVisible( !new_state );
                m_logic_world.m_camera.m_angle_control = new_state;
                break;
            }
            case InterfaceInputEvent::PointerInteract:
            {
                //Math::Float3 pointer_position_3d = TransformVertical(m_logic_world.m_camera.m_perspective_view, 0);
                Math::Float3 pointer_position_3d = TransformVertical(m_logic_world.m_camera.m_perspective_view, interface_input_output.pointer_position);
                pointer_position_3d = Math::Rotate(pointer_position_3d, m_logic_world.m_camera.m_rotation);
                auto position = m_physics_world.CastRayOnStaticEntities(BoundingShapes::RayFromStartAndDirection(m_logic_world.m_camera.m_position, pointer_position_3d));
                this->SpawnEntity( m_entity_templates.descriptions.front(), {position, Math::Identity()}, 0);
                break;
            }
            default:
                break;
        }
    }

    if( m_script_callback ) m_script_callback( m_world_configuration.time_step );
}


CollisionEvents DogWorld::DoGameTick( CollisionEvents collision_events, GameInput const & game_input, InterfaceInput const & interface_input )
{
    auto const time_step = m_world_configuration.time_step;
    {
        m_logic_world.UpdateCamera( time_step, interface_input.camera, m_physics_world.GetOrientations() );
        auto camera_position = m_logic_world.m_camera.m_position;
        auto camera_position_absolute = Math::Abs(camera_position);
        if( camera_position_absolute.x > c_move_reference_point_threshold ||
            camera_position_absolute.y > c_move_reference_point_threshold ||
            camera_position_absolute.z > c_move_reference_point_threshold )
        {
            Math::Float3 adjustment = float{-c_move_reference_point_threshold} * Round(camera_position / float{c_move_reference_point_threshold});
            m_physics_world.AdjustAllPositions(adjustment);
            m_logic_world.AdjustAllPositions(adjustment);
            m_render_world.AdjustAllPositions(adjustment);
            m_previous_camera_orientation.position += adjustment;
        }
    }
    if( !m_pause_simulation )
    {
        {
            auto const & moving_entities = m_physics_world.GetMovingEntities();
            IndexedVelocities velocities;
            velocities.indices = moving_entities.entity_to_element;
            velocities.velocities = moving_entities.velocities;
            velocities.angular_velocity = moving_entities.angular_velocities;

            auto const & orientations = m_physics_world.GetOrientations();

            auto const & logic_output = m_logic_world.UpdateGameLogic(
                time_step,
                m_entity_id_generator,
                game_input,
                orientations,
                velocities,
                m_animating_world.GetIndexedAbsolutePoses(),
                collision_events);

            Append(m_entities_to_be_spawned, logic_output.entities_to_be_spawned);
            Append(m_entity_component_replacements.entity_ids, logic_output.entity_component_replacements.entity_ids);
            Append(m_entity_component_replacements.template_ids, logic_output.entity_component_replacements.template_ids);
            Append(m_entities_to_be_pruned, logic_output.entities_to_be_pruned);

            m_animating_world.ApplyInstructions(logic_output.animating_instructions);

            m_animating_world.UpdateStatesWithExternalParameters(orientations, velocities, m_logic_world.m_camera.m_angles);
            m_animating_world.UpdateAnimations(time_step);

            m_physics_world.CalculateVelocities(time_step);
            m_physics_world.CopyCurrentToPrevious();
            m_physics_world.RemoveNonCollidingEntityPairs(logic_output.expired_non_colliding_entity_pairs);
            m_physics_world.AddNonCollidingEntityPairs(logic_output.new_non_colliding_entity_pairs);
            m_physics_world.UpdateOrientations(logic_output.positions, logic_output.rotations);
            collision_events = m_physics_world.UpdateBodies(logic_output.forces, logic_output.torques, logic_output.rotation_constraints, logic_output.velocity_constraints, logic_output.angular_velocity_constraints, time_step);
        }

        {
            auto const & orientations = m_physics_world.GetOrientations();
            auto const & poses = m_animating_world.GetIndexedAbsolutePoses();
            EntityPositions entity_positions;
            EntityRotations entity_rotations;
            m_logic_world.PostProcessOrientations(orientations, poses, entity_positions, entity_rotations);
            m_physics_world.UpdateOrientations(entity_positions, entity_rotations);

            // Update terrain around camera target or camera
            if (m_logic_world.m_camera.m_target_entity.index != c_invalid_entity_id.index)
            {
                // Get camera target entity
                auto camera_target = m_logic_world.m_camera.m_target_entity;

                // Get orientation for target
                auto orientation_index = orientations.indices[camera_target.index];
                auto target_orientation = orientations.orientations[orientation_index];
                m_render_world.UpdateTerrain(target_orientation.position);
            }
            else
            {
                m_render_world.UpdateTerrain(m_logic_world.m_camera.m_position);
            }
        }

    }

    return collision_events;
}


void DogWorld::SetCameraTarget( EntityID entity_id )
{
    m_logic_world.SetCameraTarget( entity_id );
}


void DogWorld::SetPhysicsConfiguration(Physics::WorldConfiguration const & config)
{
    m_physics_world.SetWorldConfiguration(config);
}


void DogWorld::SetGravity( Math::Float3 const & gravity )
{
    m_physics_world.m_gravity = gravity;
}


Math::Float3 const & DogWorld::GetGravity()
{
    return m_physics_world.m_gravity;
}


Orientation DogWorld::GetEntityOrientation(EntityID const entity_id)
{
    Orientation result;
    GetOrientations(CreateRange(&entity_id, 1), CreateRange(&result, 1));
    return result;
}


void DogWorld::GetOrientations(Range<EntityID const*> entity_ids, Range<Orientation *> orientations)
{
    if(Size(entity_ids) != Size(orientations))
    {
        throw DogDealerException("Incorrect size for output orientations given to " __FUNCTION__ ".", false);
    }
    auto const & indexed_orientations = m_physics_world.GetOrientations();

    auto count = Size(entity_ids);
    for (auto i = 0u; i < count; ++i)
    {
        auto orientation_index = GetOptional(indexed_orientations.indices, entity_ids[i].index);
        if(orientation_index == c_invalid_index)
        {
            throw DogDealerException("The entity given to " __FUNCTION__ " does not have a valid orientation.", false);
        }
        auto orientation = indexed_orientations.orientations[orientation_index];

        orientation.position += Float3FromUnsigned3(m_world_reference_position);
        orientations[i] = orientation;
    }
}


Math::Float3 DogWorld::FindRestingPositionOnEntity( Math::Float3 starting_position, EntityID target_entity ) const
{
    return m_physics_world.FindRestingPositionOnEntity( starting_position - Float3FromUnsigned3(m_world_reference_position), target_entity ) - Float3FromUnsigned3(m_world_reference_position);
}
