#pragma once
#include "ResourceManager.h"

#include "Device.h"
#include "ShaderContainer.h"
#include "ResourceViewContainer.h"
#include "VertexBufferContainer.h"
#include "IndexBufferContainer.h"
#include "TextureContainer.h"
#include "MeshFunctions.h"

#include "DebugBox.h"

#include <Utilities\FileFinder.h>
#include <Utilities\DogDealerException.h>
#include <Utilities\StringUtilities.h>

#include <assert.h>
#include <functional>
#include <fstream>

using namespace Graphics;
using namespace std;


namespace{

    string GetShaderFolderString()
    {
        auto executable_path = GetExecutableFilePathString( );
        auto folder = DiscardFileName( move( executable_path ) );
        folder += "Shaders\\";
        return folder;
    }

    wstring GetShaderFolderWString()
    {
        auto executable_path = GetExecutableFilePathWString( );
        auto folder = DiscardFileName( move( executable_path ) );
        folder += L"Shaders\\";
        return folder;
    }

    string GetTextureFolderString()
    {
        return "Resources\\Textures\\";
    }

    wstring GetTextureFolderWString( )
    {
        return L"Resources\\Textures\\";
    }

    string FilePathFromTextureName(string const & texture_name)
    {
        return GetTextureFolderString() + texture_name;// +".dds";
    }

	string ShaderNameFromFilePath(wstring file_name)
	{
        auto shader_name = DiscardPath( DiscardExtension( move( file_name ) ) );
		// convert wstring to string
		wstring_convert<codecvt_utf8_utf16<wchar_t>> conversion;

		return conversion.to_bytes(shader_name);
	}

	string ShaderNameFromFilePath(string file_name)
	{
        return DiscardPath( DiscardExtension( move(file_name) ) );
	}

	string FilePathFromShaderName(string const & shader_name)
	{
		auto file_path = GetShaderFolderString() + shader_name + ".cso";
		return file_path;
	}

	string FilePathFromMeshName(string const & mesh_name)
	{
		auto file_path = "Resources\\" + mesh_name + ".mesh";
		return file_path;
	}
}

void ResourceManager::ProvideTextureList(
    Range<std::pair<TextureType, std::string> const *> texture_names,
    TextureContainer& texture_container,
    ResourceViewContainer & resource_view_container,
    Device& device,
    Range<TextureTypeAndID*> texture_list)
{
	assert(Size(texture_list) == Size(texture_names));

	for( auto i = 0u; i < Size(texture_list); ++i )
    {
        auto const & texture_name = texture_names[i].second;
		auto texture_type = texture_names[i].first;

		auto found_texture_id = m_texture_dictionary.find(texture_name);
		// check if already in the dictionary and the texture is still loaded
		if (found_texture_id != m_texture_dictionary.end() && texture_container.IsValid( found_texture_id->second.first ) && resource_view_container.IsValid( found_texture_id->second.second ) )
		{
			// store pair of type and existing id
			texture_list[i] = {texture_type, found_texture_id->second.first, found_texture_id->second.second};
		}
		else
		{
			// Load texture in TextureContainer
            auto file_path = FilePathFromTextureName( texture_name );
			ifstream data_stream(file_path, std::ios::binary);
			if(!data_stream.good())
            {
                throw DogDealerException(("Failed to load texture: " + texture_name).c_str(), true);
            }

            // SCAMP
            auto force_srgb = texture_name.find( "normal" ) == string::npos;
			auto texture_id = texture_container.LoadTexture(device, data_stream, force_srgb);
            auto view = device.CreateShaderResourceView( texture_container.GetTexture2D( texture_id ) );
            auto view_id = resource_view_container.AddShaderResourceView(view);
            texture_list[i] = {texture_type, texture_id, view_id};

			// add or overwrite if the ids were invalid.
            m_texture_dictionary[texture_name] = { texture_id, view_id };
		}
	}
}

namespace
{
    bool AllValidOrUnusedIDs( Mesh const & mesh, IndexBufferContainer const & index_container, VertexBufferContainer const & vertex_container )
    {
        auto ok = mesh.index_id.index == c_invalid_index_buffer_id.index || index_container.IsValid( mesh.index_id );
        ok = ok && all_of( mesh.vertex_ids.begin(), mesh.vertex_ids.end(), [&vertex_container]( VertexBufferID id )
        {
            return id.index == c_invalid_vertex_buffer_id.index || vertex_container.IsValid( id );
        } );
        return ok;
    }
}

MeshData ResourceManager::ProvideMeshData( std::string const & mesh_name, IndexBufferContainer& index_container, VertexBufferContainer& vertex_container, Device& device )
{
    auto result = m_mesh_dictionary.find( mesh_name );
    // check if already in the dictionary
    if( result != m_mesh_dictionary.end() )
    {
        using namespace placeholders;
        auto & mesh_data = result->second;
        // check if all ids are still valid
        if( AllValidOrUnusedIDs(mesh_data.mesh, index_container, vertex_container ) )
        {
            return result->second;
        }
    }

	auto mesh_file_path = FilePathFromMeshName(mesh_name);
    ifstream data_stream( mesh_file_path, std::ios::binary );
    if( !data_stream.good() )
    {
        throw DogDealerException(("Failed to load mesh: " + mesh_name).c_str(), true);
    }

    auto mesh = LoadMesh( device, index_container, vertex_container, data_stream);

	// add or overwrite if the ids were invalid.
    m_mesh_dictionary[mesh_name] = mesh;
    return mesh;
}


MeshData ResourceManager::ProvideMeshDataFromVectors(IndexBufferContainer& index_container, VertexBufferContainer& vertex_container, Device& device, std::vector<PositionType> const & vertex_positions, std::vector<NormalType> const & vertex_normals, std::vector<unsigned> const & vertex_indices)
{
	return GenerateMesh(device, index_container, vertex_container, vertex_indices, vertex_positions, vertex_normals);
}


// Translates each of the Shader names of the description to one of the pre-loaded shader indices
DisplayTechnique ResourceManager::ProvideDisplayTechnique(DisplayTechniqueDescription const & technique_description)
{
    DisplayTechnique technique( c_invalid_shader_id );

    if( !technique_description.vertex_shader.empty() )
    {
        technique.vertex_shader_id = ToHandle<VertexShaderID>( GetShaderID( technique_description.vertex_shader ) );
    }

    if( !technique_description.pixel_shader.empty() )
    {
        technique.pixel_shader_id = ToHandle<PixelShaderID>( GetShaderID( technique_description.pixel_shader ) );
    }

	return technique;
}


// Iterate over all cso files, adding them to the queue
void ResourceManager::PreLoadShaderFiles(TextureFiltering filtering, ShaderContainer& container, Device& device)
{
    auto shader_path_prefix = GetShaderFolderWString();
    shader_path_prefix += L"*";

	auto files = FindFiles(shader_path_prefix);

    // this loop destroys the entries in files
	for(auto & file : files)
	{
		auto shader_name = ShaderNameFromFilePath(move(file.full_name));
		QueueShaderPreload(move(shader_name));
	}

	PreLoadQueuedShaders(filtering, container, device);
}

// Iterate over all queued shader names, calling the ShaderContainer for their construction
void ResourceManager::PreLoadQueuedShaders(TextureFiltering filtering, ShaderContainer& container, Device& device)
{
	for (auto const & item : m_shader_preload_list )
	{
		auto file_path = FilePathFromShaderName(item);
		LoadShader(file_path, filtering, container, device);
	}
    m_shader_preload_list.clear();
}


void ResourceManager::LoadShader(string filename, TextureFiltering filtering, ShaderContainer& container, Device& device)
{
	ifstream shader_stream(filename, ios::binary);
    if(!shader_stream.good())
    {
        throw DogDealerException(("Failed to load shader: " + filename).c_str(), true);
    }
	auto shader_id = container.CreateShader(filtering, device, shader_stream);
    shader_stream.close();

	auto shader_name = ShaderNameFromFilePath(move(filename));
	m_shader_dictionary.insert(make_pair(shader_name, shader_id));
}


// Return ID of shader in the dictionary, if it exists
UniformHandle ResourceManager::GetShaderID(string const & shader_name)
{
	auto result = m_shader_dictionary.find(shader_name);
	assert(result != m_shader_dictionary.end());

	return (*result).second;
}


void ResourceManager::QueueShaderPreload(string filename)
{
	m_shader_preload_list.emplace(move(filename));
}


void ResourceManager::CreateDebugResources(IndexBufferContainer& index_container, VertexBufferContainer& vertex_container, Device& device)
{
    std::istringstream data_stream;
    data_stream.str(std::string(reinterpret_cast<const char*>(Debug::c_wire_box_data), Debug::c_wire_box_data_size * sizeof(Debug::c_wire_box_data[0])));
    auto mesh_data = LoadMesh( device, index_container, vertex_container, data_stream );
    m_mesh_dictionary["debug_wire_box"] = mesh_data;
    std::ifstream debug_sphere(FilePathFromMeshName("debug_sphere"), std::ios::binary);
    m_mesh_dictionary["debug_sphere"] = LoadMesh(device, index_container, vertex_container, debug_sphere);
}
