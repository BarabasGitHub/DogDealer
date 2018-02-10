#pragma once

#include <Math\FloatTypes.h>

#include <Conventions\InterfaceInputEvents.h>

#include <vector>

struct CameraInput
{
    Math::Float3 movement;
    Math::Float3 angles;
    float zoom;
};

struct InterfaceInput
{
    std::vector<InterfaceInputEvent> events;
    Math::Float2 pointer_position;
    CameraInput camera;
};
