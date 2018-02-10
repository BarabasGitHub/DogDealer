#pragma once
#include "BoundingShapeHierarchyMeshFunctions.h"

#include "TriangleFunctions.h"

#include "SATFunctions.h"

#include <Math\FloatOperators.h>
#include <Math\MathFunctions.h>
#include <Math\IntegerTypes.h>

#include <Utilities\IntegerRange.h>
#include <Utilities\VertexReordering.h>

#include <tuple> // for tie


namespace BoundingShapes
{
    namespace
    {
        void Split( Range<unsigned *> indices, std::vector<Math::Float3> const & vertex_positions, FindSplitDirectionFunction const find_split_direction )
        {
            using namespace Math;
            auto const direction = find_split_direction( vertex_positions, indices );

            // do this ugly thing so we can sort the triplets, without having to copy everything
            auto const triplet_range = ReinterpretRange<Unsigned3>(indices);

            auto const size = Size(triplet_range);
            auto const half = triplet_range.start + size / 2;
            std::nth_element( triplet_range.start, half, triplet_range.stop, [&vertex_positions, direction]( Unsigned3 const & first, Unsigned3 const & second )
            {

                auto const triangle1 = CreateTriangle( vertex_positions[first[0]], vertex_positions[first[0]], vertex_positions[first[0]] );
                auto const triangle2 = CreateTriangle( vertex_positions[second[0]], vertex_positions[second[0]], vertex_positions[second[0]] );

                auto const minmax1 = SAT::ProjectToAxis( triangle1.corners, direction );
                auto const minmax2 = SAT::ProjectToAxis( triangle2.corners, direction );

                // assuming the direction is positive
                // separated: 1 before 2
                if( minmax1.max < minmax2.min )
                {
                    return true;
                }
                // separated: 2 before 1
                else if( minmax2.max < minmax1.min )
                {
                    return false;
                }
                // overlapping: 1 before 2
                else if( minmax1.min < minmax2.min && minmax1.max < minmax2.max )
                {
                    return true;
                }
                // overlapping: 2 before 1
                else if( minmax2.min < minmax1.min && minmax2.max < minmax1.max )
                {
                    return false;
                }
                // overlapping and one extends beyond the other, check the center or indices themself
                auto const center1 = minmax1.max - minmax1.min;
                auto const center2 = minmax2.max - minmax2.min;

                return std::tie( center1, first[0], first[1], first[2] ) < std::tie( center2, second[0], second[1], second[2] );
            }
            );
        }

        // returns the number of child nodes in the tree given the number of leafs
        unsigned ChildNodesFromLeafs( unsigned leaf_count )
        {
            return ( leaf_count - 1 ) * 2;
        }

        unsigned NodesFromLeafs( unsigned leaf_count )
        {
            return leaf_count * 2 - 1;
        }

        unsigned LeafsFromChildNodes( unsigned child_count )
        {
            return child_count / 2 + 1;
        }

        template<typename NodeType>
        void SortIndicesAndAddNodes( Range<unsigned *> indices, std::vector<Math::Float3> const & vertex_positions, std::vector<NodeType> & nodes, FindSplitDirectionFunction const find_split_direction )
        {
            auto const triplet_count = unsigned( Size(indices) / 3 );
            if( triplet_count > 1 )
            {
                auto const index = unsigned( nodes.size( ) );
                nodes.emplace_back( );
                nodes.back( ).escape_index = index + NodesFromLeafs( triplet_count );
                Split( indices, vertex_positions, find_split_direction );

                // divide in half, with the first half having one more if uneven number of triplets
                // multiply with 3 because we split the triplets and have a vector of bare indices
                auto const half_point = indices.start + ( ( triplet_count + 1 ) / 2 ) * 3;
                auto const half1 = CreateRange( indices.start, half_point );
                auto const half2 = CreateRange( half_point, indices.stop );
                SortIndicesAndAddNodes( half1, vertex_positions, nodes, find_split_direction );
                SortIndicesAndAddNodes( half2, vertex_positions, nodes, find_split_direction );
            }
            else
            {
                assert( triplet_count != 0 );
                nodes.emplace_back( );
                nodes.back( ).escape_index = unsigned( nodes.size( ) );
            }
        }

        template<typename ShapeType>
        void FillNodes( std::vector<typename  BoundingShapeHierarchyMesh<ShapeType>::Node> & nodes, std::vector<unsigned> const & indices, std::vector<Math::Float3> const & vertex_positions, CreateShapeFunction<ShapeType> const create_shape )
        {
            auto const node_count = unsigned( nodes.size( ) );
            auto index_begin = indices.data( );
            for( auto i : CreateIntegerRange( node_count ) )
            {
                auto & node = nodes[i];
                if( node.escape_index == i + 1 )
                {
                    std::copy_n( index_begin, 3, node.vertex_indices.begin( ) );
                    index_begin += 3;
                }
                else
                {
                    auto const leaf_count = LeafsFromChildNodes( node.escape_index - i );
                    node.shape = create_shape( vertex_positions, CreateRange( index_begin, 3 * leaf_count ) );
                }
            }
        }
    }

    template<typename ShapeType>
    BoundingShapeHierarchyMesh<ShapeType> CreateBoundingShapeHierarchyMesh( std::vector<Math::Float3> vertex_positions, std::vector<unsigned> indices, FindSplitDirectionFunction const find_split_direction, CreateShapeFunction<ShapeType> create_shape )
    {
        auto const node_count = NodesFromLeafs( unsigned( indices.size( ) / 3 ) );
        std::vector<typename BoundingShapeHierarchyMesh<ShapeType>::Node> nodes;
        nodes.reserve( node_count );
        SortIndicesAndAddNodes( indices, vertex_positions, nodes, find_split_direction );
        auto new_indices = CalculateNewIndices( indices, unsigned( vertex_positions.size( ) ) );
        vertex_positions = ReorderData( std::move( vertex_positions ), indices, new_indices );

        FillNodes( nodes, new_indices, vertex_positions, create_shape );

        return{ nodes, vertex_positions };
    }


    template<typename ShapeType>
    BoundingShapeHierarchyMesh<ShapeType> Transform( BoundingShapeHierarchyMesh<ShapeType> mesh, Math::Float4x4 const & transform )
    {
        for( auto & position : mesh.vertex_positions )
        {
            position = TransformPosition( position, transform );
        }
        return mesh;
    }

    template<typename ShapeType>
    BoundingShapeHierarchyMesh<ShapeType> TransformByOrientation( BoundingShapeHierarchyMesh<ShapeType> mesh, Orientation const & orientation )
    {
        for( auto & position : mesh.vertex_positions )
        {
            auto rotated = Rotate( position, orientation.rotation );
            position = rotated + orientation.position;
        }
        return mesh;
    }


    template<typename ShapeType, typename NodeFunctionType, typename LeafFunctionType>
    void Traverse(BoundingShapeHierarchyMesh<ShapeType> const & tree, NodeFunctionType node_function, LeafFunctionType leaf_function)
    {
        auto i = 0u;
        while( i < Size(tree.nodes) )
        {
            auto const & node = tree.nodes[i];
            ++i;

            if( node.escape_index != i )
            {
                if( !node_function(node.shape) )
                {
                    i = node.escape_index;
                }
            }
            else
            {
                if( !leaf_function(node.vertex_indices) ) return;
            }
        }
    }
}