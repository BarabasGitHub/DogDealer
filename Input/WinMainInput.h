#pragma once

#include <Windows\WindowsInclude.h>

struct WinMainInput {
    HINSTANCE instance;
    HINSTANCE previous_instance;
    wchar_t const * command_line;
    int show_command;
};

extern WinMainInput s_win_main_input;