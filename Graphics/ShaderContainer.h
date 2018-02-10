#pragma once
#include "IDs.h"
#include "DirectX\Direct3D11.h"
#include "ConstantBufferTypes.h"
#include "TextureType.h"
#include "InputElementDescriptions.h"

#include <Utilities\StreamHelpers.h>
#include <Utilities\Datablob.h>
#include <Utilities\Handle.h>

#include <vector>


namespace Graphics
{

    // forward declarations

    struct Device;
    enum struct VertexBufferType : uint8_t;
    enum struct TextureFiltering;
    struct InputElementDescriptions;


    UniformHandle const c_invalid_shader_id = { std::numeric_limits<UniformHandle::index_t>::max(), 0 };
    InputLayoutID const c_invalid_layout_id = { std::numeric_limits<InputLayoutID::index_t>::max( ), 0 };

    class ShaderContainer
    {

    public:

        typedef ComPtr<ID3D11VertexShader>		VertexShaderPtr;
        typedef ComPtr<ID3D11HullShader>		HullShaderPtr;
        typedef ComPtr<ID3D11DomainShader>		DomainShaderPtr;
        typedef ComPtr<ID3D11GeometryShader>	GeometryShaderPtr;
        typedef ComPtr<ID3D11PixelShader>		PixelShaderPtr;
        typedef ComPtr<ID3D11ComputeShader>		ComputeShaderPtr;

        typedef ComPtr<ID3D11SamplerState>		SamplerStatePtr;
        typedef ComPtr<ID3D11InputLayout>		InputLayoutPtr;

        struct Properties
        {
            bool animated;
        };

        struct CommonElements
        {
            InputLayoutID input_layout_id;
            std::vector<SamplerStatePtr> sampler_states;
            std::vector<ConstantBufferType> constant_buffer_types;
        };

        template<typename ShaderType>
        struct ShaderData
        {
            std::vector<ComPtr<ShaderType>> shaders;
            std::vector<CommonElements> elements;
            std::vector<Properties> properties;
            std::vector<unsigned> texutre_indices;
        };

    private:
        ShaderData<ID3D11VertexShader> m_vertex;
        ShaderData<ID3D11PixelShader> m_pixel;
        // and more for the other shaders

        std::vector<std::vector<TextureType>> m_texture_types;
        std::vector<std::vector<bool>> m_texture_is_array;

        std::vector<InputLayoutPtr>	 m_input_layouts;
		std::vector<InputElementDescriptions> m_input_element_descriptions;
        std::vector<std::vector<VertexBufferType>> m_vertex_buffer_types;
        std::vector<std::vector<VertexBufferType>> m_instance_buffer_types;

    public:

        ShaderContainer();
        ~ShaderContainer();

        UniformHandle CreateShader( TextureFiltering filtering, Device& device, std::istream& data_stream );

        bool IsValid( VertexShaderID id ) const { return m_vertex.shaders.size() > id.index; }
        bool IsValid( PixelShaderID id ) const { return m_pixel.shaders.size() > id.index; }

        ID3D11VertexShader* GetShader( VertexShaderID id ) const { return m_vertex.shaders[id.index]; }
        ID3D11PixelShader* GetShader( PixelShaderID id ) const { return m_pixel.shaders[id.index]; }

        CommonElements const & GetElements( VertexShaderID id ) const { return m_vertex.elements[id.index]; }
        CommonElements const & GetElements( PixelShaderID id ) const { return m_pixel.elements[id.index]; }

        Properties const & GetProperties( VertexShaderID id ) const { return m_vertex.properties[id.index]; }
        Properties const & GetProperties( PixelShaderID id ) const { return m_pixel.properties[id.index]; }

        ID3D11InputLayout* GetInputLayout( InputLayoutID id ) const { return m_input_layouts[id.index]; };
		InputElementDescriptions const &  GetInputElementDescription(InputLayoutID id ) const { return m_input_element_descriptions[id.index]; };
        std::vector<VertexBufferType> const & GetVertexBufferTypes( InputLayoutID id ) const { return m_vertex_buffer_types[id.index]; }
        std::vector<VertexBufferType> const & GetInstanceBufferTypes( InputLayoutID id ) const { return m_instance_buffer_types[id.index]; }

        std::vector<TextureType> const& GetTextureTypes( VertexShaderID id ) const { return m_texture_types[m_vertex.texutre_indices[id.index]]; }
        std::vector<TextureType> const & GetTextureTypes( PixelShaderID id ) const { return m_texture_types[m_pixel.texutre_indices[id.index]]; }

        void Remove( VertexShaderID id );
        void Remove( PixelShaderID id );

    private:
        VertexShaderID CreateVertexShader( Device& device, DataBlob const & shader_blob, CommonElements elements, Properties, unsigned textures_index );
        PixelShaderID CreatePixelShader( Device& device, DataBlob const & shader_blob, CommonElements elements, Properties, unsigned textures_index );

        InputLayoutID ProvideInputLayout( DataBlob& shader_blob, InputElementDescriptions input_element_descriptions, Device& device );

        void Remove( InputLayoutID id );
    };
}