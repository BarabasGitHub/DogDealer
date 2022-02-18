#pragma once

#include "NavigationMeshContainer.h"

#include <map>
#include <string>

namespace Logic{

    struct NavigationMeshContainer;

	class ResourceManager{

	public:

        NavigationMeshID ProvideNavigationMesh(std::string const & navigation_mesh_name, NavigationMeshContainer& navmesh_container);
    
    private:
		std::map<std::string, NavigationMeshID>	m_navmesh_dictionary;
	};
}
