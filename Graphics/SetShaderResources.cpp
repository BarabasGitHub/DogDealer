#include "SetShaderResources.h"

#include "DirectX\Direct3D11.h"
#include "ConstantBufferContainer.h"
#include "ConstantBufferTypeAndIDFunctions.h"
#include "ConstantBufferTypes.h"
#include "InputElementDescriptions.h"
#include "ShaderContainer.h"
#include "ShaderType.h"
#include "Structures.h"

#include <Utilities\IntegerRange.h>

#include <array>

namespace
{
    using namespace Graphics;

    class ShaderSetter
    {
        ShaderContainer const * shader_container;
        ID3D11DeviceContext* device_context;
        InputLayoutID input_layout_id;

    public:

        ShaderSetter(
            ShaderContainer const & shader_container,
            ID3D11DeviceContext* device_context ) :
            shader_container( &shader_container ),
            device_context( device_context ),
            input_layout_id( c_invalid_layout_id )
        {
        }

        template<typename ShaderIDType>
        void Set( ShaderIDType const shader_id )
        {
            typedef typename ShaderIDType::object_t ShaderType;

            if( shader_id.index != c_invalid_shader_id.index )
            {
                // set the shader on the device context
                auto const & shader_elements = shader_container->GetElements( shader_id );
                auto const shader = shader_container->GetShader( shader_id );
                SetShader( device_context, shader, nullptr, 0 );

                // set the input layout if it's not yet set
                if( input_layout_id.index == c_invalid_layout_id.index )
                {
                    input_layout_id = shader_elements.input_layout_id;
                    device_context->IASetInputLayout( shader_container->GetInputLayout( input_layout_id ) );
                }

                // assuming a ComPtr is nothing more than a pointer with methods
                static_assert( sizeof( ComPtr<ID3D11SamplerState> ) == sizeof( ID3D11SamplerState* ),
                               "ComPtr<ID3D11SamplerState> and ID3D11SamplerState* do not have the same size, reinterpret cast will not work!" );
                auto samplers = reinterpret_cast<ID3D11SamplerState* const *>( shader_elements.sampler_states.data() );
                SetSamplers<ShaderType>( device_context, 0, static_cast<unsigned>( shader_elements.sampler_states.size() ), samplers );
            }
            else
            {
                ShaderType* const shader = nullptr;
                SetShader( device_context, shader, nullptr, 0 );
            }
        }

        InputLayoutID GetInputLayoutID() const { return input_layout_id; }
    };


    class ConstantBufferSetter
    {
        ShaderContainer const * shader_container;
        ConstantBufferContainer const * buffer_container;
        ID3D11DeviceContext* device_context;
        Range<ConstantBufferTypeAndID const *> constant_buffers;

    public:

        ConstantBufferSetter(
            ShaderContainer const & shader_container,
            ConstantBufferContainer const & buffer_container,
            ID3D11DeviceContext* device_context,
            Range<ConstantBufferTypeAndID const *> constant_buffers ) :
            shader_container( &shader_container ),
            buffer_container( &buffer_container),
            device_context( device_context ),
            constant_buffers( constant_buffers )
        {
        }

        template<typename ShaderIDType>
        void Set( ShaderIDType const shader_id )
        {
            typedef typename ShaderIDType::object_t ShaderType;

            if( shader_id.index != c_invalid_shader_id.index )
            {
                auto const & shader_elements = shader_container->GetElements( shader_id );
                SetConstantBuffers<ShaderType>( shader_elements.constant_buffer_types );
            }
        }

    private:

        template<typename ShaderType>
        void SetConstantBuffers( std::vector<ConstantBufferType> const & types )
        {
            assert( types.size() <= D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT );
            std::array<ID3D11Buffer*, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT> constant_buffer_array;

            // make an array of buffers that we will set on the device...
            auto types_size = uint32_t( types.size() );
            for( auto index = 0u; index < types_size; ++index )
            {
                auto id = GetBufferID(constant_buffers, types[index]);
                constant_buffer_array[index] = buffer_container->GetBuffer(id);
            }

            // assume we're starting at slot 0
            Graphics::SetConstantBuffers<ShaderType>( device_context, 0, types_size, constant_buffer_array.data() );
        }
    };


    class TextureSetter
    {
        std::array<ID3D11ShaderResourceView*, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT> shader_resource_view_array;
        ID3D11ShaderResourceView* const * shader_resource_views;
        TextureType const * texture_types_begin, *texture_types_end;
        ID3D11DeviceContext* device_context;

    public:
        TextureSetter( decltype( device_context ) device_context_in, decltype( shader_resource_views ) shader_resource_views_in, TextureType const * texture_types_begin_in, unsigned type_and_resource_view_count ) :
            shader_resource_views( shader_resource_views_in ),
            texture_types_begin( texture_types_begin_in ),
            texture_types_end( texture_types_begin + type_and_resource_view_count ),
            device_context( device_context_in )
        {
        }

        template<typename ShaderType>
        void SetTextures( std::vector<TextureType> const & texture_types )
        {
            auto array_iterator = shader_resource_view_array.begin();

            for( auto const & texture_type : texture_types )
            {
                auto result = std::find( texture_types_begin, texture_types_end, texture_type );
                assert( result != texture_types_end );
                auto index = result - texture_types_begin;

                *array_iterator = shader_resource_views[index];
                ++array_iterator;
            }

            unsigned count = static_cast<unsigned>( array_iterator - shader_resource_view_array.begin() );
            SetShaderResources<ShaderType>( device_context, 0, count, shader_resource_view_array.data() );
        }
    };
}

namespace Graphics
{
    InputLayoutID SetShaders( DisplayTechnique technique, ShaderContainer const & shader_container,  DeviceContext& device_context )
    {
        auto shader_setter = ShaderSetter( shader_container, device_context );

        // vertex shader
        {
            shader_setter.Set( technique.vertex_shader_id );
        }

        // hull shader


        // domain shader
        // ...

        // geometry shader
        // ...

        // pixel shader
        {
            shader_setter.Set( technique.pixel_shader_id );
        }

        return shader_setter.GetInputLayoutID();
    }



    void SetConstantBuffers( Range<ConstantBufferTypeAndID const*> buffers,  DisplayTechnique technique,  ConstantBufferContainer const & buffer_container, ShaderContainer const & shader_container, DeviceContext& device_context )
    {
        auto constant_buffer_setter = ConstantBufferSetter( shader_container, buffer_container, device_context, buffers );

        // vertex shader
        {
            constant_buffer_setter.Set( technique.vertex_shader_id );
        }

        // hull shader


        // domain shader
        // ...

        // geometry shader
        // ...

        // pixel shader
        {
            constant_buffer_setter.Set( technique.pixel_shader_id );
        }
    }



    void SetTextures( DeviceContext& device_context, DisplayTechnique technique, ShaderContainer& shader_container, ID3D11ShaderResourceView* const * shader_resource_views, TextureType const * shader_view_texture_types, unsigned type_and_resource_view_count )
    {
        auto texture_setter = TextureSetter( device_context, shader_resource_views, shader_view_texture_types, type_and_resource_view_count );

        auto texture_types = &shader_container.GetTextureTypes( technique.vertex_shader_id );
        texture_setter.SetTextures<ID3D11VertexShader>( *texture_types );

        texture_types = &shader_container.GetTextureTypes( technique.pixel_shader_id );
        texture_setter.SetTextures<ID3D11PixelShader>( *texture_types );

    }


    void UnSetTextures( ID3D11DeviceContext* device_context )
    {
        ID3D11ShaderResourceView* null = nullptr;
        SetShaderResources<ID3D11VertexShader>( device_context, 0, 1, &null );
        SetShaderResources<ID3D11DomainShader>( device_context, 0, 1, &null );
        SetShaderResources<ID3D11HullShader>( device_context, 0, 1, &null );
        SetShaderResources<ID3D11GeometryShader>( device_context, 0, 1, &null );
        SetShaderResources<ID3D11PixelShader>( device_context, 0, 1, &null );
    }
}