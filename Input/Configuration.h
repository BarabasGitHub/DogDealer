#pragma once
#include "KeyCodes.h"

namespace Input
{

    struct DirectionalKeys
    {
        KeyCode
        up,
        down,
        left,
        right;
    };

    struct KeyConfiguration
    {
        DirectionalKeys camera_move;
        DirectionalKeys player_move;

        KeyCode lookaround;
        KeyCode jump;

        KeyCode throw_action;
        KeyCode attack_action;
        KeyCode block_action;

        KeyCode drop_weapon;
        KeyCode pick_up_weapon;

        KeyCode switch_control_mode; // switch between controlling the camera or the pointer with the mouse
        KeyCode pause_simulation; // or single step with shift
    };

    struct WorldConfiguration
    {
        KeyConfiguration keys;
    };

}
