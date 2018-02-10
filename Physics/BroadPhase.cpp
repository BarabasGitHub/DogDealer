#include "BroadPhase.h"

#include <BoundingShapes\AxisAlignedBoxFunctions.h>
#include <BoundingShapes\AxisAlignedBoxHierarchyFunctions.h>
#include <BoundingShapes\IntersectionTests.h>

#include <Utilities\StdVectorFunctions.h>

using namespace BoundingShapes;
using namespace Physics;

void Physics::DetectOverlappingPairs(
    AxisAlignedBoxHierarchy const & bounding_boxes_tree,
    Range<AxisAlignedBox const *> bounding_boxes,
    std::vector<std::pair<uint32_t, uint32_t>> & collision_pairs )
{
    for( auto i = 0u; i < Size( bounding_boxes ); ++i )
    {
        auto const box1 = bounding_boxes[i];

        auto node_callback = [box1]( AxisAlignedBox const & node_box )
        {
            return Intersect( box1, node_box );
        };

        auto leaf_callback = [=, &collision_pairs]( uint32_t j )
        {
            // HERE IS THE DIFFERENCE, we check for i < j !!
            if( ( i < j ) && Intersect( box1, bounding_boxes[j] ) )
            {
                collision_pairs.emplace_back( i, j );
            }
            // always continue to traverse the rest of the tree
            return true;
        };

        Traverse( bounding_boxes_tree, node_callback, leaf_callback );
    }
}


void Physics::DetectOverlappingPairs(
    AxisAlignedBoxHierarchy const & bounding_boxes_tree,
    Range<AxisAlignedBox const *> bounding_boxes,
    uint32_t start_offset,
    std::vector<std::pair<uint32_t, uint32_t>> & collision_pairs )
{
    auto i = uint32_t(Size( bounding_boxes ));
    while( i > start_offset )
    {
        --i;
        auto const box1 = bounding_boxes[i];

        auto node_callback = [box1]( AxisAlignedBox const & node_box )
        {
            return Intersect( box1, node_box );
        };

        auto leaf_callback = [=, &collision_pairs]( uint32_t j )
        {
            // HERE IS THE DIFFERENCE, we check for i > j !!
            if( ( i > j ) && Intersect( box1, bounding_boxes[j] ) )
            {
                collision_pairs.emplace_back( i, j );
            }
            // always continue to traverse the rest of the tree
            return true;
        };

        Traverse( bounding_boxes_tree, node_callback, leaf_callback );
    }
}


void Physics::DetectOverlappingPairs(
    Range<AxisAlignedBox const *> bounding_boxes,
    AxisAlignedBoxHierarchy const & bounding_boxes_tree,
    Range<AxisAlignedBox const *> bounding_boxes_of_tree,
    std::vector<std::pair<uint32_t, uint32_t>> & collision_pairs )
{
    for( auto i = 0u; i < Size( bounding_boxes ); ++i )
    {
        auto const box1 = bounding_boxes[i];

        auto node_callback = [box1]( AxisAlignedBox const & node_box )
        {
            return Intersect( box1, node_box );
        };

        auto leaf_callback = [=, &collision_pairs]( uint32_t j )
        {
            if( Intersect( box1, bounding_boxes_of_tree[j] ) )
            {
                collision_pairs.emplace_back( i, j );
            }
            // always continue to traverse the rest of the tree
            return true;
        };

        Traverse( bounding_boxes_tree, node_callback, leaf_callback );
    }
}


namespace
{
    void UpdateAxisAlignedBoxHierarchy(
        Range<BoundingShapes::AxisAlignedBox const *> static_boxes,
        Range<BoundingShapes::AxisAlignedBox const *> dynamic_boxes,
        Range<Orientation const *> dynamic_orientations,
        BoundingShapes::AxisAlignedBoxHierarchy & box_hierarchy,
        std::vector<BoundingShapes::AxisAlignedBox> & transformed_boxes
        )
    {
        assert(Size(dynamic_boxes) == Size(dynamic_orientations));
        auto static_box_count = Size(static_boxes);
        auto total_box_count = static_box_count + Size(dynamic_boxes);

        // first transform all boxes
        transformed_boxes.clear();
        SetCapacity(transformed_boxes, total_box_count);
        Append(transformed_boxes, static_boxes);
        auto transformed_dynamic_boxes = Grow(transformed_boxes, total_box_count - static_box_count);
        TransformByOrientation(dynamic_boxes, dynamic_orientations, transformed_dynamic_boxes);

        CreateAxisAlignedBoxHierarchy( transformed_boxes, box_hierarchy );
    }
}

void Physics::BroadPhaseCollisionDetection(
        Range<BoundingShapes::AxisAlignedBox const *> bounding_boxes,
        Range<Orientation const *> orientations,
        Range<BodyID const *> body_ids,
        uint32_t static_entity_count,
        std::vector<BodyAndOrientationPair> & output)
{
    assert(Size(bounding_boxes) == Size(orientations));
    assert(Size(bounding_boxes) == Size(body_ids));
    assert(Size(bounding_boxes) >= static_entity_count);

    auto static_boxes = CreateRange(bounding_boxes, 0, static_entity_count);
    auto dynamic_boxes = CreateRange(bounding_boxes, static_entity_count);
    auto dynamic_orientations = CreateRange(orientations, static_entity_count);

    BoundingShapes::AxisAlignedBoxHierarchy aabh;
    std::vector<BoundingShapes::AxisAlignedBox> transformed_boxes;
    UpdateAxisAlignedBoxHierarchy(static_boxes, dynamic_boxes, dynamic_orientations, aabh, transformed_boxes);
    BroadPhaseCollisionDetection(transformed_boxes, aabh, orientations, body_ids, static_entity_count, output);
}


void Physics::BroadPhaseCollisionDetection(
        Range<BoundingShapes::AxisAlignedBox const *> transformed_boxes,
        BoundingShapes::AxisAlignedBoxHierarchy const & box_hierarchy,
        Range<Orientation const *> orientations,
        Range<BodyID const *> body_ids,
        uint32_t static_entity_count,
        std::vector<BodyAndOrientationPair> & output)
{
    assert(Size(transformed_boxes) == Size(orientations));
    assert(Size(transformed_boxes) == Size(body_ids));
    assert(Size(transformed_boxes) >= static_entity_count);

    std::vector<std::pair<uint32_t, uint32_t>> overlapping_index_pairs;
    DetectOverlappingPairs( box_hierarchy, transformed_boxes, static_entity_count, overlapping_index_pairs);

    auto output_range = Grow(output, Size(overlapping_index_pairs));
    for (auto i = 0u; i < Size(overlapping_index_pairs); ++i)
    {
        auto index_pair = overlapping_index_pairs[i];
        BodyAndOrientationPair thingy;
        thingy.body1 = body_ids[index_pair.first];
        thingy.body2 = body_ids[index_pair.second];
        thingy.orientation1 = orientations[index_pair.first];
        thingy.orientation2 = orientations[index_pair.second];
        output_range[i] = thingy;
    }
}


void Physics::BroadPhaseRayCasting(
    Range<BoundingShapes::AxisAlignedBox const *> transformed_boxes,
    BoundingShapes::AxisAlignedBoxHierarchy const & box_hierarchy,
    Range<BodyID const *> body_ids,
    BoundingShapes::Ray const & ray,
    std::vector<BodyID> & output
    )
{
    assert(Size(transformed_boxes) == Size(body_ids));
    auto node_callback = [ray]( AxisAlignedBox const & node_box )
    {
        return Intersect( node_box, ray );
    };

    auto leaf_callback = [&ray,&output, body_ids, transformed_boxes]( uint32_t j )
    {
        // if( Intersect(transformed_boxes[j], ray) || true )
        if( Intersect(transformed_boxes[j], ray) )
        {
            output.emplace_back( body_ids[j] );
        }
        // always continue to traverse the rest of the tree
        return true;
    };

    Traverse( box_hierarchy, node_callback, leaf_callback );
}

