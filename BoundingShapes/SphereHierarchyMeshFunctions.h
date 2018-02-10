#include "SphereHierarchyMesh.h"

#include <Conventions\Orientation.h>

#include <Math\FloatTypes.h>

#include <vector>

namespace BoundingShapes
{
    SphereHierarchyMesh CreateSphereHierarchyMesh( std::vector<Math::Float3> vertex_positions, std::vector<unsigned> indices );

    SphereHierarchyMesh Transform( SphereHierarchyMesh mesh, Math::Float4x4 const & transform );
    SphereHierarchyMesh TransformByOrientation( SphereHierarchyMesh mesh, Orientation const & orientation );
}