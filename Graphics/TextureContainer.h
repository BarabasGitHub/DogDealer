#pragma once
#include "IDs.h"
#include "DirectX\Direct3D11.h"

#include <vector>
#include <deque>
#include <istream>
#include <cassert>

namespace Graphics
{

    struct Device;

    struct TextureContainer
    {
    public:

        std::vector<ComPtr<ID3D11Texture2D>> m_textures_2d;
        std::vector<uint8_t> m_generation_2d;
        std::deque<unsigned> m_free_list_2d;

        bool IsValid( Texture2DID id ) const
        {
            assert( m_textures_2d.size() == m_generation_2d.size() );
            return id.index < m_textures_2d.size() && id.generation == m_generation_2d[id.index];
        }

        ID3D11Texture2D * GetTexture2D( Texture2DID texture_id )
        {
            assert( IsValid( texture_id ) );
            return m_textures_2d[texture_id.index];
        }

        ID3D11Texture2D const * GetTexture2D( Texture2DID texture_id ) const
        {
            assert( IsValid( texture_id ) );
            return m_textures_2d[texture_id.index];
        }

        Texture2DID AddTexture( ComPtr<ID3D11Texture2D> texture );

        void RemoveTexture2D( Texture2DID texture_id )
        {
            if( IsValid(texture_id) )
            {
                m_textures_2d[texture_id.index].Reset();
                ++m_generation_2d[texture_id.index];
                m_free_list_2d.push_back(texture_id.index);
            }
        }

        Texture2DID CreateDepthStencilTexture2D( Device& device, ComPtr<ID3D11Texture2D> texture, DXGI_FORMAT format );


		Texture2DID LoadTexture(Device& device, std::istream& data_stream, const bool force_srgb = false);
    };
}
