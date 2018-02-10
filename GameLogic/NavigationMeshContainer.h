#pragma once
#include "Structures.h"

namespace Logic
{
    struct NavigationMeshContainer
    {
        // Read a .mesh file and store its content as a NavigationMesh
        NavigationMeshID LoadNavigationMesh(std::istream& data_stream);

        std::vector<NavigationMesh> m_navigation_meshes;
    };
}
