#pragma once

#include "DirectX\Direct3D11.h"

#include <d3d11shader.h>

#include <vector>

namespace Graphics
{
    struct InputElementDescriptions
    {
        std::vector<D3D11_INPUT_ELEMENT_DESC> m_d3d_input_element_descriptions;
        std::vector<std::string> m_semantic_names_of_input_elements; // we actually have to store these, because the creation of the InputLayoutInfo will fail if they don't match.

        InputElementDescriptions() = default;
        InputElementDescriptions( InputElementDescriptions const & other );
        InputElementDescriptions( InputElementDescriptions && other ) = default;
        InputElementDescriptions& operator=(const InputElementDescriptions&) = default;
        InputElementDescriptions& operator=(InputElementDescriptions&&) = default;

        void Fill( unsigned const input_parameter_index, D3D11_SIGNATURE_PARAMETER_DESC const parameter_description, const unsigned input_slot );
    };

    inline InputElementDescriptions::InputElementDescriptions( InputElementDescriptions const & other ) :
        m_d3d_input_element_descriptions( other.m_d3d_input_element_descriptions ),
        m_semantic_names_of_input_elements( other.m_semantic_names_of_input_elements )
    {
        for( auto index = 0u; index < m_d3d_input_element_descriptions.size(); ++index )
        {
            m_d3d_input_element_descriptions[index].SemanticName = m_semantic_names_of_input_elements[index].c_str();
        }
    }
}