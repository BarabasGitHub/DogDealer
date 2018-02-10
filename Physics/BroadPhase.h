#pragma once

#include "BodyAndOrientationPair.h"
#include "BodyID.h"

#include <Conventions\Orientation.h>
#include <Utilities\Range.h>

#include <vector>
#include <utility> // for pair
#include <cstdint>

namespace BoundingShapes
{
    struct AxisAlignedBox;
    struct AxisAlignedBoxHierarchy;
    struct Ray;
}

namespace Physics
{
    // bounds should be transformed already
    // detects overlapping pairs between axis aligned boxes in the same range
    void DetectOverlappingPairs(
        BoundingShapes::AxisAlignedBoxHierarchy const & bounding_boxes_tree,
        Range<BoundingShapes::AxisAlignedBox const *> bounding_boxes_of_tree,
        std::vector<std::pair<uint32_t, uint32_t>> & overlapping_pairs
        );

    // bounds should be transformed already
    // detects overlapping pairs between axis aligned boxes in the same range starting from start_offset
    void DetectOverlappingPairs(
        BoundingShapes::AxisAlignedBoxHierarchy const & bounding_boxes_tree,
        Range<BoundingShapes::AxisAlignedBox const *> bounding_boxes,
        uint32_t start_offset,
        std::vector<std::pair<uint32_t, uint32_t>> & collision_pairs
        );

    // bounds should be transformed already
    // detects overlapping pairs between axis aligned boxes in different ranges
    void DetectOverlappingPairs(
        Range<BoundingShapes::AxisAlignedBox const *> bounding_boxes,
        BoundingShapes::AxisAlignedBoxHierarchy const & bounding_boxes_tree,
        Range<BoundingShapes::AxisAlignedBox const *> bounding_boxes_of_tree,
        std::vector<std::pair<uint32_t, uint32_t>> & overlapping_pairs
        );


    // assumes the static entities come first in the ranges
    void BroadPhaseCollisionDetection(
        Range<BoundingShapes::AxisAlignedBox const *> bounding_boxes,
        Range<Orientation const *> orientations,
        Range<BodyID const *> body_ids,
        uint32_t static_entity_count,
        std::vector<BodyAndOrientationPair> & output
        );


    // assumes the static entities come first in the ranges
    void BroadPhaseCollisionDetection(
        Range<BoundingShapes::AxisAlignedBox const *> transformed_boxes,
        BoundingShapes::AxisAlignedBoxHierarchy const & box_hierarchy,
        Range<Orientation const *> orientations,
        Range<BodyID const *> body_ids,
        uint32_t static_entity_count,
        std::vector<BodyAndOrientationPair> & output
        );


    void BroadPhaseRayCasting(
        Range<BoundingShapes::AxisAlignedBox const *> transformed_boxes,
        BoundingShapes::AxisAlignedBoxHierarchy const & box_hierarchy,
        Range<BodyID const *> body_ids,
        BoundingShapes::Ray const & ray,
        std::vector<BodyID> & output
        );
}
