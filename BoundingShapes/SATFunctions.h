#pragma once

#include "OrientedBox.h"
#include "Triangle.h"
#include "AxisAlignedBox.h"

#include <Math\FloatTypes.h>

#include <Utilities\Range.h>
#include <Utilities\MinMax.h>

#include <array>
#include <cstdint>

namespace SAT
{
    void GetAxisAlignedBoxAxes( Range<Math::Float3 *> const output );
    void GetAxes( BoundingShapes::AxisAlignedBox const &, Range<Math::Float3 *> const output );

    // Returns transformed X, Y and Z axis of box
    std::array<Math::Float3, 3> GetAxes( BoundingShapes::OrientedBox const & box );
    void GetAxes( BoundingShapes::OrientedBox const & box, Range<Math::Float3 *> const output );


    MinMax<float> ProjectToAxis( Range<Math::Float3 const *> const points, Math::Float3 const & axis );

    float CalculateOverlap( MinMax<float> range1, MinMax<float> range2 );

    // calculates the overlap if any between points1 and points2 along the axes. The axes are assumed to be normalized
    // return true if there is overlap, false if not.
    // if there is overlap min_depth and min_depth_axis_index are assigned, otherwise they won't be touched.
    bool CalculateOverlap( Range<Math::Float3 const *> axes,
                           Range<Math::Float3 const *> points1,
                           Range<Math::Float3 const *> points2,
                           // output
                           float & min_depth, uint32_t & min_depth_axis_index );


    // calculates the overlap if any between points1 and points2 along the axes formed by any combination of and edge of edges1 and an edge of edges2
    // return true if there is overlap, false if not.
    // if there is overlap min_depth, min_depth_edge1_out and min_depth_edge2_out are assigned, otherwise they won't be touched.
    bool CalculateOverlap(
        Range<Math::Float3 const *> edges1, Range<Math::Float3 const *> edges2,
        Range<Math::Float3 const *> points1, Range<Math::Float3 const *> points2,
        float zero_axis_tolerance,
        float & min_depth_out, uint32_t & min_depth_edge1_out, uint32_t & min_depth_edge2_out, Math::Float3 & min_depth_axis_out);
}