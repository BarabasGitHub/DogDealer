#include "LuaDogWorld.h"

#include <World\DogWorld.h>

#include <lua\include\luawrapper.hpp>
#include <lua\include\luawrapperutil.hpp>

#include "LuaGameLogic.h"
#include "LuaGraphics.h"
#include "LuaMathTypes.h"
#include "LuaInputWorld.h"
#include "LuaStdTypes.h"
#include "LuaUtilities.h"
#include "LuaUtilitiesTypes.h"

#include <Math\MathFunctions.h>

#include <Input\WinMainInput.h>

#include <Utilities\StringUtilities.h>
#include <Utilities\DogDealerException.h>
#include <Windows\WindowsErrorsToException.h>

namespace
{
    template <typename FunctionType>
    void TryAndCatch(lua_State * L, FunctionType const & function)
    {
        try
        {
            function();
        }
        catch( WindowsException const & e )
        {
            luaL_error( L, "Windows error %s: %s", IntegerToHexadecimalString( e.ErrorCode() ).c_str(), e.what() );
        }
        catch( DogDealerException const & e )
        {
            luaL_error( L, "Dog Dealer exception: %s", e.what() );
        }
        catch( std::exception const & e )
        {
            luaL_error( L, "C++ exception: %s", e.what() );
        }

    }

    // helper function to check for the existance of a physics component. Raises a lua error if one is not found.
    void CheckForPhysicsComponent( lua_State * L, EntityDescription const & description )
    {
        if( description.physics_component_desc == nullptr )
        {
            luaL_error( L, "No physics component found. Entities require a physics component!" );
        }
    }


    int CreateAndShowWindow( lua_State * L )
    {
        DogWorld* dog_world = luaW_check<DogWorld>( L, 1 );
        std::wstring title = lua_isstring( L, -1 ) != 0 ? utf8utf16Converter().from_bytes( lua_tostring( L, -1 ) ) :
            L"I have not been given a name by my creator. Now I am a very sad window.";

        TryAndCatch(L, [&](){dog_world->CreateAndShowWindow( s_win_main_input.instance, s_win_main_input.show_command, move( title ) );});

        return 0;
    }


    int SetUpdateFunction( lua_State * L )
    {
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        luaL_checktype( L, 2, LUA_TFUNCTION );
        auto function_reference = luaL_ref( L, LUA_REGISTRYINDEX );
        dog_world->m_script_callback = [L, function_reference]( double const time_step )
        {
            lua_rawgeti( L, LUA_REGISTRYINDEX, function_reference );
            lua_pushvalue( L, 1 ); // push the dogworld
            lua_pushnumber( L, time_step );
            lua_call( L, 2, 0 ); // call lua function with 2 inputs and 0 results
            //lua_toboolean( L, -1 ); // get if we want to quit?
        };
        return 0;
    }


    int Run( lua_State * L )
    {
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        TryAndCatch(L, [=](){dog_world->Run();});
        return 0;
    }


    int RegisterEntityTemplate( lua_State * L )
    {
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        auto entity_description = ReadEntityDescription( L, 2 );

        auto id = dog_world->m_entity_templates.Add( std::move( entity_description ) );
        luaU_push( L, id );
        return 1;
    }


    int SpawnEntity( lua_State * L )
    {
        int number_of_arguments = lua_gettop( L );
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        auto position = luaU_check<Math::Float3>( L, 3 );
        auto euler_rotation = luaU_check<Math::Float3>( L, 4 );
        auto velocity = Math::Float3( 0 );
        if( number_of_arguments >= 5 )
        {
            velocity = luaU_check<Math::Float3>( L, 5 );
        }

        EntityID id;
        if( luaU_is<EntityTemplateID>( L, 2 ) )
        {
            auto template_id = luaU_to<EntityTemplateID>( L, 2 );
            TryAndCatch(L, [&]()
            {
                id = dog_world->SpawnEntity( template_id, { position, Math::EulerToQuaternion( euler_rotation ) }, velocity );
            });
        }
        else
        {
            auto entity_description = ReadEntityDescription( L, 2 );
            CheckForPhysicsComponent( L, entity_description );
            TryAndCatch(L, [&]()
            {
                id = dog_world->SpawnEntity( entity_description, { position, Math::EulerToQuaternion( euler_rotation ) }, velocity );
            });
        }
        luaU_push( L, id );
        return 1;
    };


    int SpawnItem( lua_State * L )
    {
        int number_of_arguments = lua_gettop( L );
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        luaL_checktype( L, 2, LUA_TTABLE );
        EntityTemplateID equipped_template_id;
        if( luaU_fieldis<EntityTemplateID>(L, 2, "equipped") )
        {
            equipped_template_id = luaU_getfield<EntityTemplateID>( L, 2, "equipped" );
        }
        else
        {
            lua_getfield(L, 2, "equipped");
            auto equipped_description = ReadEntityDescription( L, -1 );
            lua_pop(L, 1); // pop the equipped field
            equipped_template_id = dog_world->m_entity_templates.Add( std::move( equipped_description ) );
        }

        EntityTemplateID dropped_template_id;
        if( luaU_fieldis<EntityTemplateID>(L, 2, "dropped") )
        {
            dropped_template_id = luaU_getfield<EntityTemplateID>( L, 2, "dropped" );
        }
        else
        {
            lua_getfield(L, 2, "dropped");
            auto dropped_description = ReadEntityDescription( L, -1 );
            lua_pop(L, 1); // pop the dropped field
            dropped_template_id = dog_world->m_entity_templates.Add( std::move( dropped_description ) );
        }

        auto position = luaU_check<Math::Float3>( L, 3 );
        auto euler_rotation = luaU_check<Math::Float3>( L, 4 );
        auto velocity = Math::Float3( 0 );
        if( number_of_arguments >= 5 )
        {
            velocity = luaU_check<Math::Float3>( L, 5 );
        }

        Logic::ItemComponentDescription description;
        description.dropped_template = dropped_template_id;
        description.equipped_template = equipped_template_id;

        description.name = luaU_optfield<std::string>(L, 2, "name", "Unnamed item");

        auto entity_id = dog_world->SpawnItem( description, { position, Math::EulerToQuaternion( euler_rotation ) }, velocity );
        luaU_push( L, entity_id );
        return 1;
    }


    int AddComponent(lua_State * L)
    {
        auto dog_world = luaW_check<DogWorld>(L, 1);
        auto entity_id = luaU_check<EntityID>(L, 2);

        auto entity_description = ReadEntityDescription(L, 3);
        dog_world->AddEntityComponents( entity_id, entity_description, Math::Identity(), 0 );

        return 1;
    };


    int EntityWieldItem( lua_State * L )
    {
        auto dog_world = luaW_check<DogWorld>(L, 1);

        auto wielder_entity_id = luaU_check<EntityID>(L, 2);
        auto wielded_entity_id = luaU_check<EntityID>(L, 3);

        auto attachment_point = luaU_check<AttachmentPoint>(L, 4);

        TryAndCatch(L, [=]()
        {
            dog_world->EntityWieldItem(wielder_entity_id, wielded_entity_id, attachment_point);
        });

        return 1;
    };


    int SetAIAggressionTarget(lua_State * L)
    {
        auto dog_world = luaW_check<DogWorld>(L, 1);

        auto ai_entity_id = luaU_check<EntityID>(L, 2);
        auto target_entity_id = luaU_check<EntityID>(L, 3);
        auto maximum_distance = luaU_check<float>(L, 4);

        dog_world->SetAIAgressionTarget(ai_entity_id, target_entity_id, maximum_distance);

        return 1;
    }


    int SetAIFollowingTarget(lua_State * L)
    {
        auto dog_world = luaW_check<DogWorld>(L, 1);

        auto ai_entity_id = luaU_check<EntityID>(L, 2);
        auto target_entity_id = luaU_check<EntityID>(L, 3);
        auto following_distance = luaU_check<MinMax<float>>(L, 4);

        dog_world->SetAIFollowingTarget(ai_entity_id, target_entity_id, following_distance.min, following_distance.max);

        return 1;
    }

    int SetAIToPassive(lua_State * L)
    {
        auto dog_world = luaW_check<DogWorld>(L, 1);

        auto ai_entity_id = luaU_check<EntityID>(L, 2);

        dog_world->SetAIToPassive(ai_entity_id);

        return 1;
    }


	int SetAIPatrollingTarget(lua_State * L)
	{
		auto dog_world = luaW_check<DogWorld>(L, 1);

		auto ai_entity_id = luaU_check<EntityID>(L, 2);
		auto target_position = luaU_check<Math::Float3>(L, 3);

		dog_world->SetAIPatrollingTarget(ai_entity_id, target_position);

		return 1;
	}


    int Create3DTerrain(lua_State * L)
    {
        auto dog_world = luaW_check<DogWorld>(L, 1);

        auto entity_description = ReadEntityDescription(L, 2);
        CheckForPhysicsComponent(L, entity_description);

        auto terrain_center = luaU_check<Math::Float3>(L, 3);
        auto update_distance = luaU_check<float>(L, 4);

        auto terrain_block_count = luaU_check<Math::Unsigned3>(L, 5);

        auto terrain_block_dimensions = luaU_check<Math::Float3>(L, 6);
        auto terrain_block_cube_count = luaU_check<Math::Unsigned3>(L, 7);

        // Get LOD distance thresholds
        auto lod_count = lua_rawlen(L, 8);
        auto degradation_thresholds = std::vector<float>(lod_count);

        for (auto i = 0u; i < lod_count; ++i)
        {
            lua_rawgeti(L, 8, i + 1u);
            degradation_thresholds[i] = luaU_check<float>(L, -1);
        }

        std::vector<NoiseParameters<Math::Float3>> noise3d;
        ReadNoiseParameters(L, 9, noise3d);

        EntityID id;
        TryAndCatch(L, [&](){
          id = dog_world->CreateTerrain(entity_description, terrain_center, update_distance, terrain_block_count, terrain_block_dimensions, terrain_block_cube_count, degradation_thresholds, move(noise3d) );
        });

        luaU_push(L, id);
        return 1;
    };


	int Create2DTerrain(lua_State * L)
	{
		auto dog_world = luaW_check<DogWorld>(L, 1);

        auto entity_description = ReadEntityDescription(L, 2);
        CheckForPhysicsComponent(L, entity_description);

        auto terrain_center = luaU_check<Math::Float3>(L, 3);

        Math::Unsigned2 patch_dimensions = luaU_check<Math::Unsigned2>(L, 4);

        Math::Float2 patch_size = luaU_check<Math::Float2>(L, 5);

        // Get LOD distance thresholds
        auto lod_count = luaU_check<uint32_t>(L, 6);

        std::vector<NoiseParameters<Math::Float2>> noise2d;
        ReadNoiseParameters(L, 7, noise2d);

		EntityID id;
        TryAndCatch(L, [&](){
            id = dog_world->CreateTerrain(entity_description, terrain_center, patch_dimensions, patch_size, lod_count, move(noise2d) );
        });

		luaU_push(L, id);
		return 1;
	};


    int SetTerrainGrassTypes(lua_State * L)
    {
        auto dog_world = luaW_check<DogWorld>(L, 1);

        // Read grass type render component descriptions
        auto render_descriptions = ReadRenderComponents(L, 2);

        // Read grass counts
        auto length = lua_rawlen(L, 3);
        auto counts = std::vector<std::array<float, Graphics::c_lod_count>>(length);

        for (auto i = 0u; i < length; ++i)
        {
            lua_rawgeti(L, 3, i + 1u);
            counts[i] = luaU_check<std::array<float, Graphics::c_lod_count>>(L, -1);
        }

        dog_world->SetTerrainGrassTypes(render_descriptions, counts);
        return 0;
    }


    int SetAmmunition( lua_State *L )
    {
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        auto const entity_id = luaU_check<EntityID>( L, 2 );
        auto const properties = luaU_check<Logic::ThrowProperties>( L, 3 );
        auto const ammo_template_id = luaU_check<EntityTemplateID>( L, 4 );

        dog_world->SetEntityAmmunition( entity_id, properties, ammo_template_id );
        return 0;
    }


    // Setting the maximum and current hit points to the specified amount
    int SetHitpoints(lua_State *L)
    {
        auto dog_world = luaW_check<DogWorld>(L, 1);
        auto const entity_id = luaU_check<EntityID>(L, 2);
        auto const hitpoints = luaU_check<float>(L, 3);

        dog_world->SetEntityHitpoints(entity_id, hitpoints);
        return 0;
    }


    int HasPositiveHitpoints(lua_State *L )
    {
        auto dog_world = luaW_check<DogWorld>(L, 1);
        auto const entity_id = luaU_check<EntityID>(L, 2);
        auto result = dog_world->HasPositiveHitpoints(entity_id);
        lua_pushboolean(L, result);
        return 1;
    }


    int SpawnLight( lua_State *L )
    {
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        auto entity_description = ReadEntityDescription( L, 2 );
        // for now
        // TODO: remove this requirement
        CheckForPhysicsComponent( L, entity_description );
        auto euler_rotation = luaU_check<Math::Float3>( L, 4 );

        auto id = dog_world->SpawnEntity( entity_description, { 0, Math::EulerToQuaternion( euler_rotation ) }, 0 );

        auto description = luaU_check<Graphics::LightDescription>(L, 3);
        dog_world->AddLight( description, id );

        luaU_push( L, id );

        return 1;
    }


    int AddLight( lua_State * L )
    {
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        auto entity_id = luaU_check<EntityID>( L, 2 );

        Graphics::LightDescription d;
        dog_world->AddLight( d, entity_id );

        return 0;
    }


    int CreatePlayerController( lua_State * L )
    {
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        auto entity_id = luaU_check<EntityID>( L, 2 );

        TryAndCatch(L, [&]()
        {
            dog_world->CreatePlayerController( entity_id );
        });
        return 0;
    }


    int CreatePlayerThrowController( lua_State * L )
    {
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        auto entity_id = luaU_check<EntityID>( L, 2 );

        dog_world->CreatePlayerThrowController( entity_id );
        return 0;
    }


    int CreatePlayerStrikeController(lua_State * L)
    {
        auto dog_world = luaW_check<DogWorld>(L, 1);
        auto entity_id = luaU_check<EntityID>(L, 2);

        dog_world->CreatePlayerStrikeController(entity_id);
        return 0;
    }


    int SetCameraPositionAndAngles( lua_State * L )
    {
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        auto position = luaU_check<Math::Float3>( L, 2 );
        auto angles = luaU_check<Math::Float3>( L, 3 );

        dog_world->SetCameraPositionAndAngles( position, angles );
        return 0;
    }


    int FollowCameraRotation( lua_State * L )
    {
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        auto entity_id = luaU_check<EntityID>( L, 2 );

        dog_world->FollowCameraRotation( entity_id );
        return 0;
    }


	int SetCameraTarget(lua_State * L)
	{
		auto dog_world = luaW_check<DogWorld>(L, 1);
		auto entity_id = luaU_check<EntityID>(L, 2);

		dog_world->SetCameraTarget(entity_id);
		return 1;
	};


    int SetCameraKeys( lua_State * L )
    {
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        auto camera_keys = luaU_check<Input::DirectionalKeys>( L, 2 );
        dog_world->InputConfiguration().keys.camera_move = camera_keys;
        return 0;
    }


    int SetPlayerKeys( lua_State * L )
    {
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        auto player_keys = luaU_check<Input::DirectionalKeys>( L, 2 );
        dog_world->InputConfiguration().keys.player_move = player_keys;
        return 0;
    }


    int SetGraphicsConfiguration( lua_State * L )
    {
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        ReadGraphicsConfiguration(L, 2, dog_world->GraphicsConfiguration());
        return 0;
    }


    int SetPhysicsConfiguration( lua_State * L )
    {
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        Physics::WorldConfiguration config;
        ReadPhysicsConfiguration( L, 2, config );
        dog_world->SetPhysicsConfiguration(config);
        return 0;
    }


    int SetTimeStep( lua_State * L )
    {
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        dog_world->m_world_configuration.time_step = luaU_check<float>( L, 2 );
        return 0;
    }


    int SetGravity( lua_State * L )
    {
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        auto gravity = luaU_check<Math::Float3>( L, 2 );
        dog_world->SetGravity( gravity );
        return 0;
    }


    int GetEntityOrientation(lua_State * L)
    {
        auto dog_world = luaW_check<DogWorld>(L, 1);
        auto entity_id = luaU_check<EntityID>(L, 2);

        auto orientation = dog_world->GetEntityOrientation(entity_id);

        // TODO: Create quaternion or orientation wrapper
        luaU_push(L, orientation.position);
        //luaU_push(L, orientation.rotation);

        return 1;
    }


    int GetEntityPositions(lua_State * L)
    {
        auto dog_world = luaW_check<DogWorld>(L, 1);
        if(!lua_istable(L, 2))
        {
            luaL_error(L, "Second argument to GetEntityOrientations is not a table.");
        }
        auto length = lua_rawlen(L, 2);
        std::vector<EntityID> entities(length);
        for(auto i = 0u; i < length; i++)
        {
            // get the entity table
            lua_rawgeti(L, 2, i + 1);
            // read it
            entities[i] = luaU_check<EntityID>(L, -1);
            // pop the entity table
            lua_pop(L, 1);
        }

        std::vector<Orientation> orientations(length);
        TryAndCatch(L, [&](){dog_world->GetOrientations(entities, orientations);});

        lua_createtable(L, int(length), 0);
        for (auto i = 0u; i < length; ++i)
        {
            luaU_push<Math::Float3>(L, orientations[i].position);
            lua_rawseti(L, -2, i + 1);
        }

        return 1;
    }


    int KillEntity(lua_State * L)
    {
        auto dog_world = luaW_check<DogWorld>(L, 1);
        auto entity_id = luaU_check<EntityID>(L, 2);

        dog_world->KillEntity(entity_id);

        return 0;
    }


    int SetAfterDeathComponents( lua_State * L )
    {
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        auto entity_id = luaU_check<EntityID>( L, 2 );

        EntityTemplateID template_id;
        if( luaU_is<EntityTemplateID>( L, 3 ) )
        {
            template_id = luaU_to<EntityTemplateID>( L, 3 );
        }
        else
        {
            auto entity_description = ReadEntityDescription( L, 3 );
            template_id = dog_world->m_entity_templates.Add( std::move( entity_description ) );
        }

        dog_world->SetAfterDeathComponents( entity_id, template_id );

        return 0;
    }


    int FindRestingPositionOnEntity( lua_State * L )
    {
        auto dog_world = luaW_check<DogWorld>( L, 1 );
        auto starting_position = luaU_check<Math::Float3>( L, 2 );
        auto target_entity = luaU_check<EntityID>( L, 3 );

        auto position = dog_world->FindRestingPositionOnEntity( starting_position, target_entity );

        luaU_push( L, position );
        return 1;
    }
}

static luaL_Reg table[] =
{
    //{ "checkTotalMoneyInBank", BankAccount_checkTotalMoneyInBank },
    { nullptr, nullptr }
};

static luaL_Reg metatable[] =
{
    { "AddComponent", AddComponent },
    { "AddLight", AddLight },
    { "Create2DTerrain", Create2DTerrain },
    { "Create3DTerrain", Create3DTerrain },
    { "CreateAndShowWindow", CreateAndShowWindow },
    { "CreatePlayerController", CreatePlayerController },
    { "CreatePlayerStrikeController", CreatePlayerStrikeController },
    { "CreatePlayerThrowController", CreatePlayerThrowController },
    { "EntityWieldItem", EntityWieldItem },
    { "FindRestingPositionOnEntity", FindRestingPositionOnEntity },
    { "FollowCameraRotation", FollowCameraRotation },
    { "GetEntityOrientation", GetEntityOrientation },
    { "GetEntityPositions", GetEntityPositions },
    { "HasPositiveHitpoints", HasPositiveHitpoints},
    { "KillEntity", KillEntity },
    { "RegisterEntityTemplate", RegisterEntityTemplate },
    { "Run", Run },
    { "SetAfterDeathComponents", SetAfterDeathComponents },
    { "SetAIAggressionTarget", SetAIAggressionTarget },
    { "SetAIFollowingTarget", SetAIFollowingTarget },
    { "SetAIPatrollingTarget", SetAIPatrollingTarget },
    { "SetAIToPassive", SetAIToPassive },
    { "SetAmmunition", SetAmmunition },
    { "SetCameraKeys", SetCameraKeys },
    { "SetCameraPositionAndAngles", SetCameraPositionAndAngles },
    { "SetCameraTarget", SetCameraTarget },
    { "SetGraphicsConfiguration", SetGraphicsConfiguration },
    { "SetGravity", SetGravity },
    { "SetHitpoints", SetHitpoints },
    { "SetPhysicsConfiguration", SetPhysicsConfiguration },
    { "SetPlayerKeys", SetPlayerKeys },
    { "SetTerrainGrassTypes", SetTerrainGrassTypes },
    { "SetTimeStep", SetTimeStep },
    { "SetUpdateFunction", SetUpdateFunction },
    { "SpawnEntity", SpawnEntity },
    { "SpawnItem", SpawnItem },
    { "SpawnLight", SpawnLight },
    { nullptr, nullptr }
};

int luaopen_DogWorld( lua_State* L )
{
    luaW_register<DogWorld>( L,
                             "DogWorld",
                             table,
                             metatable
                             // LuaWrapper will generate a default allocator for you.
                             );
    return 1;
}
