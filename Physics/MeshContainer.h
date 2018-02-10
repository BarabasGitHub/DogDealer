#pragma once
#include "BodyID.h"
#include <BoundingShapes\AxisAlignedBoxHierarchyMesh.h>
#include <Utilities\Range.h>

#include <deque>
#include <vector>

namespace Physics
{
    typedef Handle<BoundingShapes::AxisAlignedBoxHierarchyMesh> AxisAlignedBoxHierarchyMeshID;


    struct MeshContainer
    {
        std::vector<uint32_t> body_to_data;
        std::vector<AxisAlignedBoxHierarchyMeshID::generation_t> generations;

        std::deque<uint32_t> free_list;

        std::vector<BoundingShapes::AxisAlignedBoxHierarchyMesh> meshes;
        std::vector<uint32_t> usage_counts;
    };

    // add a new mesh for a body
    AxisAlignedBoxHierarchyMeshID AddMesh( BodyID body_id, BoundingShapes::AxisAlignedBoxHierarchyMesh mesh, MeshContainer & self );
    // add a new mesh without a body. It will not be marked as used, but will not be removed until explicitly asked for it via the ID.
    // It will also be normally removed if you add it with a body later.
    AxisAlignedBoxHierarchyMeshID AddMesh( BoundingShapes::AxisAlignedBoxHierarchyMesh mesh, MeshContainer & self );
    // add a mesh usage for a body
    void AddMesh( BodyID body_id, AxisAlignedBoxHierarchyMeshID mesh_id, MeshContainer & self );
    // remove a mesh
    void Remove( AxisAlignedBoxHierarchyMeshID id, MeshContainer & self );
    // remove the meshes belonging to the bodies if they're no longer used
    void Remove( Range<BodyID const *> body_ids, MeshContainer & self );
    // remove the mesh belonging to a body if it is no longer used
    void Remove( BodyID body_id, MeshContainer & self );
    // check if this id is still in the container
    bool Contains( AxisAlignedBoxHierarchyMeshID id, MeshContainer const & self );
    // check if the container has an entry for this body
    bool Contains( BodyID id, MeshContainer const & self );
}
