#pragma once
#include "ShaderContainer.h"
#include "RecoverShaderInfo.h"
#include "InputElementDescriptions.h"
#include "Device.h"
#include "ProcessShaderInputParamters.h"
#include "ShaderType.h"
#include "ConstantBufferTypes.h"
#include "TextureFiltering.h"

#include <Utilities\DataBlob.h>
#include <Utilities\DogDealerException.h>

#include <fstream>
#include <cassert>

using namespace Graphics;
using namespace std;

namespace
{

    DataBlob ReadShaderFile( std::istream& data_stream )
    {
        return  CreateDataBlobTillEndOfStream( data_stream );
    }


    D3D11_SAMPLER_DESC CreateShadowSamplerStateDescription(TextureFiltering texture_filtering)
    {
        D3D11_SAMPLER_DESC description;
        description.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
        description.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
        description.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        description.BorderColor[0] = 0;
        description.BorderColor[1] = 0;
        description.BorderColor[2] = 0;
        description.BorderColor[3] = 0;
        switch(texture_filtering)
        {
            case TextureFiltering::NearestNeighbour:
                description.MaxAnisotropy = 1;
                description.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
                break;
            default:
                description.MaxAnisotropy = 1;
                description.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
                break;
        }
        description.MinLOD = description.MaxLOD = 0;
        description.MipLODBias = 0;
        description.ComparisonFunc = D3D11_COMPARISON_LESS;

        return description;
    }

    // Iterate over sampler states as defined in the shader info, creating sampler_states
    std::vector<ComPtr<ID3D11SamplerState>> CreateSamplerStates( std::vector<bool> const & comparison, TextureFiltering filtering, Device& device )
    {
        vector<ComPtr<ID3D11SamplerState>> sampler_states;
        sampler_states.reserve( comparison.size( ) );

        // Call constructor providers depending on sampler name
        for( auto compare : comparison )
        {
            if( compare )
            {
                auto description = CreateShadowSamplerStateDescription( filtering );
                sampler_states.emplace_back( device.CreateSamplerState( description ) );
            }
            else
            {
                // use default sampler
                // sampler_states.emplace_back( device.CreateSamplerState( filtering, D3D11_TEXTURE_ADDRESS_CLAMP ) );
                sampler_states.emplace_back( device.CreateSamplerState( filtering ) );
            }
        }

        return sampler_states;
    }

}

ShaderContainer::ShaderContainer() {}
ShaderContainer::~ShaderContainer() {}

UniformHandle ShaderContainer::CreateShader( TextureFiltering filtering, Device& device, std::istream& data_stream )
{
    auto shader_blob = ReadShaderFile( data_stream );

    InputElementDescriptions input_element_descriptions;
    std::vector<VertexBufferType> vertex_buffer_types, instance_buffer_types;

    ProcessShaderInputParamters( shader_blob, vertex_buffer_types, instance_buffer_types, input_element_descriptions );

    CommonElements elements;

    elements.input_layout_id = ProvideInputLayout( shader_blob, move(input_element_descriptions), device );

    assert( elements.input_layout_id.index == m_vertex_buffer_types.size() );
    assert( elements.input_layout_id.index == m_instance_buffer_types.size( ) );

    m_vertex_buffer_types.emplace_back( move( vertex_buffer_types ) );
    m_instance_buffer_types.emplace_back( move( instance_buffer_types ) );

    // assuming there is one input layout for every shader
    std::vector<TextureType> texture_types;
    std::vector<std::string> sampler_names;
    std::vector<bool> sampler_for_comparison;
    std::vector<bool> texture_is_array;

    auto shader_type = RecoverShaderInfo( shader_blob, elements.constant_buffer_types, texture_types, texture_is_array, sampler_names, sampler_for_comparison );

    Properties properties;
    properties.animated = std::find( elements.constant_buffer_types.begin(), elements.constant_buffer_types.end(), ConstantBufferType::Bones ) != elements.constant_buffer_types.end();

    auto index = static_cast<unsigned>( m_texture_types.size() );
    m_texture_types.emplace_back( move( texture_types ) );
    m_texture_is_array.emplace_back( move( texture_is_array ) );

    // Create SamplerStates
    elements.sampler_states = CreateSamplerStates( sampler_for_comparison, filtering, device );


    switch( shader_type )
    {
    case( ShaderType::Vertex ) :
        return CreateVertexShader( device, shader_blob, elements, properties, index );
    case( ShaderType::Pixel ) :
        return CreatePixelShader( device, shader_blob, elements, properties, index );
    }

    // unknown shader type, not good!
    throw DogDealerException("Unknown shader type encountered.", false);
}


VertexShaderID ShaderContainer::CreateVertexShader( Device& device, DataBlob const & shader_blob, CommonElements elements, Properties properties, unsigned textures_index )
{
    auto index = static_cast<unsigned>( m_vertex.shaders.size() );

    // Create ComPtr
    auto vertex_shader = device.CreateVertexShader( shader_blob );

    m_vertex.shaders.emplace_back( move( vertex_shader ) );
    m_vertex.elements.emplace_back( move( elements ) );
    m_vertex.texutre_indices.emplace_back( textures_index );
    m_vertex.properties.emplace_back( properties );

    return{ index, 0 };
}


PixelShaderID ShaderContainer::CreatePixelShader( Device& device, DataBlob const & shader_blob, CommonElements elements, Properties properties, unsigned textures_index )
{
    auto index = static_cast<unsigned>( m_pixel.shaders.size() );

    // Create ComPtr
    auto shader = device.CreatePixelShader( shader_blob );

    m_pixel.shaders.emplace_back( move( shader ) );
    m_pixel.elements.emplace_back( move( elements ) );
    m_pixel.texutre_indices.emplace_back( textures_index );
    m_pixel.properties.emplace_back( properties );

    return{ index, 0 };
}


InputLayoutID ShaderContainer::ProvideInputLayout( DataBlob& shader_blob, InputElementDescriptions input_element_descriptions, Device& device )
{
    auto input_layout_index = static_cast<unsigned>( m_input_layouts.size() );

    if( input_element_descriptions.m_d3d_input_element_descriptions.size() > 0 )
    {
        auto input_layout = device.CreateInputLayout( input_element_descriptions, shader_blob );
        m_input_layouts.emplace_back( move( input_layout ) );
    }
    else
    {
        m_input_layouts.emplace_back();
    }


    m_input_element_descriptions.emplace_back( move( input_element_descriptions ) );

    return{ input_layout_index, 0 };
}


void ShaderContainer::Remove( VertexShaderID id )
{
    auto texture_index = m_vertex.texutre_indices[id.index];

    m_texture_types.erase( begin( m_texture_types ) + texture_index );

    auto input_layout_id = m_vertex.elements[id.index].input_layout_id;
    Remove( input_layout_id );

    m_vertex.shaders.erase( begin( m_vertex.shaders ) + id.index );
    m_vertex.elements.erase( begin( m_vertex.elements ) + id.index );
    m_vertex.properties.erase( begin( m_vertex.properties ) + id.index );
    m_vertex.texutre_indices.erase( begin( m_vertex.texutre_indices ) + id.index );
}


void ShaderContainer::Remove( PixelShaderID id )
{
    auto texture_index = m_pixel.texutre_indices[id.index];

    m_texture_types.erase( begin( m_texture_types ) + texture_index );

    auto input_layout_id = m_pixel.elements[id.index].input_layout_id;
    Remove( input_layout_id );


    m_pixel.shaders.erase( begin( m_pixel.shaders ) + id.index );
    m_pixel.elements.erase( begin( m_pixel.elements ) + id.index );
    m_pixel.properties.erase( begin( m_pixel.properties ) + id.index );
    m_pixel.texutre_indices.erase( begin( m_pixel.texutre_indices ) + id.index );
}


void ShaderContainer::Remove( InputLayoutID id )
{
    auto input_layout_index = id.index;

    m_input_layouts.erase( begin( m_input_layouts ) + input_layout_index );

    m_vertex_buffer_types.erase( begin( m_vertex_buffer_types ) + input_layout_index );
    m_instance_buffer_types.erase( begin( m_instance_buffer_types ) + input_layout_index );
    m_input_element_descriptions.erase( begin( m_input_element_descriptions ) + input_layout_index );
}