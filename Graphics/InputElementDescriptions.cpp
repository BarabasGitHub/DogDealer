#include "InputElementDescriptions.h"

#include "ShaderKeyWords.h"

using namespace Graphics;

namespace
{
    inline DXGI_FORMAT ConvertToDXGIFormat( const D3D_REGISTER_COMPONENT_TYPE compoment_type, BYTE mask )
    {
        const BYTE mask0001 = 1;
        const BYTE mask0010 = 2;
        const BYTE mask0100 = 4;
        const BYTE mask1000 = 8;
        const BYTE mask0011 = 3;
        const BYTE mask0110 = 6;
        const BYTE mask1100 = 9;
        const BYTE mask0111 = 7;
        const BYTE mask1110 = 12;
        const BYTE mask1111 = 15;

        switch( mask )
        {
        case mask0001:
        case mask0010:
        case mask0100:
        case mask1000:
        {
            switch( compoment_type )
            {
            case D3D_REGISTER_COMPONENT_UINT32:  return DXGI_FORMAT_R32_UINT;
            case D3D_REGISTER_COMPONENT_SINT32:  return DXGI_FORMAT_R32_SINT;
            case D3D_REGISTER_COMPONENT_FLOAT32: return DXGI_FORMAT_R32_FLOAT;
            default: break;
            }
        }
        case mask0011:
        case mask0110:
        case mask1100:
        {
            switch( compoment_type )
            {
            case D3D_REGISTER_COMPONENT_UINT32:  return DXGI_FORMAT_R32G32_UINT;
            case D3D_REGISTER_COMPONENT_SINT32:  return DXGI_FORMAT_R32G32_SINT;
            case D3D_REGISTER_COMPONENT_FLOAT32: return DXGI_FORMAT_R32G32_FLOAT;
            default: break;
            }
        }
        case mask0111:
        case mask1110:
        {
            switch( compoment_type )
            {
            case D3D_REGISTER_COMPONENT_UINT32:  return DXGI_FORMAT_R32G32B32_UINT;
            case D3D_REGISTER_COMPONENT_SINT32:  return DXGI_FORMAT_R32G32B32_SINT;
            case D3D_REGISTER_COMPONENT_FLOAT32: return DXGI_FORMAT_R32G32B32_FLOAT;
            default: break;
            }
        }
        case mask1111:
        {
            switch( compoment_type )
            {
            case D3D_REGISTER_COMPONENT_UINT32:  return DXGI_FORMAT_R32G32B32A32_UINT;
            case D3D_REGISTER_COMPONENT_SINT32:  return DXGI_FORMAT_R32G32B32A32_SINT;
            case D3D_REGISTER_COMPONENT_FLOAT32: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            default: break;
            }
        }
        default: break;
        }
        return DXGI_FORMAT_UNKNOWN;
    }
}

void InputElementDescriptions::Fill( unsigned const input_parameter_index, D3D11_SIGNATURE_PARAMETER_DESC const parameter_description, const unsigned input_slot )
{
    m_semantic_names_of_input_elements[input_parameter_index] = parameter_description.SemanticName;
    m_d3d_input_element_descriptions[input_parameter_index].SemanticName = m_semantic_names_of_input_elements[input_parameter_index].c_str();
    m_d3d_input_element_descriptions[input_parameter_index].SemanticIndex = parameter_description.SemanticIndex;
    m_d3d_input_element_descriptions[input_parameter_index].Format = ConvertToDXGIFormat( parameter_description.ComponentType, parameter_description.Mask );
    m_d3d_input_element_descriptions[input_parameter_index].InputSlot = input_slot;
    m_d3d_input_element_descriptions[input_parameter_index].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    if( KeyWord::instance.compare( 0, KeyWord::instance.size(), parameter_description.SemanticName, 0, KeyWord::instance.size() ) == 0 )
    {
        m_d3d_input_element_descriptions[input_parameter_index].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA; // we can always update it later.
        m_d3d_input_element_descriptions[input_parameter_index].InstanceDataStepRate = 1; // 0 for vertexdata otherwise ehm... it will probably be 1.
    }
    else
    {
        m_d3d_input_element_descriptions[input_parameter_index].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA; // we can always update it later.
        m_d3d_input_element_descriptions[input_parameter_index].InstanceDataStepRate = 0; // 0 for vertexdata otherwise ehm... it will probably be 1.
    }
}