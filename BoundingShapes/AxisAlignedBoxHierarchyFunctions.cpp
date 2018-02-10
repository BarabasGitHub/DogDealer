#pragma once
#include "AxisAlignedBoxHierarchyFunctions.h"

#include "AxisAlignedBoxFunctions.h"

#include <Utilities\MinMax.h>

#include <Math\FloatOperators.h>
#include <Math\MathFunctions.h>
#include <Math\IntegerTypes.h>

#include <Utilities\IntegerRange.h>
#include <Utilities\VertexReordering.h>
#include <Utilities\StdVectorFunctions.h>

namespace BoundingShapes
{
    namespace
    {

        uint32_t DivideUpper( uint32_t value, uint32_t divider )
        {
            return ( value + divider - 1 ) / divider;
        }


        uint32_t Ceil( uint32_t value, uint32_t rounding )
        {
            return DivideUpper(value, rounding) * rounding;
        }


        uint32_t HalfCeil( uint32_t value, uint32_t rounding )
        {
            return DivideUpper( DivideUpper( value, rounding ), 2) * rounding;
        }

        void Split( uint8_t direction, Range<MinMax<Math::Float3> const *> boxes_minmax, Range<uint32_t *> indices )
        {

            auto const box_count = uint32_t(Size(indices));
            auto const half = begin( indices ) + HalfCeil( box_count, AxisAlignedBoxHierarchy::Node::c_leaf_entries );
            std::nth_element( begin(indices), half, end(indices), [direction, &boxes_minmax]( uint32_t i_a, uint32_t i_b )
            {
                auto & a_minmax_float3 = boxes_minmax[i_a];
                auto & b_minmax_float3 = boxes_minmax[i_b];
                MinMax<float> a_minmax = {a_minmax_float3.min[direction], a_minmax_float3.max[direction]};
                MinMax<float> b_minmax = {b_minmax_float3.min[direction], b_minmax_float3.max[direction]};


                // assuming the direction is positive
                // separated: 1 before 2
                if( a_minmax.max < b_minmax.min )
                {
                    return true;
                }
                // separated: 2 before 1
                else if( b_minmax.max < a_minmax.min )
                {
                    return false;
                }
                // overlapping: 1 before 2
                else if( a_minmax.min < b_minmax.min && a_minmax.max < b_minmax.max )
                {
                    return true;
                }
                // overlapping: 2 before 1
                else if( b_minmax.min < a_minmax.min && b_minmax.max < a_minmax.max )
                {
                    return false;
                }
                // overlapping and one extends beyond the other, check the centre or indices them self
                auto const a_center = a_minmax.max - a_minmax.min;
                auto const b_center = b_minmax.max - b_minmax.min;

                return a_center < b_center;
            }
            );
        }

        // returns the number of child nodes in the tree given the number of leafs
        uint32_t ChildNodesFromLeafs( uint32_t leaf_count )
        {
            return ( leaf_count - 1 ) * 2;
        }


        uint32_t NodesFromLeafs( uint32_t leaf_count )
        {
            return leaf_count * 2 - 1;
        }


        uint32_t LeafsFromChildNodes( uint32_t child_count )
        {
            return child_count / 2 + 1;
        }


        void SortBoxesAndAddNodes( Range<MinMax<Math::Float3> const *> boxes_minmax, Range<uint32_t *> indices, std::vector< AxisAlignedBoxHierarchy::Node > & nodes )
        {
            auto const box_count = uint32_t( Size( indices ) );
            AxisAlignedBoxHierarchy::Node node;
            node.escape_index = uint32_t( Size( nodes ) );
            if( box_count > AxisAlignedBoxHierarchy::Node::c_leaf_entries )
            {
                node.box = CreateAxisAlignedBox( boxes_minmax, indices );
                auto split_direction = GetMaxElementIndex( node.box.extent );
                node.escape_index += NodesFromLeafs( DivideUpper( box_count, AxisAlignedBoxHierarchy::Node::c_leaf_entries ) );
                Append(nodes, node);
                Split( split_direction, boxes_minmax, indices );

                // divide in half, with the first half having one more if uneven number of triplets
                auto const half_point = begin( indices ) + HalfCeil( box_count, AxisAlignedBoxHierarchy::Node::c_leaf_entries );
                auto const half1 = CreateRange( begin( indices ), half_point );
                auto const half2 = CreateRange( half_point, end( indices ) );
                SortBoxesAndAddNodes( boxes_minmax, half1, nodes );
                SortBoxesAndAddNodes( boxes_minmax, half2, nodes );
            }
            else
            {
                assert( box_count != 0 );
                auto i = std::copy_n(begin(indices), Size(indices), begin(node.indices));
                std::fill_n( i, Size( node.indices ) - Size( indices ), uint32_t(-1) );
                node.escape_index += 1;
                Append(nodes, node );
            }
        }
    }


    AxisAlignedBoxHierarchy CreateAxisAlignedBoxHierarchy( Range<MinMax<Math::Float3> const *> minmax, std::vector<uint32_t>& indices )
    {
        AxisAlignedBoxHierarchy hierarchy;
        CreateAxisAlignedBoxHierarchy(minmax, indices, hierarchy);
        return hierarchy;
    }


    void CreateAxisAlignedBoxHierarchy( Range<MinMax<Math::Float3> const *> minmax, std::vector<uint32_t>& indices, AxisAlignedBoxHierarchy & hierarchy )
    {
        if( IsEmpty( minmax ) )
        {
            return;
        }
        auto const box_count = uint32_t( Size( minmax ) );
        auto index_range = CreateIntegerRange( box_count );
        indices.assign( begin( index_range ), end( index_range ) );

        auto leafs = DivideUpper( box_count, AxisAlignedBoxHierarchy::Node::c_leaf_entries );
        auto node_count = NodesFromLeafs( leafs );
        auto & nodes = hierarchy.nodes;
        nodes.clear();
        SetCapacity(nodes, node_count);

        SortBoxesAndAddNodes(minmax, indices, nodes);
    }


    AxisAlignedBoxHierarchy CreateAxisAlignedBoxHierarchy( Range<AxisAlignedBox const *> boxes, std::vector<uint32_t>& indices )
    {
        std::vector<MinMax<Math::Float3>> minmax(Size(boxes));
        std::transform( begin( boxes ), end( boxes ), begin( minmax ), GetMinMax );

        return CreateAxisAlignedBoxHierarchy( minmax, indices );
    }


    void CreateAxisAlignedBoxHierarchy( Range<AxisAlignedBox const *> boxes, std::vector<uint32_t>& indices, AxisAlignedBoxHierarchy & hierarchy )
    {
        std::vector<MinMax<Math::Float3>> minmax(Size(boxes));
        std::transform( begin( boxes ), end( boxes ), begin( minmax ), GetMinMax );

        CreateAxisAlignedBoxHierarchy( minmax, indices, hierarchy );
    }


    AxisAlignedBoxHierarchy CreateAxisAlignedBoxHierarchy( Range<AxisAlignedBox const *> boxes )
    {
        std::vector<uint32_t> indices;
        return CreateAxisAlignedBoxHierarchy( boxes, indices );
    }


    void CreateAxisAlignedBoxHierarchy( Range<AxisAlignedBox const *> boxes, AxisAlignedBoxHierarchy & hierarchy )
    {
        std::vector<uint32_t> indices;
        CreateAxisAlignedBoxHierarchy( boxes, indices, hierarchy );
    }
}
