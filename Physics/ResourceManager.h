#pragma once
#include "MeshContainer.h" // for the AxisAlignedBoxHierarchyMeshID

#include <BoundingShapes\FileLayout.h>
#include <BoundingShapes\AxisAlignedBox.h>
#include <BoundingShapes\AxisAlignedBoxHierarchyMesh.h>

#include <map>

namespace BoundingShapes
{
    struct Sphere;
    struct OrientedBox;
}

namespace Physics{

    struct NewCollisionData
    {
        BoundingShapes::AxisAlignedBox axis_aligned_box;
        // narrow shapes
        std::vector<BoundingShapes::OrientedBox> oriented_boxes;
        std::vector<BoundingShapes::Sphere> spheres;
        std::vector<BoundingShapes::AxisAlignedBoxHierarchyMesh> meshes;
    };

    struct StoredCollisionData
    {
        BoundingShapes::AxisAlignedBox axis_aligned_box;
        // narrow shapes
        std::vector<BoundingShapes::OrientedBox> oriented_boxes;
        std::vector<BoundingShapes::Sphere> spheres;
        std::vector<AxisAlignedBoxHierarchyMeshID> mesh_ids;
    };

	class ResourceManager
    {
	public:

        bool GetLoadedCollisionData(std::string const & file_name, StoredCollisionData & collision_data);
        void LoadCollisionData(std::string const & file_name, NewCollisionData & collision_data);
        void StoreCollisionData(std::string file_name, StoredCollisionData const & collision_data);

    private:

        std::map<std::string, StoredCollisionData> m_loaded_collision_data;
	};
}