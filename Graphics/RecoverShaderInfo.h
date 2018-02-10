#pragma once

#include <Utilities\Datablob.h>

#include <vector>
#include <string>

// forward declarations

namespace Graphics
{
    enum struct ShaderType;
    enum struct ConstantBufferType;
    enum struct TextureType;

    ShaderType RecoverShaderInfo( const DataBlob& shader_byte_code, std::vector<ConstantBufferType> & constant_buffer_types, std::vector<TextureType> & texture_types, std::vector<bool> & texture_is_array, std::vector<std::string> & sampler_names, std::vector<bool> & sampler_for_comparison );
}