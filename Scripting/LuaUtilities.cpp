#include "LuaUtilities.h"

#include "LuaStdTypes.h"
#include "LuaUtilitiesTypes.h"
#include "LuaMathTypes.h"
#include "LuaGameLogic.h"

#include <Windows\WindowsInclude.h>

#include <lua\include\lua.hpp>
#include <lua\include\luawrapperutil.hpp>

#include <Graphics\ResourceDescriptions.h>
#include <Graphics\TextureFiltering.h>
#include <Animating\ResourceDescriptions.h>
#include <Physics\ConstraintSolverType.h>

#include <World\DogWorld.h>

#include <bitset>
// disable warning: "symbol will be dynamically initialized (implementation limitation)"
#pragma warning(disable : 4592)
namespace
{
    namespace Identifiers
    {
        std::string const
            // entities
            c_graphics_comp = "graphics",
            c_physics_comp = "physics",
            c_animating_comp = "animating",
            c_logic_component = "logic",

            // physics components
            c_static = "static",
            c_mass = "mass",
            c_bouncincess = "bounciness",
            c_friction = "friction",
            c_collision_file = "collision_file",
            c_lock_rotation = "lock_rotation",

            // physics configuration
            c_position_correction_iterations = "position_correction_iterations",
            c_velocity_correction_iterations = "velocity_correction_iterations",
            c_position_correction_fraction = "position_correction_fraction",
            c_penetration_depth_tolerance = "penetration_depth_tolerance",
            c_angular_velocity_correction_fraction = "angular_velocity_correction_fraction",
            c_fixed_fraction_velocity_loss_per_second = "fixed_fraction_velocity_loss_per_second",
            c_precondition_constraint_solver = "precondition_constraint_solver",
            c_constraint_solver_type = "constraint_solver_type",

            // graphics components
            c_mesh = "mesh",
            c_display_technique = "technique",
            c_shadow_display_technique = "shadow_" + c_display_technique,
            c_alpha_to_coverage = "alpha_to_coverage",

            // graphics configuration
            c_render_sample_count = "render_sample_count",
            c_maximum_texture_filtering_quality = "maximum_texture_filtering_quality",
            c_display_bounding_boxes = "display_bounding_boxes",

            // textures
            c_textures = "textures",
            c_diffuse_texture = "diffuse",
            c_normal_texture = "normal",
            c_specular_texture = "specular",

            // shaders
            c_vertex_shader = "vertex_shader",
            c_hull_shader = "hull_shader",
            c_domain_shader = "domain_shader",
            c_geometry_shader = "geometry_shader",
            c_pixel_shader = "pixel_shader",
            c_compute_shader = "compute_shader",

            // animating components
            c_skeleton = "skeleton",
            c_animations = "animations",

            // animation blenders
            c_circle_blender = "circle_blender";
    }

    const std::map<const std::string, AnimationStateType> c_animation_state_type_lookup =
    {
        { "idle", AnimationStateType::Idle },
		{ "turning", AnimationStateType::Turning },
        { "motion", AnimationStateType::Motion },
        { "throw", AnimationStateType::Throw },

        { "ready_strike_left", AnimationStateType::ReadyStrikeLeft },
        { "hold_strike_left", AnimationStateType::HoldStrikeLeft },
        { "strike_left", AnimationStateType::StrikeLeft },

        { "ready_strike_right", AnimationStateType::ReadyStrikeRight },
        { "hold_strike_right", AnimationStateType::HoldStrikeRight },
        { "strike_right", AnimationStateType::StrikeRight },

        { "ready_strike_up", AnimationStateType::ReadyStrikeUp },
        { "hold_strike_up", AnimationStateType::HoldStrikeUp },
        { "strike_up", AnimationStateType::StrikeUp },

        { "ready_strike_down", AnimationStateType::ReadyStrikeDown },
        { "hold_strike_down", AnimationStateType::HoldStrikeDown },
        { "strike_down", AnimationStateType::StrikeDown },

        { "block_left", AnimationStateType::BlockLeft },
        { "block_right", AnimationStateType::BlockRight },
        { "block_up", AnimationStateType::BlockUp },
        { "block_down", AnimationStateType::BlockDown },

        { "torso_tilting", AnimationStateType::TorsoTilting },

        { "strike_rebound", AnimationStateType::StrikeRebound },
        { "flinch", AnimationStateType::Flinch },
        { "dying", AnimationStateType::Dying },
        { "dead", AnimationStateType::Dead },

        // maybe just provide a case insensitive compare function
        { "Idle", AnimationStateType::Idle },
		{ "Turning", AnimationStateType::Turning },
        { "Motion", AnimationStateType::Motion },
        { "Throw", AnimationStateType::Throw },

        { "ReadyStrikeLeft", AnimationStateType::ReadyStrikeLeft },
        { "HoldStrikeLeft", AnimationStateType::HoldStrikeLeft },
        { "StrikeLeft", AnimationStateType::StrikeLeft },

        { "ReadyStrikeRight", AnimationStateType::ReadyStrikeRight },
        { "HoldStrikeRight", AnimationStateType::HoldStrikeRight },
        { "StrikeRight", AnimationStateType::StrikeRight },

        { "ReadyStrikeUp", AnimationStateType::ReadyStrikeUp },
        { "HoldStrikeUp", AnimationStateType::HoldStrikeUp },
        { "StrikeUp", AnimationStateType::StrikeUp },

        { "ReadyStrikeDown", AnimationStateType::ReadyStrikeDown },
        { "HoldStrikeDown", AnimationStateType::HoldStrikeDown },
        { "StrikeDown", AnimationStateType::StrikeDown },

        { "BlockLeft", AnimationStateType::BlockLeft },
        { "BlockRight", AnimationStateType::BlockRight },
        { "BlockUp", AnimationStateType::BlockUp },
        { "BlockDown", AnimationStateType::BlockDown },

        { "TorsoTilting", AnimationStateType::TorsoTilting },

        { "StrikeRebound", AnimationStateType::StrikeRebound },
        { "Flinch", AnimationStateType::Flinch },
        { "Dying", AnimationStateType::Dying },
        { "Dead", AnimationStateType::Dead }

    };

    const std::map<std::string, Animating::ExternalParameterType> c_animating_external_parameter_type =
    {
        { "none", Animating::ExternalParameterType::None},
        { "horizontal_movement", Animating::ExternalParameterType::HorizontalMovement },
        { "camera_angle", Animating::ExternalParameterType::CameraAngle },
		{ "rotational_offset", Animating::ExternalParameterType::RotationalOffset }
    };

    Graphics::DisplayTechniqueDescription ReadDisplayTechniqueTable( lua_State * L, int table_index )
    {
        using namespace Identifiers;
        Graphics::DisplayTechniqueDescription description;
        table_index = lua_absindex( L, table_index );

        lua_getfield( L, table_index, c_vertex_shader.c_str() );
        auto luastring = lua_tostring( L, -1 );
        if( luastring != nullptr )
        {
            description.vertex_shader = luastring;
            lua_pop( L, 1 );
        }

        lua_getfield( L, table_index, c_pixel_shader.c_str() );
        luastring = lua_tostring( L, -1 );
        if( luastring != nullptr )
        {
            description.pixel_shader = luastring;
            lua_pop( L, 1 );
        }

        return description;
    }

    void IterateThroughRenderComponentTable( lua_State* L, int table_index, Graphics::RenderComponentDescription& description )
    {
        using namespace Identifiers;

        auto abs_index = lua_absindex( L, table_index );

        lua_pushnil( L );  /* first key */
        while( lua_next( L, abs_index ) != 0 )
        {
            /* uses 'key' (at index -2) and 'value' (at index -1) */
            std::string const key = lua_tostring( L, -2 );
            if( lua_istable( L, -1 ) )
            {
                if( key == c_display_technique )
                {
                    description.technique = ReadDisplayTechniqueTable( L, -1 );
                }
                else if( key == c_shadow_display_technique )
                {
                    description.shadow_technique = ReadDisplayTechniqueTable( L, -1 );
                }
                else
                {
                    IterateThroughRenderComponentTable( L, -1, description );
                }
            }
            else if( lua_isstring( L, -1 ) )
            {
                std::string const value = lua_tostring( L, -1 );

                if( key == c_mesh ) description.mesh = value;

                // textures
                else if( key == c_diffuse_texture ) description.textures.emplace_back( Graphics::TextureType::Diffuse, value );
                else if( key == c_normal_texture ) description.textures.emplace_back( Graphics::TextureType::Normal, value );
                else if( key == c_specular_texture ) description.textures.emplace_back( Graphics::TextureType::Specular, value );

                //else luaL_error( L, "Unkown key value: \"%s\" with value: \"%s\"", key.c_str( ), value.c_str( ) );
            }
            else if( lua_isboolean( L, -1 ) )
            {
                if( key == c_alpha_to_coverage )
                {
                    description.alpha_to_coverage = !!lua_toboolean( L, -1 );
                }
            }
            /* removes 'value'; keeps 'key' for next iteration */
            lua_pop( L, 1 );
        }
    }

    Animating::CircleBlenderDescription ReadCircleBlender(lua_State* L, int table_index)
    {
        Animating::CircleBlenderDescription description;

        auto length = lua_rawlen( L, table_index );
        description.nodes = std::vector<Animating::AnimationBlenderNodeDescription>(length);

        for( auto i = 0u; i < length; ++i )
        {
            lua_rawgeti( L, table_index, i + 1u );

            // Iterate over blender nodes
            assert(lua_istable(L, -1));

            Animating::AnimationBlenderNodeDescription node;
            node.center = luaU_checkfield<Math::Float2>(L, -1, "center");
            node.radius = luaU_checkfield<float>(L, -1, "radius");
            description.nodes[i] = node;

            // pop the node [table?]
            lua_pop(L, 1);
        }

        std::string const parameter_type_key = luaU_checkfield<std::string>(L, table_index, "input_type");
        description.parameter_type = c_animating_external_parameter_type.at(parameter_type_key);

        return description;
    }


    auto ReadPhysicsBodyComponent( lua_State * L, int table_index, Physics::ComponentDescription::Body & description )
    {
        using namespace Identifiers;
        // if collision file is empty we will have a non-colliding physics component and the bounciness and friction factor won't matter
        description.collision_file = luaU_optfield<std::string>( L, table_index, c_collision_file.c_str() );
        description.bounciness = luaU_optfield<float>( L, table_index, c_bouncincess.c_str(), 1 );
        description.friction_factor = luaU_optfield<float>( L, table_index, c_friction.c_str(), 0.5 );

        description.lock_rotation = luaU_optfield<bool>( L, table_index, c_lock_rotation.c_str(), false );

        if( luaU_checkfield<bool>( L, table_index, c_static.c_str() ) )
        {
            description.mass = std::numeric_limits<decltype( description.mass )>::infinity();
            return Physics::BodyType::Static;
        }
        else
        {
            description.mass = luaU_optfield<float>( L, table_index, c_mass.c_str(), 0 );
            if( description.mass < 0 )
            {
                luaL_error( L, "Invalid mass provided for a physics component" );
                assert(false);
                return Physics::BodyType(-1); // will never get here, because luaL_error doesn't return
            }
            else if( description.mass == 0 ) // means mass was nil or 0
            {
                return Physics::BodyType::Kinematic;
            }
            else
            {
                return Physics::BodyType::Rigid;
            }
        }
    }


    auto ReadPhysicsBodyComponents(lua_State * L, int table_index, std::vector<Physics::ComponentDescription::Body> & descriptions)
    {
        auto length = lua_rawlen(L, table_index);
        descriptions.reserve(length);
        auto type = Physics::BodyType::Static;
        for(auto i = 0u; i < length; i++)
        {
            // get the body description table
            lua_rawgeti(L, table_index, i + 1);
            // read it
            descriptions.emplace_back();
            auto current_type = ReadPhysicsBodyComponent(L, -1, descriptions.back());
            if(i == 0)
            {
                type = current_type;
            }
            else if(type != current_type)
            {
                luaL_error(L, "All bodies of an entity have to be of the same type.");
            }
            // pop the table
            lua_pop(L, 1);
        }
        return type;
    }


    auto ReadPhysicsConnection(lua_State * L, int table_index)
    {
        auto type_string = luaU_checkfield<std::string>(L, table_index, "connection_type");
        Physics::Connection connection;
        connection.type = Physics::ConnectionTypeFromString(type_string);
        if(connection.type == Physics::ConnectionType::Invalid)
        {
            luaL_error(L, "Invalid connection type: '%s'", type_string.c_str());
        }
        connection.parent = luaU_checkfield<uint32_t>(L, table_index, "parent_index");
        connection.child = luaU_checkfield<uint32_t>(L, table_index, "child_index");
        return connection;
    }


    void ReadPhysicsConnections(lua_State * L, int table_index, std::vector<Physics::Connection> & connections)
    {
        auto length = lua_rawlen(L, table_index);
        connections.reserve(length);
        for(auto i = 0u; i < length; i++)
        {
            // get the body description table
            lua_rawgeti(L, table_index, i + 1);
            // read it
            connections.emplace_back(ReadPhysicsConnection(L, -1));
            // pop the table
            lua_pop(L, 1);
        }
    }


    void ReadPhysicsComponent(lua_State * L, int table_index, Physics::ComponentDescription & description)
    {
        assert(lua_istable(L, table_index));
        table_index = lua_absindex(L, table_index);
        lua_getfield(L, table_index, "bodies");
        if(lua_istable(L, -1))
        {
            // read multiple bodies
            description.body_type = ReadPhysicsBodyComponents(L, -1, description.bodies);
            lua_getfield(L, table_index, "connections");
            if(lua_istable(L, -1))
            {
                ReadPhysicsConnections(L, -1, description.connections);
            }
            // pop 'connections' table
            lua_pop(L, 1);
        }
        else
        {
            // read single body from original table
            description.bodies.emplace_back();
            description.body_type = ReadPhysicsBodyComponent(L, table_index, description.bodies.back());
        }
        // pop the 'bodies' table
        lua_pop(L, 1);
    }
}


void ReadGraphicsConfiguration( lua_State * L, int table_index, Graphics::WorldConfiguration& configuration )
{
    using namespace Graphics;
    configuration.render_sample_count = luaU_checkfield<unsigned>( L, table_index, Identifiers::c_render_sample_count.c_str() );
    if( std::bitset<32>( configuration.render_sample_count ).count() != 1)
    {
        luaL_error(L, "Wrong value for render_sample_count");
    }
    auto texture_filtering_setting = luaU_checkfield<unsigned>( L, table_index, Identifiers::c_maximum_texture_filtering_quality.c_str() );
    switch(texture_filtering_setting)
    {
        case 0:
            configuration.maximum_texture_filtering_quality = TextureFiltering::NearestNeighbour;
            break;
        case 1:
            configuration.maximum_texture_filtering_quality = TextureFiltering::Linear;
            break;
        case 2:
            configuration.maximum_texture_filtering_quality = TextureFiltering::Anisotropic_2x;
            break;
        case 4:
            configuration.maximum_texture_filtering_quality = TextureFiltering::Anisotropic_4x;
            break;
        case 8:
            configuration.maximum_texture_filtering_quality = TextureFiltering::Anisotropic_8x;
            break;
        case 16:
            configuration.maximum_texture_filtering_quality = TextureFiltering::Anisotropic_16x;
            break;
        default:
            luaL_error(L, "Wrong value for maximum_texture_filtering_quality");
    }

    configuration.display_bounding_boxes = luaU_optfield<bool>(L, table_index, Identifiers::c_display_bounding_boxes.c_str());
    configuration.force_wire_frame = luaU_optfield<bool>(L, table_index, "force_wire_frame");
    configuration.render_external_debug_components = luaU_optfield<bool>(L, table_index, "render_external_debug_components");
}


// assuming the render component table is on the top of the stack
std::vector<Graphics::RenderComponentDescription> ReadRenderComponents( lua_State * L, int table_index )
{
    auto abs_index = lua_absindex( L, table_index );

    std::vector<Graphics::RenderComponentDescription> descriptions;

    auto length = lua_rawlen( L, abs_index );
    if( length == 0 )
    {
        descriptions.resize( 1 );
        IterateThroughRenderComponentTable( L, abs_index, descriptions.front() );
    }
    else
    {
        descriptions.resize( length );

        for( auto i = 0u; i < length; i++ )
        {
            lua_rawgeti( L, abs_index, i + 1u );
            IterateThroughRenderComponentTable( L, -1, descriptions[i] );
            lua_pop( L, 1 ); // pop the table?
        }
    }

    return descriptions;
}


void ReadPhysicsConfiguration( lua_State * L, int table_index, Physics::WorldConfiguration& configuration )
{
    using namespace Identifiers;
    configuration.position_correction_iterations = luaU_checkfield<unsigned>( L, table_index, c_position_correction_iterations.c_str() );
    configuration.velocity_correction_iterations = luaU_checkfield<unsigned>( L, table_index, c_velocity_correction_iterations.c_str() );
    configuration.position_correction_fraction = luaU_checkfield<float>( L, table_index, c_position_correction_fraction.c_str() );
    configuration.angular_velocity_correction_fraction = luaU_checkfield<float>( L, table_index, c_angular_velocity_correction_fraction.c_str() );
    configuration.fixed_fraction_velocity_loss_per_second = luaU_checkfield<float>( L, table_index, c_fixed_fraction_velocity_loss_per_second.c_str() );
    configuration.penetration_depth_tolerance = luaU_checkfield<float>( L, table_index, c_penetration_depth_tolerance.c_str() );
    configuration.warm_start_factor = luaU_optfield<float>( L, table_index, "warm_start_factor", 0.f );
    configuration.persitent_contact_expiry_age = luaU_optfield<uint8_t>( L, table_index, "persitent_contact_expiry_age", 5);
    configuration.solve_parallel = luaU_optfield<bool>( L, table_index, "solve_parallel", false );
    if(luaU_fieldis<float>( L, table_index, "relaxation_factor" ))
    {
        auto single_relaxation_factor = luaU_getfield<float>(L, table_index, "relaxation_factor");
        configuration.solver_relaxation_factor ={single_relaxation_factor, single_relaxation_factor};
    }
    else
    {
        configuration.solver_relaxation_factor = luaU_optfield<MinMax<float>>(L, table_index, "relaxation_factor", MinMax<float>{1,1});
    }
    if(configuration.solver_relaxation_factor.min <= 0 || configuration.solver_relaxation_factor.min >= 2||
       configuration.solver_relaxation_factor.max <= 0 || configuration.solver_relaxation_factor.max >= 2)
    {
        luaL_error(L, "relaxation_factor has an invalid value. The valid range is (0, 2).");
    }
    configuration.minimal_island_size = luaU_optfield<uint32_t>( L, table_index, "minimal_island_size", 128 );
    auto solver_name = luaU_optfield<std::string>( L, table_index, c_constraint_solver_type.c_str(), "Implicit" );
    if( solver_name == "Implicit" )
    {
        configuration.constraint_solver_type = Physics::ConstraintSolverType::Implicit;
    }
    else
    {
        luaL_error( L, "Invalid constraint solver type provided." );
    }
}


std::unique_ptr<Physics::ComponentDescription> ReadPhysicsComponents( lua_State * L, int table_index )
{
    assert( lua_istable( L, table_index ) );
    auto description = std::make_unique<Physics::ComponentDescription>();
    ReadPhysicsComponent( L, table_index, *description );
    return description;
}


// Read the sequence used by an animation.
// If a circle blender is being used, read all the involved sequences
void ReadAnimatingCircleBlenderAndSequences(lua_State * L,
                                            int table_index,
                                            Animating::AnimationDescription & description)
{
    using namespace Identifiers;

    // Check for an existing circle blender
    lua_getfield(L, table_index, c_circle_blender.c_str());
    if (lua_istable(L, -1))
    {
        // If it exists, read the blender
        description.circle_blender = ReadCircleBlender(L, -1);

        // Read the multiple sequences controlled by the blender
        lua_getfield(L, table_index, "sequences");
        auto number_of_sequences = lua_rawlen(L, -1);
        assert(number_of_sequences == description.circle_blender.nodes.size());
        for (auto i = 0u; i < number_of_sequences; ++i)
        {
            lua_rawgeti(L, -1, i + 1); // get the sequence name
            description.sequences.emplace_back(luaL_checkstring(L, -1));
            lua_pop(L, 1); // pop the sequence name
        }
        lua_pop(L, 1); // pop the sequences array/table
    }
    else // get a single sequence name
    {
        // Otherwise read the single sequence used by the animation
        description.sequences.emplace_back(luaU_checkfield<std::string>(L, table_index, "sequence"));
    }
    lua_pop(L, 1); // pop the motion blender table
}


void ReadAnimatingBoneMask(lua_State * L,
                        std::vector<float> & bone_weights)
{
    assert(lua_istable(L, -1) && "Attempting to read bone mask which is not a table!");
    auto bone_count = lua_rawlen(L, -1);
    for (auto i = 0u; i < bone_count; ++i)
    {
        lua_rawgeti(L, -1, i + 1);
        bone_weights.push_back(luaU_check<float>(L, -1));
        lua_pop(L, 1); // pop the number
    }
    lua_pop(L, 1); // pop the bone mask array/table
}


void ReadAnimatingBoneMasksAndPriorities(lua_State * L,
                                        int table_index,
                                        Animating::AnimationDescription & description)
{
    using namespace Identifiers;

    // Keep track of whether one or multiple
    // bone weight masks are being used for the animation
    size_t number_of_priorities = 1;

    // Check for a table of priorities
    lua_getfield(L, table_index, "priorities");
    if (lua_istable(L, -1))
    {
        // If a table exists, multiple bone masks
        // with different priorities are being used
        number_of_priorities = lua_rawlen(L, -1);

        // Read all priorities
        for (auto i = 0u; i < number_of_priorities; ++i)
        {
            lua_rawgeti(L, -1, i + 1); // get the priority
            description.priorities.emplace_back(luaL_checkunsigned(L, -1));

            lua_pop(L, 1); // pop the sequence name
        }
    }
    else
    {

        // Otherwise read the single priority used by the animation
        description.priorities.emplace_back(luaU_checkfield<unsigned>(L, table_index, "priority"));

        // Read the single bone mask used by the animation
        lua_getfield(L, table_index, "bone_mask");
        description.bone_masks.resize(1);
        ReadAnimatingBoneMask(L, description.bone_masks[0]);
    }

    lua_pop(L, 1); // pop the priority array/table

    // If the animation uses multiple blend masks, read all of them
    if (number_of_priorities > 1)
    {
        // Get the table of bone masks
        lua_getfield(L, table_index, "bone_masks");
        assert(lua_istable(L, -1) && "Multiple animation priorities defined without multiple bone masks!");

        // Ensure that there exists one priority for each bone mask
        auto number_of_bone_masks = lua_rawlen(L, -1);
        assert((number_of_bone_masks == number_of_priorities) && "Unequal count of priorities and bone masks defined!");

        // Reserve space for all bone masks
        description.bone_masks.resize(number_of_priorities);

        // Read all bone masks
        for (auto i = 0u; i < number_of_bone_masks; ++i)
        {
            lua_rawgeti(L, -1, i + 1); // get the bone mask

            ReadAnimatingBoneMask(L, description.bone_masks[i]);
        }
        lua_pop(L, 1); // pop the bone mask array/table
    }
}


// Read the optional external input parameter for an animation state description,
// used for the calculation of a speed modifier from logic input.
void ReadAnimatingExternalParameter(lua_State * L,
                                    int table_index,
                                    Animating::AnimationDescription & description)
{
    // Get optional parameter type field
    std::string const parameter_type_key = luaU_optfield<std::string>(L, table_index, "input_type", "");

    // If set, also get parameter factor, otherwise keep default values
    if (parameter_type_key != "")
    {
        description.external_parameter_type = c_animating_external_parameter_type.at(parameter_type_key);
        description.external_parameter_factor = luaU_checkfield<float>(L, table_index, "input_factor");
    }
}


Animating::AnimationDescription ReadAnimation( lua_State * L, int table_index )
{
    assert( lua_istable( L, table_index ));
    Animating::AnimationDescription description;
    using namespace Identifiers;
    table_index = lua_absindex( L, table_index );

    ReadAnimatingCircleBlenderAndSequences(L, table_index, description);

    description.blend_time = luaU_checkfield<float>(L, table_index, "blend_time");
    description.cyclic = luaU_checkfield<bool>(L, table_index, "cyclic" );
    description.duration = luaU_checkfield<float>(L, table_index, "duration" );

    // Get optional external logic input setting
    ReadAnimatingExternalParameter(L, table_index, description);

    ReadAnimatingBoneMasksAndPriorities(L, table_index, description);
    return description;
}


Animating::ComponentDescription ReadAnimatingComponent( lua_State * L, int table_index )
{
    Animating::ComponentDescription description;
    table_index = lua_absindex( L, table_index );

    using namespace Identifiers;

    description.skeleton = luaU_checkfield<std::string>( L, table_index, c_skeleton.c_str() );

    lua_getfield(L, table_index, "animations" );
    assert( lua_istable( L, -1 ) );
    auto animation_table_index = lua_absindex( L, -1 );

    for( lua_pushnil( L ); lua_next( L, animation_table_index ) != 0; lua_pop(L, 1) )
    {
        // SCAMP ??
        // using the key as animation state type is maybe a bit scampy.
        std::string const key = luaL_checkstring( L, -2 );
        description.animations.emplace( c_animation_state_type_lookup.at( key ), ReadAnimation( L, -1 ) );
    }

    return description;
}


EntityDescription ReadEntityDescription( lua_State * L, int table_index )
{
    EntityDescription description;
    using namespace Identifiers;
    lua_getfield( L, table_index, c_physics_comp.c_str() );
    if( lua_istable( L, -1 ) )
    {
        description.physics_component_desc = ReadPhysicsComponents( L, -1 );
    }
    lua_pop(L, 1);

    lua_getfield( L, table_index, c_graphics_comp.c_str() );
    if( lua_istable( L, -1 ) )
    {
        description.render_component_desc = ReadRenderComponents( L, -1 );
    }
    lua_pop(L, 1);

    lua_getfield( L, table_index, c_animating_comp.c_str() );
    if( lua_istable( L, -1 ) )
    {
        description.animating_component_desc = std::make_unique<Animating::ComponentDescription>( ReadAnimatingComponent( L, -1 ) );
    }
    lua_pop(L, 1);

    lua_getfield( L, table_index, c_logic_component.c_str() );
    if( luaU_is<Logic::MobileComponentDescription>( L, -1 ) )
    {
        description.logic_mobile_component_desc = std::make_unique<Logic::MobileComponentDescription>( luaU_check<Logic::MobileComponentDescription>(L, -1) );
    }
    else if( luaU_is<Logic::ItemComponentDescription>( L, -1 ) )
    {
        description.logic_item_component_desc = std::make_unique<Logic::ItemComponentDescription>( luaU_check<Logic::ItemComponentDescription>( L, -1 ) );
    }

    // SCAMP
    if( lua_istable(L, -1) && luaU_fieldis<float>(L, -1, "damage_value") )
    {
        description.logic_damage_dealer_component_description = std::make_unique<Logic::DamageDealerComponentDescription>(luaU_getfield<float>(L, -1, "damage_value"));
    }
    lua_pop(L, 1);

    description.name = luaU_optfield<std::string>(L, table_index, "name", "Unnamed entity");

    return description;
}



void ReadNoiseParameters( lua_State * L, int index, std::vector<NoiseParameters<Math::Float3>>& noise3d )
{
    if( !lua_istable( L, index ) ) luaL_error(L, "Trying to read Noise Parameters from a non-table entry.");
    auto parameter_count = lua_rawlen( L, index );
    noise3d.resize(parameter_count);
    for( auto i = 0u; i < parameter_count; ++i )
    {
        lua_rawgeti( L, index, i + 1 );
        noise3d[i] = luaU_check<NoiseParameters<Math::Float3>>( L, -1 );
        lua_pop( L, 1 ); // pop the number
    }
}


void ReadNoiseParameters( lua_State * L, int index, std::vector<NoiseParameters<Math::Float2>>& noise2d )
{
    if( !lua_istable( L, index ) ) luaL_error(L, "Trying to read Noise Parameters from a non-table entry.");
    auto parameter_count = lua_rawlen( L, index );
    noise2d.resize(parameter_count);
    for( auto i = 0u; i < parameter_count; ++i )
    {
        lua_rawgeti( L, index, i + 1 );
        noise2d[i] = luaU_check<NoiseParameters<Math::Float2>>( L, -1 );
        lua_pop( L, 1 ); // pop the number
    }
}
