#pragma once
#include <GameLogic\Configuration.h>
#include <GameLogic\EntityAbilities.h>
#include <GameLogic\ResourceDescriptions.h>

#include <lua\include\luawrapperutil.hpp>

// needed for the luaU_to luaU_push functions


template<>
struct luaU_Impl<Logic::ThrowProperties>
{
    static Logic::ThrowProperties luaU_check( lua_State * L, int index );
    static Logic::ThrowProperties luaU_to( lua_State * L, int index );

    //static void luaU_push( lua_State* L, Logic::ThrowProperties const & val );
};

template<>
struct luaU_Impl < AttachmentPoint >
{
    // identifiers
    static char const c_bone_index[];
    static char const c_position[];
    static char const c_rotation[];

    static AttachmentPoint luaU_check(lua_State* L, int index);
    static AttachmentPoint luaU_to(lua_State* L, int index);
    //static void luaU_push(lua_State* L, AttachmentPoint const & val); // tricky due to quaternion
    static bool luaU_is(lua_State* L, int index);
};

template<>
struct luaU_Impl<Logic::MobileComponentDescription>
{
    // identifiers
    static char const c_motion_power[];
    static char const c_maximum_torque[];
    static char const c_motion_bias_right[];
    static char const c_motion_bias_forwards[];
    static char const c_motion_bias_left[];
    static char const c_motion_bias_backwards[];
    static char const c_target_speed[];
    static char const c_attachment_point[];

    static Logic::MobileComponentDescription luaU_check( lua_State* L, int index );
    static Logic::MobileComponentDescription luaU_to( lua_State* L, int index );
    static void luaU_push( lua_State* L, Logic::MobileComponentDescription const & val );
    static bool luaU_is( lua_State* L, int index );
};


template<>
struct luaU_Impl<Logic::ItemComponentDescription>
{
    // identifiers
    static char const c_dropped_template_id[];
    static char const c_equipped_template_id[];

    static Logic::ItemComponentDescription luaU_check( lua_State* L, int index );
    static Logic::ItemComponentDescription luaU_to( lua_State* L, int index );
    static void luaU_push( lua_State* L, Logic::ItemComponentDescription const & val );
    static bool luaU_is( lua_State* L, int index );
};
