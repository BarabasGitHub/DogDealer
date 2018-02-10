#include "CreateDeviceAndSwapChain.h"
#include "DirectX\Direct3D11.h"
#include <Windows\WindowsErrorsToException.h>
#include "Device.h"
#include "SwapChain.h"

#include <array>

#ifdef ENABLE_FLOATING_POINT_EXCEPTIONS
#include <float.h>
#endif

using namespace std;
namespace Graphics
{

    pair<Device, SwapChain> CreateDefaultDeviceAndSwapchain( HWND window, unsigned sample_count )
    {
        unsigned create_device_flags = 0u; //D3D11_CREATE_DEVICE_BGRA_SUPPORT; //for direct2d support. Which doesn't work on anything but Windows 8.
    #ifdef _DEBUG
        create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

        array<D3D_DRIVER_TYPE, 1> driver_types =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            //D3D_DRIVER_TYPE_WARP,
            //D3D_DRIVER_TYPE_REFERENCE,
        };

        array<D3D_FEATURE_LEVEL, 1> feature_levels =
        {
            D3D_FEATURE_LEVEL_11_0,
        };

        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 1;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        // sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = window;
        sd.SampleDesc.Count = sample_count;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; //Set this flag to enable an application to switch modes by calling IDXGISwapChain::ResizeTarget. When switching from windowed to full-screen mode, the display mode (or monitor resolution) will be changed to match the dimensions of the application window.

        auto result = S_OK;
        auto final_driver_type = D3D_DRIVER_TYPE_NULL;
        ComPtr<IDXGISwapChain> dxgi_swap_chain;
        ComPtr<ID3D11Device> d3d_device;
    #ifdef ENABLE_FLOATING_POINT_EXCEPTIONS
        _controlfp_s( nullptr, _MCW_EM, _MCW_EM );
    #endif
        for( auto driver_type : driver_types )
        {
            result = D3D11CreateDeviceAndSwapChain( nullptr, driver_type, nullptr, create_device_flags, feature_levels.data(), static_cast<unsigned>( feature_levels.size() ),
                                                    D3D11_SDK_VERSION, &sd, dxgi_swap_chain.GetAddressOf(), d3d_device.GetAddressOf(), nullptr, nullptr );
            if( SUCCEEDED( result ) )
            {
                final_driver_type = driver_type;
                break;
            }
        }
    #ifdef ENABLE_FLOATING_POINT_EXCEPTIONS
        _controlfp_s( nullptr, _EM_INEXACT, _MCW_EM );
    #endif
        ThrowIfFailed( result );


        return make_pair( Device( d3d_device, final_driver_type ), dxgi_swap_chain );
    }
}