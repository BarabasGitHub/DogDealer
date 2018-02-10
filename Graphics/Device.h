#pragma once
#include "DirectX/Direct3D11.h"

#include <array>
#include <vector>
#include <cstdint>
#include <string>

#include "DeviceContext.h"
#include "TextureFiltering.h"
#include "InputElementDescriptions.h"

#include <Utilities\ComPtr.h>
#include <Utilities\Datablob.h>
#include <Utilities\Range.h>

// forward declarations

namespace Graphics
{

    struct Device
    {
        // This class is responsible (at least) for (efficiently) creating and destroying the resources on the GPU.
        // currently it just naively creates what it is asked for and doesn't do anything fancy at all.
    protected:
        ComPtr<ID3D11Device> m_d3d_device;
        D3D_DRIVER_TYPE m_driver_type;

    public:
        Device();
        Device( ComPtr<ID3D11Device> device, D3D_DRIVER_TYPE driver_type = D3D_DRIVER_TYPE_UNKNOWN );
        ~Device( void );

        ID3D11Device* operator->( ) noexcept
        {
            return m_d3d_device.Get();
        }

        ID3D11Device const * operator->( ) const noexcept
        {
            return m_d3d_device.Get();
        }

        // not sure what should stay public of this.

        unsigned GetCreationFlags() const noexcept
        {
            return m_d3d_device->GetCreationFlags();
        }
        D3D_FEATURE_LEVEL GetFeatureLevel() const noexcept
        {
            return m_d3d_device->GetFeatureLevel();
        }
        D3D_DRIVER_TYPE GetDriverType() const noexcept
        {
            return m_driver_type;
        }
        DeviceContext GetImmediateContext() noexcept
        {
            ComPtr<ID3D11DeviceContext> context;
            m_d3d_device->GetImmediateContext( context.GetAddressOf() );
            return context;
        }


        ComPtr<ID3D11BlendState> CreateAlphaCoverageBlendState();
        ComPtr<ID3D11BlendState> CreateBlendState();

        ComPtr<ID3D11BlendState> CreateBlendState( const D3D11_RENDER_TARGET_BLEND_DESC& description, const bool alpha_to_coverage = false );
        ComPtr<ID3D11BlendState> CreateBlendState( const std::array<D3D11_RENDER_TARGET_BLEND_DESC, 8>, const bool alpha_to_coverage = false );

	    ComPtr<ID3D11BlendState> CreateBlendState( const D3D11_BLEND_DESC& description );


		ComPtr<ID3D11SamplerState> CreateSamplerState( const TextureFiltering texture_filtering = TextureFiltering::Linear, const D3D11_TEXTURE_ADDRESS_MODE texture_adress_mode = D3D11_TEXTURE_ADDRESS_WRAP );
        ComPtr<ID3D11SamplerState> CreateSamplerState( const D3D11_SAMPLER_DESC& description );

        static D3D11_RASTERIZER_DESC GetDefaultRasterizerDescription();
        ComPtr<ID3D11RasterizerState> CreateRasterizerState( const D3D11_RASTERIZER_DESC& description );

        ComPtr<ID3D11DepthStencilState> CreateDepthStencilState( const bool depth = true, const bool stencil = false );
        ComPtr<ID3D11DepthStencilState> CreateDepthStencilState( const D3D11_DEPTH_STENCIL_DESC& description );

        template<size_t N>
        ComPtr<ID3D11Buffer> CreateIndexBuffer( Range< std::array<uint16_t, N> const * > indices, const bool dynamic = false )
        {
            auto byte_size = static_cast<unsigned>( indices.size() * sizeof( indices.front() ) );
            auto d3d_buffer = CreateIndexBuffer( byte_size, indices.data(), dynamic );
            return d3d_buffer;
        }

        template<size_t N>
        ComPtr<ID3D11Buffer> CreateIndexBuffer( Range< std::array<uint32_t, N> const * > indices, const bool dynamic = false )
        {
            auto byte_size = static_cast<unsigned>( indices.size() * sizeof( indices.front() ) );
            auto d3d_buffer = CreateIndexBuffer( byte_size, indices.data(), dynamic );
            return d3d_buffer;

        }

        ComPtr<ID3D11Buffer> CreateIndexBuffer( const Range<uint16_t const *> indices, const bool dynamic = false );
        ComPtr<ID3D11Buffer> CreateIndexBuffer( const Range<uint32_t const *> indices, const bool dynamic = false );

        template<typename T> ComPtr<ID3D11Buffer> CreateVertexBuffer( const std::vector<T>& vertex_data, const bool dynamic = false )
        {
            return CreateVertexBuffer( CreateRange(vertex_data), dynamic );
        }

        template<typename T> ComPtr<ID3D11Buffer> CreateVertexBuffer( Range<T*> vertex_data, const bool dynamic = false )
        {
            return CreateVertexBuffer( static_cast<unsigned>( Size(vertex_data) ), begin(vertex_data), dynamic );
        }

        template<typename T>
        ComPtr<ID3D11Buffer> CreateVertexBuffer( const unsigned size, const T* vertex_data, const bool dynamic = false )
        {
            return CreateVertexBuffer<void>( size * D3Dsizeof<T>(), vertex_data, dynamic );
        }

        template<>
        ComPtr<ID3D11Buffer> CreateVertexBuffer<void>( unsigned const byte_size, void const * const vertex_data, bool const dynamic )
        {
            auto description = CreateVertexBufferDescription( byte_size, dynamic );
            return CreateBuffer( description, vertex_data );
        }

        ComPtr<ID3D11Buffer> CreateVertexBuffer( unsigned const byte_size )
        {
            auto description = CreateVertexBufferDescription( byte_size, true );
            return CreateBuffer( description );
        }

        ComPtr<ID3D11Buffer> CreateConstantBuffer( unsigned& byte_size );


        ComPtr<ID3D11Texture2D> CreateTexture2D( const DXGI_FORMAT format, const unsigned width, const unsigned height, const unsigned bind_flags );
        ComPtr<ID3D11Texture2D> CreateTexture2D( const D3D11_TEXTURE2D_DESC& description, const D3D11_SUBRESOURCE_DATA* data = nullptr );
        ComPtr<ID3D11Texture2D> CreateTexture2D( DataBlob const & blob, bool const forceSRGB = false );

        ComPtr<ID3D11Texture2D> CreateDepthStencilTexture2D( const unsigned width, const unsigned height, const DXGI_FORMAT format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT );
        ComPtr<ID3D11Texture2D> CreateDepthStencilTexture2D( ComPtr<ID3D11Texture2D> texture, const DXGI_FORMAT format );

        ComPtr<ID3D11ShaderResourceView> CreateShaderResourceView( ID3D11Resource * const resource, D3D11_SHADER_RESOURCE_VIEW_DESC const * const description = nullptr );
        ComPtr<ID3D11DepthStencilView> CreateDepthStencilView( ID3D11Resource * const resource, D3D11_DEPTH_STENCIL_VIEW_DESC const * const description = nullptr );
        ComPtr<ID3D11RenderTargetView> CreateRenderTargetView( ID3D11Resource * const resource, D3D11_RENDER_TARGET_VIEW_DESC const * const description = nullptr );

		ComPtr<ID3D11VertexShader> CreateVertexShader( DataBlob const& byte_code );
        ComPtr<ID3D11HullShader> CreateHullShader( DataBlob const& byte_code );
        ComPtr<ID3D11DomainShader> CreateDomainShader( DataBlob const& byte_code );
        ComPtr<ID3D11GeometryShader> CreateGeometryShader( DataBlob const& byte_code );
        ComPtr<ID3D11PixelShader> CreatePixelShader( DataBlob const& byte_code );
        ComPtr<ID3D11InputLayout> CreateInputLayout( InputElementDescriptions const & info, DataBlob const& byte_code );
        ComPtr<ID3D11ComputeShader> CreateComputeShader( DataBlob const & byte_code );


    private:


        // note DirectX only supports uint32 and uint16 index buffers
        ComPtr<ID3D11Buffer> CreateIndexBuffer( const unsigned byte_size, const void* index_data = nullptr, const bool dynamic = false );
        ComPtr<ID3D11Buffer> CreateBuffer( const D3D11_BUFFER_DESC& description, const void* data );
        ComPtr<ID3D11Buffer> CreateBuffer( const D3D11_BUFFER_DESC& description, const D3D11_SUBRESOURCE_DATA* data = nullptr );

        static D3D11_BUFFER_DESC CreateVertexBufferDescription( uint32_t const byte_size, bool const dynamic );

        // for now we don't support 1d and 3d textures
        ComPtr<ID3D11Texture1D> CreateTexture1D( const DXGI_FORMAT format, const unsigned width );
        ComPtr<ID3D11Texture1D> CreateTexture1D( const D3D11_TEXTURE1D_DESC& description, const D3D11_SUBRESOURCE_DATA* data = nullptr );
        ComPtr<ID3D11Texture3D> CreateTexture3D( const DXGI_FORMAT format, const unsigned width, const unsigned height, const unsigned depth );
        ComPtr<ID3D11Texture3D> CreateTexture3D( const D3D11_TEXTURE3D_DESC& description, const D3D11_SUBRESOURCE_DATA* data = nullptr );

        // other advanced stuff
        ComPtr<ID3D11ClassLinkage> CreateClassLinkage();

        // I think this belongs to the compute shader, not 100% sure
        ComPtr<ID3D11UnorderedAccessView> CreateUnorderedAccessView( ComPtr<ID3D11Resource> resource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* description );


    };
}