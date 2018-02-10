#include "ResourceManager.h"

#include "NavigationMeshContainer.h"

#include <Utilities\IndexedHelp.h>

#include <fstream>


using namespace std;

namespace {
	string FilePathFromNavigationMeshName(string const & mesh_name)
	{
		auto file_path = "Resources\\" + mesh_name + ".mesh";
		return file_path;
	}
}

namespace Logic{
	
    NavigationMeshID ResourceManager::ProvideNavigationMesh(std::string const & mesh_name, NavigationMeshContainer & navmesh_container)
    {

		auto result = m_navmesh_dictionary.find(mesh_name);
		// check if already in the dictionary
		if (result != m_navmesh_dictionary.end())
		{
			return result->second;
		}

		// Otherwise load from file
		auto navmesh_file_path = FilePathFromNavigationMeshName(mesh_name);
		ifstream data_stream(navmesh_file_path, std::ios::binary);
		assert(data_stream.good());

		auto navmesh = navmesh_container.LoadNavigationMesh(data_stream);
		data_stream.close();

		// add or overwrite if the ids were invalid.
		m_navmesh_dictionary[mesh_name] = navmesh;
		return navmesh;
	}
}
