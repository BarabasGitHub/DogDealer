#pragma once

#include <Math\FloatTypes.h>

struct MouseState
{
    Math::Float2 screen_position = 0;
    Math::Float2 screen_movement = 0;
    Math::Float2 raw_position = 0;
    Math::Float2 raw_movement = 0;

    int wheel_rotation = 0;
};

