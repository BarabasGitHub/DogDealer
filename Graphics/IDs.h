#pragma once

#include <Utilities\Handle.h>

// d3d structs, declared here so we don't have to include the whole directx header with all the muck that comes with it
// resource view container
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;
// shader container
struct ID3D11VertexShader;
struct ID3D11HullShader;
struct ID3D11DomainShader;
struct ID3D11GeometryShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
// texture container
struct ID3D11Texture2D;

namespace Graphics
{

    // constant buffer container
    struct ConstantBuffer;
    typedef Handle<ConstantBuffer> ConstantBufferID;
    const ConstantBufferID c_invalid_constant_buffer_id = {ConstantBufferID::index_t(-1), 0};

    // index buffer container
    struct IndexBufferInfo;
    typedef Handle<IndexBufferInfo> IndexBufferID;

    const IndexBufferID c_invalid_index_buffer_id = { IndexBufferID::index_t( -1 ), 0 };

    // resource view container
    typedef Handle<ID3D11RenderTargetView> RenderTargetViewID;
    typedef Handle<ID3D11DepthStencilView> DepthStencilViewID;
    typedef Handle<ID3D11ShaderResourceView> ShaderRersourceViewID;
    const ShaderRersourceViewID c_invalid_shader_resource_view_id = {ShaderRersourceViewID::index_t(-1), 0};

    // shader container
    typedef Handle<ID3D11VertexShader> VertexShaderID;
    typedef Handle<ID3D11HullShader> HullShaderID;
    typedef Handle<ID3D11DomainShader> DomainShaderID;
    typedef Handle<ID3D11GeometryShader> GeometryShaderID;
    typedef Handle<ID3D11PixelShader> PixelShaderID;
    typedef Handle<ID3D11InputLayout> InputLayoutID;

    // texture container
    typedef Handle<ID3D11Texture2D> Texture2DID;
    const Texture2DID c_invalid_texture_2d_id = {Texture2DID::index_t(-1), 0};

    // vertex buffer container
    struct VertexBufferInfo;
    typedef Handle<VertexBufferInfo> VertexBufferID;

    const VertexBufferID c_invalid_vertex_buffer_id = { VertexBufferID::index_t( -1 ), 0 };
}
