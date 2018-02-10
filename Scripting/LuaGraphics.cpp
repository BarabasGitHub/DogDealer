#include "LuaGraphics.h"

#include "LuaMathTypes.h"

using namespace Graphics;

LightDescription luaU_Impl<LightDescription>::luaU_check( lua_State* L, int index )
{
    return LightDescription{luaU_checkfield<Math::Float3>( L, index, "color" )};
}


LightDescription luaU_Impl<LightDescription>::luaU_to( lua_State* L, int index )
{
    return Graphics::LightDescription{ luaU_getfield<Math::Float3>(L, index, "color" )};
}


void luaU_Impl<LightDescription>::luaU_push( lua_State* L, const LightDescription& val )
{
    lua_newtable( L );
    luaU_setfield<Math::Float3>( L, -1, "color", val.color );
}


bool luaU_Impl<LightDescription>::luaU_is(lua_State* L, int index)
{
    return
        lua_istable(L, index) &&
        luaU_fieldis<Math::Float3>(L, index, "color");
}