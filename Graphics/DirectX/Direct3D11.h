#pragma once
// because d3d11 includes the windows header we include it first ourselves with the right preprocessor defines
#include <Windows\WindowsInclude.h>
#include <d3d11.h>
#include <string>
#include <codecvt>

#include <Utilities\ComPtr.h>

namespace Graphics
{
    template<typename T> unsigned D3Dsizeof( T = T() )
    {
        return static_cast<unsigned>( sizeof( T ) );
    }

    inline void SetObjectNameDebugOnly( ComPtr<ID3D11DeviceChild> resource, const std::string & name )
    {
        (void)name;
#if defined(_DEBUG) || defined(PROFILE)
        resource->SetPrivateData( WKPDID_D3DDebugObjectName, static_cast<unsigned>( name.length() ), name.c_str() );
#endif
    }

    inline void SetObjectNameDebugOnly( ComPtr<ID3D11DeviceChild> resource, const std::wstring & name )
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conversion;
        auto temp = conversion.to_bytes( std::move( name ) );
        SetObjectNameDebugOnly( std::move( resource ), std::move( temp ) );
    }

    // this shouldn't be here, but I have currently no better place to store it
    namespace
    {
        inline DXGI_FORMAT ToSRGB( DXGI_FORMAT format )
        {
            switch( format )
            {
            case DXGI_FORMAT_R8G8B8A8_UNORM:
                return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            case DXGI_FORMAT_BC1_UNORM:
                return DXGI_FORMAT_BC1_UNORM_SRGB;
            case DXGI_FORMAT_BC2_UNORM:
                return DXGI_FORMAT_BC2_UNORM_SRGB;
            case DXGI_FORMAT_BC3_UNORM:
                return DXGI_FORMAT_BC3_UNORM_SRGB;
            case DXGI_FORMAT_B8G8R8A8_UNORM:
                return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
            case DXGI_FORMAT_B8G8R8X8_UNORM:
                return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
            case DXGI_FORMAT_BC7_UNORM:
                return DXGI_FORMAT_BC7_UNORM_SRGB;
            default:
                return format;
            }
        }
        inline DXGI_FORMAT ToNONSRGB( DXGI_FORMAT format )
        {
            switch( format )
            {
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
                return DXGI_FORMAT_R8G8B8A8_UNORM;
            case DXGI_FORMAT_BC1_UNORM_SRGB:
                return DXGI_FORMAT_BC1_UNORM;
            case DXGI_FORMAT_BC2_UNORM_SRGB:
                return DXGI_FORMAT_BC2_UNORM;
            case DXGI_FORMAT_BC3_UNORM_SRGB:
                return DXGI_FORMAT_BC3_UNORM;
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
                return DXGI_FORMAT_B8G8R8A8_UNORM;
            case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
                return DXGI_FORMAT_B8G8R8X8_UNORM;
            case DXGI_FORMAT_BC7_UNORM_SRGB:
                return DXGI_FORMAT_BC7_UNORM;
            default:
                return format;
            }
        }

    }
}