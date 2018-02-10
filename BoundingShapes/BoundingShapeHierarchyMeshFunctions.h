#pragma once
#include "BoundingShapeHierarchyMesh.h"

#include <Conventions\Orientation.h>

#include <Math\FloatTypes.h>

#include <Utilities\Range.h>

#include <vector>

namespace BoundingShapes
{

    typedef Math::Float3( *FindSplitDirectionFunction )( Range<Math::Float3 const *> vertex_positions, Range<unsigned *> indices );

    template<typename ShapeType>
    using CreateShapeFunction = ShapeType( *)( Range<Math::Float3 const*> positions, Range<unsigned const *> indices );

    template<typename ShapeType>
    BoundingShapeHierarchyMesh<ShapeType> CreateBoundingShapeHierarchyMesh( std::vector<Math::Float3> vertex_positions, std::vector<unsigned> indices, FindSplitDirectionFunction const find_split_direction, CreateShapeFunction<ShapeType> create_shape );

    template<typename ShapeType>
    BoundingShapeHierarchyMesh<ShapeType> Transform( BoundingShapeHierarchyMesh<ShapeType> mesh, Math::Float4x4 const & transform );

    template<typename ShapeType>
    BoundingShapeHierarchyMesh<ShapeType> TransformByOrientation( BoundingShapeHierarchyMesh<ShapeType> mesh, Orientation const & orientation );


    // Traverse the tree
    // calls node_function for each node
    //   - input is the shape of the node
    //   - if it returns true continue, otherwise abort this branch of the tree and continue with the next branch
    // calls leaf_function for each leaf
    //   - input is the vertex indices of the leaf
    //   - if it returns true continue, otherwise stop traversing
    template<typename ShapeType, typename NodeFunctionType, typename LeafFunctionType>
    void Traverse(BoundingShapeHierarchyMesh<ShapeType> const & tree, NodeFunctionType node_function, LeafFunctionType leaf_function);
}

#include "BoundingShapeHierarchyMeshFunctions.inl"