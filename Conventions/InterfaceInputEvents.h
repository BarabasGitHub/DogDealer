#pragma once

enum struct InterfaceInputEvent
{
    SwitchPauseSimulation,
    SingleSimulationStep,
    SwitchControlMode, // used to switch between camera and pointer control
    PointerInteract,
};
