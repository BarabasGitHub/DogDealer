#pragma once
#include <lua\include\luawrapperutil.hpp>
#include <Math\FloatTypes.h>
#include <Math\IntegerTypes.h>


// needed for the luaU_to luaU_push functions
template<>
struct luaU_Impl<Math::Float3>
{
    inline static Math::Float3 luaU_check( lua_State* L, int index )
    {
        return Math::Float3(
            luaU_checkfield<float>( L, index, "x" ),
            luaU_checkfield<float>( L, index, "y" ),
            luaU_checkfield<float>( L, index, "z" ) );
    }

    inline static Math::Float3 luaU_to( lua_State* L, int index )
    {
        return Math::Float3(
            luaU_getfield<float>( L, index, "x" ),
            luaU_getfield<float>( L, index, "y" ),
            luaU_getfield<float>( L, index, "z" ) );
    }

    inline static void luaU_push( lua_State* L, const Math::Float3& val )
    {
        lua_newtable( L );
        luaU_setfield<float>( L, -1, "x", val.x );
        luaU_setfield<float>( L, -1, "y", val.y );
        luaU_setfield<float>( L, -1, "z", val.z );
    }

    inline static bool luaU_is(lua_State* L, int index)
    {
        return
            lua_istable(L, index) &&
            luaU_fieldis<float>(L, index, "x") &&
            luaU_fieldis<float>(L, index, "y") &&
            luaU_fieldis<float>(L, index, "z");
    }
};

// needed for the luaU_to luaU_push functions
template<>
struct luaU_Impl<Math::Unsigned3>
{
    inline static Math::Unsigned3 luaU_check( lua_State* L, int index )
    {
        return Math::Unsigned3(
            luaU_checkfield<unsigned>( L, index, "x" ),
            luaU_checkfield<unsigned>( L, index, "y" ),
            luaU_checkfield<unsigned>( L, index, "z" ) );
    }

    inline static Math::Unsigned3 luaU_to( lua_State* L, int index )
    {
        return Math::Unsigned3(
            luaU_getfield<unsigned>( L, index, "x" ),
            luaU_getfield<unsigned>( L, index, "y" ),
            luaU_getfield<unsigned>( L, index, "z" ) );
    }

    inline static void luaU_push( lua_State* L, const Math::Unsigned3& val )
    {
        lua_newtable( L );
        luaU_setfield<unsigned>( L, -1, "x", val.x );
        luaU_setfield<unsigned>( L, -1, "y", val.y );
        luaU_setfield<unsigned>( L, -1, "z", val.z );
    }
};

// needed for the luaU_to luaU_push functions
template<>
struct luaU_Impl<Math::Float2>
{
    inline static Math::Float2 luaU_check( lua_State* L, int index )
    {
        return Math::Float2(
            luaU_checkfield<float>( L, index, "x" ),
            luaU_checkfield<float>( L, index, "y" ));
    }

    inline static Math::Float2 luaU_to( lua_State* L, int index )
    {
        return Math::Float2(
            luaU_getfield<float>( L, index, "x" ),
            luaU_getfield<float>( L, index, "y" ) );
    }

    inline static void luaU_push( lua_State* L, const Math::Float2 val )
    {
        lua_newtable( L );
        luaU_setfield<float>( L, -1, "x", val.x );
        luaU_setfield<float>( L, -1, "y", val.y );
    }


    inline static bool luaU_is( lua_State* L, int index)
    {
        return lua_istable(L, index) && luaU_fieldis<float>(L, index, "x") && luaU_fieldis<float>(L, index, "y");
    }
};

// needed for the luaU_to luaU_push functions
template<>
struct luaU_Impl<Math::Unsigned2>
{
    inline static Math::Unsigned2 luaU_check( lua_State* L, int index )
    {
        return Math::Unsigned2(
            luaU_checkfield<unsigned>( L, index, "x" ),
            luaU_checkfield<unsigned>( L, index, "y" )
            );
    }

    inline static Math::Unsigned2 luaU_to( lua_State* L, int index )
    {
        return Math::Unsigned2(
            luaU_getfield<unsigned>( L, index, "x" ),
            luaU_getfield<unsigned>( L, index, "y" )
            );
    }

    inline static void luaU_push( lua_State* L, const Math::Unsigned2& val )
    {
        lua_newtable( L );
        luaU_setfield<unsigned>( L, -1, "x", val.x );
        luaU_setfield<unsigned>( L, -1, "y", val.y );
    }
};