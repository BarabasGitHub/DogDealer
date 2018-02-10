#include "MeshContainer.h"

#include <Utilities\ContainerHelpers.h>
#include <Utilities\IndexedHelp.h>
#include <Utilities\VectorHelper.h>

using namespace Physics;

AxisAlignedBoxHierarchyMeshID Physics::AddMesh( BodyID body_id, BoundingShapes::AxisAlignedBoxHierarchyMesh mesh, MeshContainer & self )
{
    AxisAlignedBoxHierarchyMeshID id;
    Add( id, std::move( mesh ), self.meshes, self.generations, self.free_list );

    // add an usage count of 1 for the mesh and fill any extra elements with zero (it is actually never needed to fill anything)
    AddIndexToIndices(self.usage_counts, id.index, 1, 0);

    AddIndexToIndices(self.body_to_data, body_id.index, id.index);

    return id;
}


AxisAlignedBoxHierarchyMeshID Physics::AddMesh( BoundingShapes::AxisAlignedBoxHierarchyMesh mesh, MeshContainer & self )
{
    AxisAlignedBoxHierarchyMeshID id;
    Add( id, std::move( mesh ), self.meshes, self.generations, self.free_list );

    // add an usage count of 0 for the mesh and fill any extra elements with zero (it is actually never needed to fill anything)
    AddIndexToIndices(self.usage_counts, id.index, 0, 0);

    return id;
}


void Physics::AddMesh( BodyID body_id, AxisAlignedBoxHierarchyMeshID mesh_id, MeshContainer & self )
{
    assert(Contains(mesh_id, self));

    self.usage_counts[mesh_id.index] += 1;

    AddIndexToIndices( self.body_to_data, body_id.index, mesh_id.index );
}


namespace
{
    void UncheckedRemove(AxisAlignedBoxHierarchyMeshID::index_t mesh_index , MeshContainer & self)
    {
        ::Remove( mesh_index, self.meshes, self.body_to_data, self.generations, self.free_list );
        self.usage_counts[mesh_index] = 0;
    }
}


void Physics::Remove( Range<BodyID const *> body_ids, MeshContainer & self )
{
    for( auto i = 0u; i < Size(body_ids); ++i )
    {
        Remove(body_ids[i], self);
    }
}


void Physics::Remove( BodyID body_id, MeshContainer & self )
{
    // check if it's the last used
    auto index = GetOptional(self.body_to_data, body_id.index);
    if( index != c_invalid_index )
    {
        assert(self.usage_counts[index] > 0);
        if( (self.usage_counts[index] -= 1) == 0 )
        {
            UncheckedRemove(index, self);
        }
    }
}


void Physics::Remove( AxisAlignedBoxHierarchyMeshID id, MeshContainer & self )
{
    if( !Contains( id, self ) ) return;
    UncheckedRemove(id.index, self);
}


bool Physics::Contains( AxisAlignedBoxHierarchyMeshID id, MeshContainer const & self )
{
    return id.index < self.generations.size() && self.generations[id.index] == id.generation;
}


bool Physics::Contains( BodyID id, MeshContainer const & self )
{
    return GetOptional(self.body_to_data, id.index) != c_invalid_index;
}
