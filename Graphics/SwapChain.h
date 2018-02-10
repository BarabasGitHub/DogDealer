#pragma once
#include "DirectX/Direct3D11.h"
#include <Utilities\ComPtr.h>

namespace Graphics
{
    //forward declaration


    class SwapChain
    {
    protected:
        ComPtr<IDXGISwapChain> m_swap_chain;

    public:
        SwapChain( void );
        SwapChain( ComPtr<IDXGISwapChain> dxgi_swap_chain );
        ~SwapChain( void );

        ComPtr<ID3D11Texture2D> GetBuffer( unsigned index );
        DXGI_SWAP_CHAIN_DESC GetDescription();
        DXGI_FRAME_STATISTICS  GetFrameStatistics();
        bool IsFullScreen();
        unsigned GetLastPresentCount();
        float GetAspectRatio();
        void Present();
        void PresentWithVerticalSync( unsigned interval = 1 );
        void PresentCustom( unsigned interval, unsigned flags );
        void ResizeBuffers( unsigned buffer_count = 0, unsigned width = 0, unsigned height = 0, DXGI_FORMAT new_format = DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG flags = static_cast<DXGI_SWAP_CHAIN_FLAG>( 0 ) );
    };

}