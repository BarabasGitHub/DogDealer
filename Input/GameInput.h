#pragma once

#include <Math\FloatTypes.h>

#include <Conventions\GameInputEvents.h>

#include <vector>

struct GameInput
{
    Math::Float2 movement;
    Math::Float2 attack_direction;
    std::vector<GameInputEvent> events;
};
