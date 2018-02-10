#include "TextureContainer.h"

#include "Device.h"

#include <Utilities\ContainerHelpers.h>

#include <iterator>

using namespace Graphics;


Texture2DID TextureContainer::CreateDepthStencilTexture2D( Device& device, ComPtr<ID3D11Texture2D> texture, DXGI_FORMAT format )
{
    auto element = device.CreateDepthStencilTexture2D( texture, format );
    return AddTexture( element );
}

Texture2DID TextureContainer::AddTexture( ComPtr<ID3D11Texture2D> texture )
{
    Texture2DID id;
    Add( id, std::move( texture ), m_textures_2d, m_generation_2d, m_free_list_2d );
    return id;
}

Texture2DID TextureContainer::LoadTexture( Device& device, std::istream& data_stream, const bool force_srgb )
{
    // load the texture 
    auto blob = CreateDataBlobTillEndOfStream( data_stream );
    auto texture = device.CreateTexture2D( blob, force_srgb );
    return AddTexture( texture );
}