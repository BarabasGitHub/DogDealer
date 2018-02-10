#pragma once

#include <Utilities\Datablob.h> // typedef

#include <vector>

// forward declarations

namespace Graphics
{
    enum struct VertexBufferType : uint8_t;
    struct InputElementDescriptions;

    void ProcessShaderInputParamters( DataBlob const & shader_byte_code, std::vector<VertexBufferType> & vertex_buffer_types, std::vector<VertexBufferType> & instance_buffer_types, InputElementDescriptions & input_element_descriptions );
}