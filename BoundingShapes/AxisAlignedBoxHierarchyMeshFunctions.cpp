#include "AxisAlignedBoxHierarchyMeshFunctions.h"

#include "BoundingShapeHierarchyMeshFunctions.h"

#include "AxisAlignedBoxFunctions.h"

#include <Math\MathFunctions.h>
#include <Math\FloatOperators.h>

using namespace Math;

namespace BoundingShapes
{
    namespace
    {
        Math::Float3 FindAxisAlignedSplitDirection( Range<Math::Float3 const *> vertex_positions, Range<unsigned *> indices )
        {

            auto const box = CreateAxisAlignedBox( vertex_positions, indices );
            auto const max_index = GetMaxElementIndex( box.extent );
            Math::Float3 direction = 0;
            direction[max_index] = 1;
            return direction;
        }
    }

    AxisAlignedBoxHierarchyMesh CreateAxisAlignedBoxHierarchyMesh( std::vector<Math::Float3> vertex_positions, std::vector<unsigned> indices )
    {
        return CreateBoundingShapeHierarchyMesh( move( vertex_positions ), move( indices ), FindAxisAlignedSplitDirection, CreateAxisAlignedBox );
    }


    AxisAlignedBoxHierarchyMesh Transform( AxisAlignedBoxHierarchyMesh mesh, Math::Float4x4 const & transform )
    {
        mesh = Transform<>( std::move( mesh ), transform );
        auto node_count = Size( mesh.nodes );
        for( auto i : CreateIntegerRange( node_count ) )
        {
            auto & node = mesh.nodes[i];
            if( node.escape_index != i + 1 )
            {
                node.shape = Transform( node.shape, transform );
            }
        }
        return mesh;
    }


    AxisAlignedBoxHierarchyMesh TransformByOrientation( AxisAlignedBoxHierarchyMesh mesh, Orientation const & orientation )
    {
        mesh = TransformByOrientation<>( std::move( mesh ), orientation );
        auto node_count = Size( mesh.nodes );
        for( auto i : CreateIntegerRange( node_count ) )
        {
            auto & node = mesh.nodes[i];
            if( node.escape_index != i + 1 )
            {
                node.shape = TransformByOrientation( node.shape, orientation );
            }
        }
        return mesh;
    }
}