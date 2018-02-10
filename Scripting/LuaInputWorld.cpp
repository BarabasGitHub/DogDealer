#include "LuaInputWorld.h"

using namespace Input;

DirectionalKeys luaU_Impl<DirectionalKeys>::luaU_check( lua_State* L, int index )
{
    return{
        luaU_checkfield<KeyCode>( L, index, "up" ),
        luaU_checkfield<KeyCode>( L, index, "down" ),
        luaU_checkfield<KeyCode>( L, index, "left" ),
        luaU_checkfield<KeyCode>( L, index, "right" )
    };
}


DirectionalKeys luaU_Impl<DirectionalKeys>::luaU_to( lua_State* L, int index )
{
    return{
        luaU_getfield<KeyCode>( L, index, "up" ),
        luaU_getfield<KeyCode>( L, index, "down" ),
        luaU_getfield<KeyCode>( L, index, "left" ),
        luaU_getfield<KeyCode>( L, index, "right" )
    };
}


void luaU_Impl<DirectionalKeys>::luaU_push( lua_State* L, DirectionalKeys const & val )
{
    lua_newtable( L );
    luaU_setfield( L, -1, "up", val.up );
    luaU_setfield( L, -1, "down", val.down );
    luaU_setfield( L, -1, "left", val.left );
    luaU_setfield( L, -1, "right", val.right );
}
