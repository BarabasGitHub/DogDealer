#pragma once
#include <Input\Configuration.h>

#include <lua\include\luawrapperutil.hpp>


template<>
struct luaU_Impl<Input::DirectionalKeys>
{
    static Input::DirectionalKeys luaU_check( lua_State* L, int index );
    static Input::DirectionalKeys luaU_to( lua_State* L, int index );
    static void luaU_push( lua_State* L, Input::DirectionalKeys const & val );
};
