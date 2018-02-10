#pragma once
#include <lua\include\luawrapperutil.hpp>
#include "Graphics\ResourceDescriptions.h"

template<>
struct luaU_Impl<Graphics::LightDescription>
{
    static Graphics::LightDescription luaU_check( lua_State* L, int index );

    static Graphics::LightDescription luaU_to( lua_State* L, int index );

    static void luaU_push( lua_State* L, const Graphics::LightDescription& val );

    static bool luaU_is(lua_State* L, int index);
};
