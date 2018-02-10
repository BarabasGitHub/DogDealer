#include "AxisAlignedBoxHierarchyMesh.h"

#include <Conventions\Orientation.h>

#include <Math\FloatTypes.h>

#include <vector>

namespace BoundingShapes
{
    AxisAlignedBoxHierarchyMesh CreateAxisAlignedBoxHierarchyMesh( std::vector<Math::Float3> vertex_positions, std::vector<unsigned> indices );

    AxisAlignedBoxHierarchyMesh Transform( AxisAlignedBoxHierarchyMesh mesh, Math::Float4x4 const & transform );
    AxisAlignedBoxHierarchyMesh TransformByOrientation( AxisAlignedBoxHierarchyMesh mesh, Orientation const & orientation );
}