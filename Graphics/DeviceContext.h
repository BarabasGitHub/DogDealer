#pragma once
#include "DirectX\Direct3D11.h"
#include <Utilities\ComPtr.h>

namespace Graphics
{
    typedef ComPtr<ID3D11DeviceContext> DeviceContext;


    inline void SetShader( ID3D11DeviceContext* context, ID3D11VertexShader* shader, ID3D11ClassInstance * const * class_instances, unsigned class_instances_count )
    {
        context->VSSetShader( shader, class_instances, class_instances_count );
    }

    inline void SetShader( ID3D11DeviceContext* context, ID3D11HullShader* shader, ID3D11ClassInstance * const * class_instances, unsigned class_instances_count )
    {
        context->HSSetShader( shader, class_instances, class_instances_count );
    }

    inline void SetShader( ID3D11DeviceContext* context, ID3D11DomainShader* shader, ID3D11ClassInstance * const * class_instances, unsigned class_instances_count )
    {
        context->DSSetShader( shader, class_instances, class_instances_count );
    }

    inline void SetShader( ID3D11DeviceContext* context, ID3D11GeometryShader* shader, ID3D11ClassInstance * const * class_instances, unsigned class_instances_count )
    {
        context->GSSetShader( shader, class_instances, class_instances_count );
    }

    inline void SetShader( ID3D11DeviceContext* context, ID3D11PixelShader* shader, ID3D11ClassInstance * const * class_instances, unsigned class_instances_count )
    {
        context->PSSetShader( shader, class_instances, class_instances_count );
    }

    template<typename ShaderType>
    void SetConstantBuffers( ID3D11DeviceContext* context, unsigned start_slot, unsigned count, ID3D11Buffer * const * buffers );

    template<>
    inline void SetConstantBuffers<ID3D11VertexShader>( ID3D11DeviceContext* context, unsigned start_slot, unsigned count, ID3D11Buffer * const * buffers )
    {
        context->VSSetConstantBuffers( start_slot, count, buffers );
    }

    template<>
    inline void SetConstantBuffers<ID3D11HullShader>( ID3D11DeviceContext* context, unsigned start_slot, unsigned count, ID3D11Buffer * const * buffers )
    {
        context->HSSetConstantBuffers( start_slot, count, buffers );
    }

    template<>
    inline void SetConstantBuffers<ID3D11DomainShader>( ID3D11DeviceContext* context, unsigned start_slot, unsigned count, ID3D11Buffer * const * buffers )
    {
        context->DSSetConstantBuffers( start_slot, count, buffers );
    }

    template<>
    inline void SetConstantBuffers<ID3D11GeometryShader>( ID3D11DeviceContext* context, unsigned start_slot, unsigned count, ID3D11Buffer * const * buffers )
    {
        context->GSSetConstantBuffers( start_slot, count, buffers );
    }

    template<>
    inline void SetConstantBuffers<ID3D11PixelShader>( ID3D11DeviceContext* context, unsigned start_slot, unsigned count, ID3D11Buffer * const * buffers )
    {
        context->PSSetConstantBuffers( start_slot, count, buffers );
    }

    template<typename ShaderType>
    void SetSamplers( ID3D11DeviceContext* context, unsigned offset, unsigned count, ID3D11SamplerState * const * buffers );

    template<>
    inline void SetSamplers<ID3D11VertexShader>( ID3D11DeviceContext* context, unsigned offset, unsigned count, ID3D11SamplerState * const * samplers )
    {
        context->VSSetSamplers( offset, count, samplers );
    }

    template<>
    inline void SetSamplers<ID3D11HullShader>( ID3D11DeviceContext* context, unsigned offset, unsigned count, ID3D11SamplerState * const * samplers )
    {
        context->HSSetSamplers( offset, count, samplers );
    }

    template<>
    inline void SetSamplers<ID3D11DomainShader>( ID3D11DeviceContext* context, unsigned offset, unsigned count, ID3D11SamplerState * const * samplers )
    {
        context->DSSetSamplers( offset, count, samplers );
    }

    template<>
    inline void SetSamplers<ID3D11GeometryShader>( ID3D11DeviceContext* context, unsigned offset, unsigned count, ID3D11SamplerState * const * samplers )
    {
        context->GSSetSamplers( offset, count, samplers );
    }

    template<>
    inline void SetSamplers<ID3D11PixelShader>( ID3D11DeviceContext* context, unsigned offset, unsigned count, ID3D11SamplerState * const * samplers )
    {
        context->PSSetSamplers( offset, count, samplers );
    }

    template<typename ShaderType>
    void SetShaderResources( ID3D11DeviceContext* context, unsigned offset, unsigned count, ID3D11ShaderResourceView * const * views );

    template<>
    inline void SetShaderResources<ID3D11VertexShader>( ID3D11DeviceContext* context, unsigned offset, unsigned count, ID3D11ShaderResourceView * const * views )
    {
        context->VSSetShaderResources( offset, count, views );
    }

    template<>
    inline void SetShaderResources<ID3D11HullShader>( ID3D11DeviceContext* context, unsigned offset, unsigned count, ID3D11ShaderResourceView * const * views )
    {
        context->HSSetShaderResources( offset, count, views );
    }

    template<>
    inline void SetShaderResources<ID3D11DomainShader>( ID3D11DeviceContext* context, unsigned offset, unsigned count, ID3D11ShaderResourceView * const * views )
    {
        context->DSSetShaderResources( offset, count, views );
    }

    template<>
    inline void SetShaderResources<ID3D11GeometryShader>( ID3D11DeviceContext* context, unsigned offset, unsigned count, ID3D11ShaderResourceView * const * views )
    {
        context->GSSetShaderResources( offset, count, views );
    }

    template<>
    inline void SetShaderResources<ID3D11PixelShader>( ID3D11DeviceContext* context, unsigned offset, unsigned count, ID3D11ShaderResourceView * const * views )
    {
        context->PSSetShaderResources( offset, count, views );
    }

}