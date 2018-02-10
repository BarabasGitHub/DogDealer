#include "RecoverShaderInfo.h"

#include "ConstantBufferTypes.h"
#include "TextureType.h"
#include "ShaderType.h"
#include "ShaderKeyWords.h"

#include <Utilities\IntegerRange.h>
#include <Utilities\DogDealerException.h>

#include "DirectX/Direct3D11.h"
#include <d3d11shader.h>
#include <d3dcompiler.inl>

#include <string>
#include <cassert>


using namespace Graphics;

namespace
{
    ConstantBufferType ConvertToConstantBufferType( std::string name )
    {
        using namespace KeyWord;
        if( name == bones ) return ConstantBufferType::Bones;
        else if( name == camera ) return ConstantBufferType::Camera;
        else if( name == object ) return ConstantBufferType::Object;
        else if( name == projection ) return ConstantBufferType::Projection;
        else if( name == sky ) return ConstantBufferType::Sky;
        else if( name == light ) return ConstantBufferType::Light;
        else if( name == singlelighttransform ) return ConstantBufferType::SingleLightTransform;
        else if( name == terrain2d_constant ) return ConstantBufferType::Terrain2D;
        else if( name == terrain2d_patch ) return ConstantBufferType::Terrain2DPatch;
        else throw DogDealerException(("Unknown ConstantBuffer name: " + name).c_str(), true);
    }

    ShaderType VersionToType( unsigned version )
    {
        switch( D3D11_SHVER_GET_TYPE( version ) )
        {
        case D3D11_SHVER_PIXEL_SHADER:
            return ShaderType::Pixel;
        case D3D11_SHVER_VERTEX_SHADER:
            return ShaderType::Vertex;
        case D3D11_SHVER_GEOMETRY_SHADER:
            return ShaderType::Geometry;
        case D3D11_SHVER_HULL_SHADER:
            return ShaderType::Hull;
        case D3D11_SHVER_DOMAIN_SHADER:
            return ShaderType::Domain;
        case D3D11_SHVER_COMPUTE_SHADER:
            return ShaderType::Compute;
        default:
            return ShaderType::None;
        }
    }
}

namespace Graphics
{

    ShaderType RecoverShaderInfo( const DataBlob& shader_byte_code, std::vector<ConstantBufferType> & constant_buffer_types, std::vector<TextureType> & texture_types, std::vector<bool> & texture_is_array, std::vector<std::string> & sampler_names, std::vector<bool> & sampler_for_comparison )
    {

        ComPtr<ID3D11ShaderReflection> reflector;
        ThrowIfFailed( D3D11Reflect( shader_byte_code.data(), shader_byte_code.size(), reflector.GetAddressOf() ) );

        D3D11_SHADER_DESC shader_description;

        ThrowIfFailed( reflector->GetDesc( &shader_description ) );

        for( auto index : CreateIntegerRange( shader_description.BoundResources ) )
        {
            D3D11_SHADER_INPUT_BIND_DESC resource_description;
            ThrowIfFailed( reflector->GetResourceBindingDesc( index, &resource_description ) );

            switch( resource_description.Type )
            {
            case D3D_SIT_CBUFFER:
                ;
                {
                    auto type = ConvertToConstantBufferType( resource_description.Name );
                    assert( constant_buffer_types.size() == resource_description.BindPoint );
                    constant_buffer_types.emplace_back( type );
                    break;
                }
            case D3D_SIT_TBUFFER: // dunno how these work
                break;
            case D3D_SIT_TEXTURE:
                ;
                {
                    // we only support simple 2d textures for now
                    if( resource_description.Dimension != D3D_SRV_DIMENSION_TEXTURE2D && resource_description.Dimension != D3D_SRV_DIMENSION_TEXTURE2DARRAY )
                    {
                        throw DogDealerException( "Only 2D textures are supported currently.", false );
                    }
                    auto type = ConvertToTextureType( resource_description.Name );
                    assert( texture_types.size() == resource_description.BindPoint );
                    texture_types.emplace_back( type );
                    texture_is_array.push_back( resource_description.Dimension == D3D_SRV_DIMENSION_TEXTURE2DARRAY );
                    break;
                }
            case D3D_SIT_SAMPLER:
                ;
                {
                    assert( sampler_names.size() == resource_description.BindPoint );
                    sampler_names.emplace_back( resource_description.Name );
                    sampler_for_comparison.push_back( (resource_description.uFlags & D3D_SIF_COMPARISON_SAMPLER) == D3D_SIF_COMPARISON_SAMPLER );
                    break;
                }
            case D3D_SIT_UAV_RWTYPED:
                break;
            case D3D_SIT_STRUCTURED:
                break;
            case D3D_SIT_UAV_RWSTRUCTURED:
                break;
            case D3D_SIT_BYTEADDRESS:
                break;
            case D3D_SIT_UAV_RWBYTEADDRESS:
                break;
            case D3D_SIT_UAV_APPEND_STRUCTURED:
                break;
            case D3D_SIT_UAV_CONSUME_STRUCTURED:
                break;
            case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
                break;
            default:
                break;
            }
        }

        auto type = VersionToType( shader_description.Version );
        assert( type != ShaderType::None );
        return type;
    }
}