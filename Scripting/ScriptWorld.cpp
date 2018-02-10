#pragma once
#include "ScriptWorld.h"

#include <Windows/WindowsInclude.h>	// used for message box

extern "C" {
#include "lua/include/lua.h"
#include "lua/include/lualib.h"
#include "lua/include/lauxlib.h"
}

#include "LuaDogWorld.h"
#include <Utilities\StringUtilities.h>


namespace
{
    void ShowError( std::string const & script_name, std::string const & error_message )
    {
        auto message_string = "Error running lua script: " + script_name + "\n" +
            "Lua error message:\n" + error_message + ".";
        auto message = message_string.c_str();

        MessageBoxA( nullptr, message, "Error", MB_OK );
    }


	void SetLuaPath(lua_State* L)
	{
		lua_getglobal(L, "package");
		lua_getfield(L, -1, "path"); // get field "path" from table at top of stack (-1)
		std::string path = lua_tostring(L, -1); // grab path string from top of stack
		lua_pop(L, 1); // get rid of the string on the stack we just pushed

        // Redirect from debug folder to main lua script one
        path += ";";
        path += GetCurrentWorkingDirectoryString();
        path += "\\lua\\scripts\\?.lua";

		lua_pushstring(L, path.c_str()); // push the new one
		lua_setfield(L, -2, "path"); // set the field "path" in table at -2 with value at top of stack
		lua_pop(L, 1); // get rid of package table from top of stack
	}


    class  LuaState
    {
        lua_State* L;
    public:
        LuaState() :
            L( luaL_newstate( ) )
        {}

        ~LuaState( )
        {
            lua_close( L );
        }

        operator lua_State*( )
        {
            return L;
        }
    };

}



namespace Scripting
{
    void LoadScriptFile( std::string filename )
    {
        using namespace std;

        LuaState L;

        luaL_openlibs( L );

        luaopen_DogWorld( L );

        SetLuaPath( L );

        if( luaL_loadfile( L, filename.c_str() ) || lua_pcall( L, 0, 0, 0 ) )
        {
            ShowError( filename, lua_tostring( L, -1 ) );
            return;
        }

#if defined(_DEBUG)
        lua_pushboolean( L, true );
        lua_setglobal( L, "DEBUG" );
#endif

        lua_getglobal( L, "main" );
        auto result = lua_pcall( L, 0, 0, 0 );
        if( result != 0 )
        {
            ShowError( filename, lua_tostring( L, -1 ) );
            return;
        }
    }
}