#include "SwapChain.h"
#include <Windows\WindowsErrorsToException.h>

using namespace Graphics;

SwapChain::SwapChain( void )
{
}

SwapChain::SwapChain( ComPtr<IDXGISwapChain> dxgi_swap_chain ) :
m_swap_chain( dxgi_swap_chain )
{
}

SwapChain::~SwapChain( void )
{
    //ThrowIfFailed(m_swap_chain->SetFullscreenState( false, nullptr));
    if(m_swap_chain) m_swap_chain->SetFullscreenState( false, nullptr );
}


ComPtr<ID3D11Texture2D> SwapChain::GetBuffer( unsigned index )
{
    ComPtr<ID3D11Texture2D> texture;
    ThrowIfFailed( m_swap_chain->GetBuffer( index, __uuidof( texture ), reinterpret_cast<void**>( texture.GetAddressOf() ) ) );
    return texture;
}

DXGI_SWAP_CHAIN_DESC SwapChain::GetDescription()
{
    DXGI_SWAP_CHAIN_DESC description;
    ThrowIfFailed( m_swap_chain->GetDesc( &description ) );
    return description;
}

float SwapChain::GetAspectRatio()
{
    auto description = GetDescription();
    return static_cast<float>( description.BufferDesc.Width ) / static_cast<float>( description.BufferDesc.Height );
}

DXGI_FRAME_STATISTICS SwapChain::GetFrameStatistics()
{
    DXGI_FRAME_STATISTICS statistics;
    ThrowIfFailed( m_swap_chain->GetFrameStatistics( &statistics ) );
    return statistics;
}

bool SwapChain::IsFullScreen()
{
    ComPtr<IDXGIOutput> output;
    BOOL fullscreen;
    ThrowIfFailed( m_swap_chain->GetFullscreenState( &fullscreen, output.GetAddressOf() ) );
    return ( fullscreen != 0 );
}

unsigned SwapChain::GetLastPresentCount()
{
    unsigned count;
    ThrowIfFailed( m_swap_chain->GetLastPresentCount( &count ) );
    return count;
}

void SwapChain::Present()
{
    ThrowIfFailed( m_swap_chain->Present( 0, 0 ) );
}

void SwapChain::PresentWithVerticalSync( unsigned interval )
{
    ThrowIfFailed( m_swap_chain->Present( interval, DXGI_PRESENT_DO_NOT_SEQUENCE ) );
}

void SwapChain::PresentCustom( unsigned interval, unsigned flags )
{
    ThrowIfFailed( m_swap_chain->Present( interval, flags ) );
}

void SwapChain::ResizeBuffers( unsigned buffer_count, unsigned width, unsigned height, DXGI_FORMAT new_format, DXGI_SWAP_CHAIN_FLAG flags )
{
    ThrowIfFailed( m_swap_chain->ResizeBuffers( buffer_count, width, height, new_format, flags ) );
}