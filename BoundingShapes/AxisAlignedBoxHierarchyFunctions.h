#pragma once
#include "AxisAlignedBoxHierarchy.h"

#include <Utilities\Range.h>
#include <Utilities\MinMax.h>

#include <cstdint>

namespace BoundingShapes
{
    // creates a hierarchy from a range of minmaxes, the indices in the leafs will point to the index of the minmax in the input range
    AxisAlignedBoxHierarchy CreateAxisAlignedBoxHierarchy( Range<MinMax<Math::Float3> const *> minmax, std::vector<uint32_t>& indices );
    void CreateAxisAlignedBoxHierarchy( Range<MinMax<Math::Float3> const *> minmax, std::vector<uint32_t>& indices, AxisAlignedBoxHierarchy & hierarchy );
    // creates a hierarchy from a range of boxes, the indices in the leafs will point to the index of the box in the range
    AxisAlignedBoxHierarchy CreateAxisAlignedBoxHierarchy( Range<AxisAlignedBox const *> boxes );
    void CreateAxisAlignedBoxHierarchy( Range<AxisAlignedBox const *> boxes, AxisAlignedBoxHierarchy & hierarchy );
    // creates a hierarchy from a range of boxes and returns the indices used
    AxisAlignedBoxHierarchy CreateAxisAlignedBoxHierarchy( Range<AxisAlignedBox const *> boxes, std::vector<uint32_t>& indices );
    void CreateAxisAlignedBoxHierarchy( Range<AxisAlignedBox const *> boxes, std::vector<uint32_t>& indices, AxisAlignedBoxHierarchy & hierarchy );

    // Traverse the tree
    // calls node_function for each node
    //   - input is the box of the node
    //   - if it returns true continue, otherwise abort this branch of the tree and continue with the next branch
    // calls leaf_function for each leaf
    //   - input is the index of the leaf data
    //   - if it returns true continue, otherwise stop traversing
    template<typename NodeFunctionType, typename LeafFunctionType>
    void Traverse(AxisAlignedBoxHierarchy const & tree, NodeFunctionType node_function, LeafFunctionType leaf_function);
}


namespace BoundingShapes
{

    template<typename NodeFunctionType, typename LeafFunctionType>
    void Traverse(AxisAlignedBoxHierarchy const & tree, NodeFunctionType node_function, LeafFunctionType leaf_function)
    {
        auto i = 0u;
        while( i < Size(tree.nodes) )
        {
            auto const & node = tree.nodes[i];
            ++i;

            if( node.escape_index != i )
            {
                if( !node_function(node.box) )
                {
                    i = node.escape_index;
                }
            }
            else
            {
                for( auto index : node.indices )
                {
                    if( index == uint32_t(-1) ) break;
                    if( !leaf_function(index) ) return;
                }
            }
        }
    }
}
