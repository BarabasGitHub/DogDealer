#include "Device.h"
#include <Windows\WindowsErrorsToException.h>
#include <array>
#include <vector>
#include <algorithm>
#include <DirectXColors.h>

#include <Utilities\Datablob.h>
#include "SwapChain.h"

#include <external\DDSTextureLoader\DDSTextureLoader.h>

#ifdef ENABLE_FLOATING_POINT_EXCEPTIONS
#include <float.h>
#endif

using namespace std;
using namespace Graphics;

namespace
{

}


Device::Device() :
m_d3d_device( nullptr ),
m_driver_type( D3D_DRIVER_TYPE_UNKNOWN )
{
}


Device::Device( ComPtr<ID3D11Device> device, D3D_DRIVER_TYPE driver_type ) :
m_d3d_device( device ),
m_driver_type( driver_type )
{
}


Device::~Device( void )
{
}


ComPtr<ID3D11BlendState>  Device::CreateBlendState( D3D11_RENDER_TARGET_BLEND_DESC const & description, bool const alpha_to_coverage )
{
    CD3D11_BLEND_DESC blend_description;
    blend_description.AlphaToCoverageEnable = alpha_to_coverage;
    blend_description.IndependentBlendEnable = false;
    blend_description.RenderTarget[0] = description;
    return CreateBlendState( blend_description );
}


ComPtr<ID3D11BlendState>  Device::CreateBlendState( std::array<D3D11_RENDER_TARGET_BLEND_DESC, 8> const render_target_blend_descriptions, const bool alpha_to_coverage )
{
    CD3D11_BLEND_DESC description;
    description.AlphaToCoverageEnable = alpha_to_coverage;
    description.IndependentBlendEnable = true;
    move( render_target_blend_descriptions.begin(), render_target_blend_descriptions.end(), description.RenderTarget );
    return CreateBlendState( move( description ) );
}


ComPtr<ID3D11BlendState> Device::CreateBlendState( D3D11_BLEND_DESC const & description )
{
    ComPtr<ID3D11BlendState> state;
    ThrowIfFailed( m_d3d_device->CreateBlendState( &description, state.GetAddressOf() ) );
    return state;
}


ComPtr<ID3D11BlendState>  Device::CreateBlendState()
{
    return CreateBlendState( CD3D11_BLEND_DESC( CD3D11_DEFAULT() ) );
}


ComPtr<ID3D11BlendState>  Device::CreateAlphaCoverageBlendState()
{
	// Create default description with alpha-to-coverage enabled
	auto blend_desc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
	blend_desc.AlphaToCoverageEnable = TRUE;

	return CreateBlendState(blend_desc);
}


ComPtr<ID3D11Buffer> Device::CreateIndexBuffer( const Range<uint16_t const *> indices, const bool dynamic )
{
    auto byte_size = uint32_t(Size(indices) * sizeof(First(indices)));
    auto d3d_buffer = CreateIndexBuffer( byte_size, begin(indices), dynamic );
    return d3d_buffer;
}


ComPtr<ID3D11Buffer> Device::CreateIndexBuffer( const Range<uint32_t const *> indices, const bool dynamic )
{
    auto byte_size = uint32_t(Size(indices) * sizeof(First(indices)));
    auto d3d_buffer = CreateIndexBuffer( byte_size, begin(indices), dynamic );
    return d3d_buffer;
}


ComPtr<ID3D11Buffer> Device::CreateIndexBuffer( const unsigned byte_size, const void* index_data, const bool dynamic )
{
    D3D11_BUFFER_DESC description;
    description.ByteWidth = byte_size;
    description.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
    description.BindFlags = D3D11_BIND_INDEX_BUFFER;
    description.CPUAccessFlags = dynamic ? D3D10_CPU_ACCESS_WRITE : 0;
    description.MiscFlags = 0;
    description.StructureByteStride = 0;
    return CreateBuffer( description, index_data );
}


ComPtr<ID3D11Buffer> Device::CreateConstantBuffer( unsigned& byte_size )
{
    // If the bind flag is D3D11_BIND_CONSTANT_BUFFER, you must set the ByteWidth value in multiples of 16,
    // and less than or equal to D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT.
    // http://msdn.microsoft.com/en-us/library/windows/desktop/ff476092%28v=vs.85%29.aspx

    assert( byte_size <= D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 32 );
    // round up to nearest size
    const auto size_multiple = 16u;
    byte_size += size_multiple - 1;
    byte_size /= size_multiple;
    byte_size *= size_multiple;

    D3D11_BUFFER_DESC description;
    description.Usage = D3D11_USAGE_DYNAMIC;
    description.ByteWidth = byte_size;
    description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    description.MiscFlags = 0;
    description.StructureByteStride = 0;
    return CreateBuffer( description );
}


ComPtr<ID3D11Buffer> Device::CreateBuffer( D3D11_BUFFER_DESC const & description, const void* data )
{
    D3D11_SUBRESOURCE_DATA subresource_data;
    subresource_data.pSysMem = data;
    return CreateBuffer( description, &subresource_data );
}


ComPtr<ID3D11Buffer> Device::CreateBuffer( D3D11_BUFFER_DESC const & description, const D3D11_SUBRESOURCE_DATA* data )
{
#ifdef ENABLE_FLOATING_POINT_EXCEPTIONS
    _controlfp_s( nullptr, _MCW_EM, _MCW_EM );
#endif
    ComPtr<ID3D11Buffer> buffer;
    ThrowIfFailed( m_d3d_device->CreateBuffer( &description, data, buffer.GetAddressOf() ) );
#ifdef ENABLE_FLOATING_POINT_EXCEPTIONS
    _controlfp_s( nullptr, _EM_INEXACT, _MCW_EM );
#endif
    return buffer;
}


ComPtr<ID3D11Texture1D> Device::CreateTexture1D( const DXGI_FORMAT format, const unsigned width )
{
    auto description = CD3D11_TEXTURE1D_DESC( format, width );
    return CreateTexture1D( description );
}


ComPtr<ID3D11Texture1D> Device::CreateTexture1D( D3D11_TEXTURE1D_DESC const & description, const D3D11_SUBRESOURCE_DATA* data )
{
    ComPtr<ID3D11Texture1D> texture;
    ThrowIfFailed( m_d3d_device->CreateTexture1D( &description, data, texture.GetAddressOf() ) );
    return texture;
}


ComPtr<ID3D11Texture2D> Device::CreateTexture2D( const DXGI_FORMAT format, const unsigned width, const unsigned height, const unsigned bind_flags )
{
    auto description = CD3D11_TEXTURE2D_DESC( format, width, height );
    description.BindFlags = bind_flags;
    return CreateTexture2D( description );
}


ComPtr<ID3D11Texture2D> Device::CreateTexture2D( D3D11_TEXTURE2D_DESC const & description, const D3D11_SUBRESOURCE_DATA* data )
{
#ifdef ENABLE_FLOATING_POINT_EXCEPTIONS
    _controlfp_s( nullptr, _MCW_EM, _MCW_EM );
#endif
    ComPtr<ID3D11Texture2D> texture;
    ThrowIfFailed( m_d3d_device->CreateTexture2D( &description, data, texture.GetAddressOf() ) );
#ifdef ENABLE_FLOATING_POINT_EXCEPTIONS
    _controlfp_s( nullptr, _EM_INEXACT, _MCW_EM );
#endif
    return texture;
}


ComPtr<ID3D11Texture2D> Device::CreateTexture2D( DataBlob const & blob, bool const forceSRGB )
{
    D3D11_USAGE const usage = D3D11_USAGE_DEFAULT;
    unsigned const bind_flags = D3D11_BIND_SHADER_RESOURCE;
    unsigned const cpu_access_flags = 0;
    unsigned const misc_flags = 0;

    ComPtr<ID3D11Texture2D> texture_2d;
    DirectX::DDS_ALPHA_MODE alpha_mode;
    ThrowIfFailed( CreateDDSTextureFromMemoryEx( m_d3d_device, blob.data( ), blob.size( ), blob.size( ),
        usage, bind_flags, cpu_access_flags, misc_flags, forceSRGB,
        reinterpret_cast<ID3D11Resource**>( texture_2d.GetAddressOf() ), nullptr, &alpha_mode ) );
    return texture_2d;
}


ComPtr<ID3D11Texture3D> Device::CreateTexture3D( const DXGI_FORMAT format, const unsigned width, const unsigned height, const unsigned depth )
{
    auto description = CD3D11_TEXTURE3D_DESC( format, width, height, depth );
    return CreateTexture3D( description );
}


ComPtr<ID3D11Texture3D> Device::CreateTexture3D( D3D11_TEXTURE3D_DESC const & description, const D3D11_SUBRESOURCE_DATA* data )
{
    ComPtr<ID3D11Texture3D> texture;
    ThrowIfFailed( m_d3d_device->CreateTexture3D( &description, data, texture.GetAddressOf() ) );
    return texture;
}


ComPtr<ID3D11Texture2D> Device::CreateDepthStencilTexture2D( ComPtr<ID3D11Texture2D> texture, const DXGI_FORMAT format )
{
    D3D11_TEXTURE2D_DESC description;
    texture->GetDesc( &description );
    description.Format = format;
    description.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    return CreateTexture2D( description );
}


ComPtr<ID3D11Texture2D> Device::CreateDepthStencilTexture2D( const unsigned width, const unsigned height, const DXGI_FORMAT format )
{
    auto description = CD3D11_TEXTURE2D_DESC( format, width, height, 1, 0, D3D11_BIND_DEPTH_STENCIL );
    return CreateTexture2D( description );
}


ComPtr<ID3D11DepthStencilState> Device::CreateDepthStencilState( const bool depth, const bool stencil )
{
    CD3D11_DEPTH_STENCIL_DESC description;
    // Depth test parameters
    description.DepthEnable = depth;
    description.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    description.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    // Stencil test parameters
    description.StencilEnable = stencil;
    description.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    description.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

    // Stencil operations if pixel is front-facing
    description.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    description.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    description.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    description.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing
    description.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    description.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    description.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    description.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    return CreateDepthStencilState( description );
}


ComPtr<ID3D11DepthStencilState> Device::CreateDepthStencilState( D3D11_DEPTH_STENCIL_DESC const & description )
{
    ComPtr<ID3D11DepthStencilState> state;
    ThrowIfFailed( m_d3d_device->CreateDepthStencilState( &description, state.GetAddressOf() ) );
    return state;
}


ComPtr<ID3D11DepthStencilView> Device::CreateDepthStencilView( ID3D11Resource * const resource, D3D11_DEPTH_STENCIL_VIEW_DESC const * const description )
{
    ComPtr<ID3D11DepthStencilView> view;
    ThrowIfFailed( m_d3d_device->CreateDepthStencilView( resource, description, view.GetAddressOf() ) );
    return view;
}


D3D11_RASTERIZER_DESC Device::GetDefaultRasterizerDescription()
{
    D3D11_RASTERIZER_DESC description;
    description.FillMode = D3D11_FILL_SOLID;
    // description.FillMode = D3D11_FILL_WIREFRAME;
    description.CullMode = D3D11_CULL_BACK;
    description.FrontCounterClockwise = true;
    description.DepthBias = 0;
    description.DepthBiasClamp = 0.0f;
    description.SlopeScaledDepthBias = 0.0f;
    description.DepthClipEnable = true;
    description.ScissorEnable = false;
    description.MultisampleEnable = true;
    description.AntialiasedLineEnable = true;
    return description;
}


ComPtr<ID3D11RasterizerState> Device::CreateRasterizerState( D3D11_RASTERIZER_DESC const & description )
{
#ifdef ENABLE_FLOATING_POINT_EXCEPTIONS
    _controlfp_s( nullptr, _MCW_EM, _MCW_EM );
#endif
    ComPtr<ID3D11RasterizerState> state;
    ThrowIfFailed( m_d3d_device->CreateRasterizerState( &description, state.GetAddressOf() ) );
#ifdef ENABLE_FLOATING_POINT_EXCEPTIONS
    _controlfp_s( nullptr, _EM_INEXACT, _MCW_EM );
#endif
    return state;
}


ComPtr<ID3D11RenderTargetView> Device::CreateRenderTargetView( ID3D11Resource* const resource, D3D11_RENDER_TARGET_VIEW_DESC const * const description )
{
    ComPtr<ID3D11RenderTargetView> view;
    ThrowIfFailed( m_d3d_device->CreateRenderTargetView( resource, description, view.GetAddressOf() ) );
    return view;
}


ComPtr<ID3D11SamplerState> Device::CreateSamplerState( const TextureFiltering texture_filtering, const D3D11_TEXTURE_ADDRESS_MODE texture_adress_mode )
{
    D3D11_SAMPLER_DESC description;
    description.AddressU = description.AddressV = description.AddressW = texture_adress_mode;
    description.MipLODBias = 0;
    switch(texture_filtering)
    {
        case TextureFiltering::NearestNeighbour:
            description.MaxAnisotropy = 1;
            description.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            break;
        case TextureFiltering::Linear:
            description.MaxAnisotropy = 1;
            description.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            break;
        case TextureFiltering::Anisotropic_2x:
            description.MaxAnisotropy = 2;
            description.Filter = D3D11_FILTER_ANISOTROPIC;
            break;
        case TextureFiltering::Anisotropic_4x:
            description.MaxAnisotropy = 4;
            description.Filter = D3D11_FILTER_ANISOTROPIC;
            break;
        case TextureFiltering::Anisotropic_8x:
            description.MaxAnisotropy = 8;
            description.Filter = D3D11_FILTER_ANISOTROPIC;
            break;
        case TextureFiltering::Anisotropic_16x:
            description.MaxAnisotropy = 16;
            description.Filter = D3D11_FILTER_ANISOTROPIC;
            break;
    }
    description.ComparisonFunc = D3D11_COMPARISON_NEVER;
    auto color = DirectX::Colors::HotPink; // so we can see when it happens
    description.BorderColor[0] = color[0];
    description.BorderColor[1] = color[1];
    description.BorderColor[2] = color[2];
    description.BorderColor[3] = color[3];
    description.MinLOD = numeric_limits<float>::lowest();
    description.MaxLOD = numeric_limits<float>::max();
    return CreateSamplerState( description );
}


ComPtr<ID3D11SamplerState> Device::CreateSamplerState( D3D11_SAMPLER_DESC const & description )
{
#ifdef ENABLE_FLOATING_POINT_EXCEPTIONS
    _controlfp_s( nullptr, _MCW_EM, _MCW_EM );
#endif
    ComPtr<ID3D11SamplerState> state;
    ThrowIfFailed( m_d3d_device->CreateSamplerState( &description, state.GetAddressOf() ) );
#ifdef ENABLE_FLOATING_POINT_EXCEPTIONS
    _controlfp_s( nullptr, _EM_INEXACT, _MCW_EM );
#endif
    return state;
}


ComPtr<ID3D11ShaderResourceView> Device::CreateShaderResourceView( ID3D11Resource* const resource, D3D11_SHADER_RESOURCE_VIEW_DESC const * const description )
{
#ifdef ENABLE_FLOATING_POINT_EXCEPTIONS
    _controlfp_s( nullptr, _MCW_EM, _MCW_EM );
#endif
    ComPtr<ID3D11ShaderResourceView> view;
    ThrowIfFailed( m_d3d_device->CreateShaderResourceView( resource, description, view.GetAddressOf() ) );
#ifdef ENABLE_FLOATING_POINT_EXCEPTIONS
    _controlfp_s( nullptr, _EM_INEXACT, _MCW_EM );
#endif
    return view;
}

//shared_ptr<VertexShader> Device::CreateVertexShader( const DataBlob& byte_code, shared_ptr<InputLayout> input_layout )
//{
//    ComPtr<ID3D11VertexShader> d3d_shader;
//    ThrowIfFailed(m_d3d_device->CreateVertexShader( byte_code.GetBufferPointer(), byte_code.GetBufferSize(), nullptr /*class_linkage*/, d3d_shader.GetAddressOf() ));
//    return make_shared<VertexShader>( byte_code, move(d3d_shader), move(input_layout) );
//}


ComPtr<ID3D11VertexShader> Device::CreateVertexShader( const DataBlob& byte_code )
{
    ComPtr<ID3D11VertexShader> d3d_shader;
    ThrowIfFailed( m_d3d_device->CreateVertexShader( byte_code.data(), byte_code.size(), nullptr /*class_linkage*/, d3d_shader.GetAddressOf() ) );
    return d3d_shader;
}


ComPtr<ID3D11HullShader> Device::CreateHullShader( const DataBlob& byte_code )
{
    ComPtr<ID3D11HullShader> d3d_shader;
    ThrowIfFailed( m_d3d_device->CreateHullShader( byte_code.data(), byte_code.size(), nullptr /*class_linkage*/, d3d_shader.GetAddressOf() ) );
    return d3d_shader;
}


ComPtr<ID3D11DomainShader> Device::CreateDomainShader( const DataBlob& byte_code )
{
    ComPtr<ID3D11DomainShader> d3d_shader;
    ThrowIfFailed( m_d3d_device->CreateDomainShader( byte_code.data(), byte_code.size(), nullptr /*class_linkage*/, d3d_shader.GetAddressOf() ) );
    return d3d_shader;
}


ComPtr<ID3D11GeometryShader> Device::CreateGeometryShader( const DataBlob& byte_code )
{
    ComPtr<ID3D11GeometryShader> d3d_shader;
    ThrowIfFailed( m_d3d_device->CreateGeometryShader( byte_code.data(), byte_code.size(), nullptr /*class_linkage*/, d3d_shader.GetAddressOf() ) );
    return d3d_shader;
}


ComPtr<ID3D11PixelShader> Device::CreatePixelShader( const DataBlob& byte_code )
{
    ComPtr<ID3D11PixelShader> d3d_shader;
    ThrowIfFailed( m_d3d_device->CreatePixelShader( byte_code.data(), byte_code.size(), nullptr /*class_linkage*/, d3d_shader.GetAddressOf() ) );
    return d3d_shader;
}


ComPtr<ID3D11InputLayout> Device::CreateInputLayout( const InputElementDescriptions &info, const DataBlob& byte_code )
{
    ComPtr<ID3D11InputLayout> d3d_input_layout;
    ThrowIfFailed( m_d3d_device->CreateInputLayout( info.m_d3d_input_element_descriptions.data(), static_cast<unsigned>( info.m_d3d_input_element_descriptions.size() ), byte_code.data(),
        static_cast<unsigned>( byte_code.size() ), d3d_input_layout.GetAddressOf() ) );
    return d3d_input_layout;
}


ComPtr<ID3D11ComputeShader> Device::CreateComputeShader( const DataBlob& byte_code )
{
    ComPtr<ID3D11ComputeShader> d3d_shader;
    ThrowIfFailed( m_d3d_device->CreateComputeShader( byte_code.data(), byte_code.size(), nullptr /*class_linkage*/, d3d_shader.GetAddressOf() ) );
    return d3d_shader;
}


ComPtr<ID3D11UnorderedAccessView> Device::CreateUnorderedAccessView( ComPtr<ID3D11Resource> resource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* description )
{
    ComPtr<ID3D11UnorderedAccessView> view;
    ThrowIfFailed( m_d3d_device->CreateUnorderedAccessView( resource, description, view.GetAddressOf() ) );
    return view;
}


ComPtr<ID3D11ClassLinkage> Device::CreateClassLinkage()
{
    ComPtr<ID3D11ClassLinkage> class_linkage;
    ThrowIfFailed( m_d3d_device->CreateClassLinkage( class_linkage.GetAddressOf() ) );
    return class_linkage;
}


D3D11_BUFFER_DESC Device::CreateVertexBufferDescription(uint32_t const byte_size, bool const dynamic)
{
    CD3D11_BUFFER_DESC description;
    description.ByteWidth = byte_size;
    description.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
    description.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    description.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
    description.MiscFlags = 0;
    description.StructureByteStride = 0;
    return description;
}