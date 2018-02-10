#pragma once
#include <Windows\WindowsInclude.h>

#include <utility>

namespace Graphics
{

    //forward declare
    struct Device;
    class SwapChain;

    std::pair<Device, SwapChain> CreateDefaultDeviceAndSwapchain( HWND window, unsigned sample_count = 1 );
}
