#include "ProcessShaderInputParamters.h"

#include "InputElementDescriptions.h"
#include "VertexBufferType.h"
#include "ShaderKeyWords.h"

#include <Utilities\Datablob.h>
#include <Utilities\IntegerRange.h>
#include <Utilities\DogDealerException.h>

#include "DirectX\Direct3D11.h"
#include <d3d11shader.h>
#include <d3dcompiler.h>

#include <map>
#include <string>


using namespace Graphics;

namespace
{
    inline std::pair<VertexBufferType, bool> ConvertToVertexBufferType( std::string semantic_name )
    {
        using namespace KeyWord;
        auto instanced = false;
        if( instance.compare( 0, instance.size(), semantic_name, 0, instance.size() ) == 0 )
        {
            semantic_name = semantic_name.substr( instance.size() );
            instanced = true;
        }

        std::map<std::string, VertexBufferType const> const lookup = {
                { position, VertexBufferType::Position },
                { normal, VertexBufferType::Normal },
                { tangent, VertexBufferType::Tangent },
                { bitangent, VertexBufferType::Tangent },
                { handedness, VertexBufferType::Tangent },
                { texture, VertexBufferType::Texture },
                { color, VertexBufferType::Color },
                { blendindices, VertexBufferType::Bone },
                { blendweights, VertexBufferType::Bone },
                { worldmatrix, VertexBufferType::WorldMatrix },
        };

        auto result = lookup.find( semantic_name );
        auto type = result == lookup.end() ? VertexBufferType::Undefined : result->second;

        return{ type, instanced };

        //if( semantic_name == position ) return std::make_pair( VertexBufferType::Position, instanced );
        //else if( semantic_name == normal ) return std::make_pair( VertexBufferType::Normal, instanced );
        //else if( semantic_name == tangent ) return std::make_pair( VertexBufferType::Tangent, instanced );
        //else if( semantic_name == bitangent ) return std::make_pair( VertexBufferType::Tangent, instanced );
        //else if( semantic_name == handedness ) return std::make_pair( VertexBufferType::Tangent, instanced );
        //else if( semantic_name == texture ) return std::make_pair( VertexBufferType::Texture, instanced );
        //else if( semantic_name == color ) return std::make_pair( VertexBufferType::Color, instanced );
        //else if( semantic_name == blendindices ) return std::make_pair( VertexBufferType::Bone, instanced );
        //else if( semantic_name == blendweights ) return std::make_pair( VertexBufferType::Bone, instanced );
        //else if( semantic_name == worldmatrix ) return std::make_pair( VertexBufferType::WorldMatrix, instanced );
        //else return std::make_pair( VertexBufferType::Undefined, instanced );
    }
}

namespace Graphics
{


    void ProcessShaderInputParamters( DataBlob const & shader_byte_code, std::vector<VertexBufferType> & vertex_buffer_types, std::vector<VertexBufferType> & instance_buffer_types, InputElementDescriptions & input_element_descriptions )
    {
        ComPtr<ID3D11ShaderReflection> reflector;
        ThrowIfFailed(D3DReflect(shader_byte_code.data(), shader_byte_code.size(), IID_ID3D12ShaderReflection, (void**)reflector.GetAddressOf()));

        D3D11_SHADER_DESC shader_description;

        ThrowIfFailed( reflector->GetDesc( &shader_description ) );

        input_element_descriptions.m_d3d_input_element_descriptions.resize( shader_description.InputParameters );
        input_element_descriptions.m_semantic_names_of_input_elements.resize( shader_description.InputParameters );

        // using system generated here because we will always add these
        auto previous_buffer_type = std::make_pair( VertexBufferType::SystemGenerated, false );

        for( auto input_parameter_index : CreateIntegerRange( shader_description.InputParameters ) )
        {
            D3D11_SIGNATURE_PARAMETER_DESC parameter_description;
            ThrowIfFailed( reflector->GetInputParameterDesc( input_parameter_index, &parameter_description ) );

            switch( parameter_description.SystemValueType )
            {
            case D3D_NAME_POSITION:
                ;
                {
                    vertex_buffer_types.emplace_back( VertexBufferType::Position );
                    previous_buffer_type = std::make_pair( VertexBufferType::Position, false );
                    break;
                }
            case D3D_NAME_VERTEX_ID:
            case D3D_NAME_PRIMITIVE_ID:
                ;
                {
                    vertex_buffer_types.emplace_back( VertexBufferType::SystemGenerated );
                    previous_buffer_type = std::make_pair( VertexBufferType::SystemGenerated, false );
                    break;
                }
            case D3D_NAME_INSTANCE_ID:
                ;
                {
                    instance_buffer_types.emplace_back( VertexBufferType::SystemGenerated );
                    previous_buffer_type = std::make_pair( VertexBufferType::SystemGenerated, true );
                    break;
                }
            case D3D_NAME_UNDEFINED:  // detect on the semantic name
                ;
                {
                    if( parameter_description.SemanticIndex == 0 ) // assuming multiples of the same thing are in the same buffertype and don't need any further processing
                    {
                        auto buffer_type = ConvertToVertexBufferType( parameter_description.SemanticName );
                        if( buffer_type != previous_buffer_type ) // Does not belong to the same buffer. We could check if the user put it in the right order, but for now I just assume they do.
                        {
                            if( buffer_type.second )
                            {
                                instance_buffer_types.emplace_back( buffer_type.first );
                            }
                            else
                            {
                                vertex_buffer_types.emplace_back( buffer_type.first );
                            }
                            previous_buffer_type = buffer_type;
                        }
                    }
                    break;
                }
            default:
                ;
                {
                    // this shouldn't happen, but if it does we don't know what to do with it ^^
                    // for now throw an exception.
                    throw DogDealerException( "Unknown shader vertex input.", false );
                }
            }

            // fill the d3d input element descriptions
            input_element_descriptions.Fill( input_parameter_index, parameter_description, static_cast<unsigned>( vertex_buffer_types.size() + instance_buffer_types.size() - 1 ) );
        }
    }
}