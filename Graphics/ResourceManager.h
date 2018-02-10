#pragma once
#include "ResourceDescriptions.h"
#include "Structures.h"

#include <FileLayout\VertexDataType.h>
#include <Utilities\Handle.h>
#include <Utilities\Range.h>

#include <string>
#include <map>
#include <set>

namespace Graphics{

	struct Device;
	class ShaderContainer;
    struct IndexBufferContainer;
    struct VertexBufferContainer;
	struct TextureContainer;
    struct ResourceViewContainer;
    enum struct TextureFiltering;

	class ResourceManager{

	public:

        DisplayTechnique ProvideDisplayTechnique( DisplayTechniqueDescription const & technique_description );

		MeshData ProvideMeshData( std::string const & mesh_name, IndexBufferContainer& index_container, VertexBufferContainer& vertex_container, Device& device );

		static MeshData ProvideMeshDataFromVectors(IndexBufferContainer& index_container, VertexBufferContainer& vertex_container, Device& device, std::vector<PositionType> const & vertex_positions, std::vector<NormalType> const & vertex_normals, std::vector<unsigned> const & vertex_indices);

        void ProvideTextureList(
            Range<std::pair<TextureType, std::string> const *> texture_names,
            TextureContainer& texture_container,
            ResourceViewContainer & resource_view_container,
            Device& device,
            Range<TextureTypeAndID*> textures);

		UniformHandle GetShaderID(std::string const & shader_name);

		void PreLoadShaderFiles(TextureFiltering filtering, ShaderContainer& container, Device& device);
		void PreLoadQueuedShaders(TextureFiltering filtering, ShaderContainer& container, Device& device);

		void QueueShaderPreload(std::string filename);

        void CreateDebugResources(IndexBufferContainer& index_container, VertexBufferContainer& vertex_container, Device& device);

	private:
        typedef std::vector<TextureTypeAndID> TextureList;

		void LoadShader(std::string filename, TextureFiltering filtering, ShaderContainer& container, Device& device);

		std::set<std::string> m_shader_preload_list;

		std::map<std::string, UniformHandle> m_shader_dictionary;
		std::map<std::string, MeshData> m_mesh_dictionary;
		std::map<std::string, std::pair<Texture2DID, ShaderRersourceViewID>> m_texture_dictionary;
	};
}