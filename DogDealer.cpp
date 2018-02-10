#include <Windows\WindowsInclude.h>

#include <Scripting\ScriptWorld.h>

#include <Input\WinMainInput.h>


int APIENTRY wWinMain( _In_ HINSTANCE instance, _In_opt_ HINSTANCE previous_instance, _In_ LPWSTR command_line, _In_ int command_show )
{

    s_win_main_input = { instance, previous_instance, command_line, command_show };

    std::string script_directory = "lua\\scripts\\";

    Scripting::LoadScriptFile( script_directory + "main_script.lua");

    return 0;
}