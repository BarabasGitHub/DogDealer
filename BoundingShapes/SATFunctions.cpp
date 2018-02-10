#include "SATFunctions.h"

#include "AxisAlignedBoxFunctions.h"

#include <Utilities\MinMaxFunctions.h>

#include <Math\MathFunctions.h>
#include <Math\TransformFunctions.h>
#include <Math\FloatOperators.h>
#include <Math\SSEFloatFunctions.h>
#include <Math\SSETransformFunctions.h>
#include <Math\SSEQuaternionFunctions.h>
#include <Math\SSEMathConversions.h>
#include <Math\SSE.h>

using namespace Math;
using namespace BoundingShapes;

namespace SAT
{

    void GetAxisAlignedBoxAxes( Range<Math::Float3 *> const output )
    {
        assert( Size( output ) == 3 );
        output[0] = Float3( 1, 0, 0 );
        output[1] = Float3( 0, 1, 0 );
        output[2] = Float3( 0, 0, 1 );
    }


    void GetAxes( BoundingShapes::AxisAlignedBox const &, Range<Math::Float3 *> const output )
    {
        GetAxisAlignedBoxAxes( output );
    }


    // Returns transformed X, Y and Z axis of box
    std::array<Math::Float3, 3> GetAxes( BoundingShapes::OrientedBox const & box )
    {
        std::array<Math::Float3, 3> output;
        GetAxes(box, output);
        return output;
    }

    void GetAxes( BoundingShapes::OrientedBox const & box, Range<Math::Float3 *> const output )
    {
        assert( Size(output) == 3 );
        using namespace Math::SSE;
        auto rotation_matrix = RotationMatrixFromQuaternion( QuaternionConjugate( SSEFromQuaternion( box.rotation ) ) );
        output[0] = Float3FromSSE(rotation_matrix.row[0]);
        output[1] = Float3FromSSE(rotation_matrix.row[1]);
        output[2] = Float3FromSSE(rotation_matrix.row[2]);
    }

    // Project all vertices of the shape onto the input axis
    // return minimum and maximum value
    MinMax<float> ProjectToAxis( Range<Math::Float3 const *> const points, Math::Float3 const & axis_in )
    {
        using namespace Math::SSE;
        // Get minimum and maximum values
        MinMax<Float32Vector> minmax = { SetAll(std::numeric_limits<float>::max()), SetAll(std::numeric_limits<float>::lowest()) };
        auto axis = SSEFromFloat3(axis_in);

        auto i = 0u;
        auto size = Size(points);
        auto batch_limit = 4 * (size / 4);
        Math::Float3 const * __restrict points_data = begin(points);
        while(  i < batch_limit )
        {
            // do four dot products at the same time
            FloatMatrix four_points;
            four_points.row[0] = SSEFromFloat3(points_data[i + 0]);
            four_points.row[1] = SSEFromFloat3(points_data[i + 1]);
            four_points.row[2] = SSEFromFloat3(points_data[i + 2]);
            four_points.row[3] = SSEFromFloat3(points_data[i + 3]);
            auto four_dots = Multiply4D(four_points, axis);

            minmax = Update(minmax, four_dots);

            i += 4;
        }

        // reduce the four min & max-es to one common min & max
        minmax.min = Min(Swizzle<2, 3>(minmax.min), minmax.min);
        minmax.min = MinSingle(Swizzle<1>(minmax.min), minmax.min);

        minmax.max = Max(Swizzle<2, 3>(minmax.max), minmax.max);
        minmax.max = MaxSingle(Swizzle<1>(minmax.max), minmax.max);

        // process the remainder
        while( i < size )
        {
            auto dot = Dot3( SSEFromFloat3(points_data[i]), axis );

            minmax.min = MinSingle(minmax.min, dot);
            minmax.max = MaxSingle(minmax.max, dot);
            i += 1;
        }

        return{ GetSingle(minmax.min), GetSingle(minmax.max) };
    }


    float CalculateOverlap( MinMax<float> range1, MinMax<float> range2 )
    {
        auto overlap1 = range1.max - range2.min;
        auto overlap2 = range2.max - range1.min;
        return Math::Abs(overlap1) < Math::Abs(overlap2) ? overlap1 : overlap2;
        //using namespace SSE;
        // auto min2max2 = Load2( &range2.min );
        // auto min2max2min1max1 = Load2Upper( &range1.min, min2max2 );
        // auto max1min1max2min2 = Swizzle<3,2,1,0>( min2max2min1max1 );

        // auto overlap_a = Subtract( max1min1max2min2, min2max2min1max1 );
        // auto overlap_b = Swizzle<3, 2, 1, 0>( overlap_a );
        // auto abs_overlap = Max( overlap_a, overlap_b );
        // return GetSingle( abs_overlap ) < GetSingle( Swizzle<1>( abs_overlap ) ) ? GetSingle( overlap_a ) : GetSingle( Swizzle<2>( overlap_a ) );
    }


    bool CalculateOverlap(Range<Math::Float3 const *> axes, Range<Math::Float3 const *> points1, Range<Math::Float3 const *> points2, float & min_depth_out, uint32_t & min_depth_axis_out )
    {
        auto min_depth = std::numeric_limits<float>::max();
        auto min_depth_axis = 0u;

        auto axes_count = Size( axes );
        for( auto axis_index = 0u; axis_index < axes_count; ++axis_index )
        {
            auto axis = axes[axis_index];
            auto minmax1 = SAT::ProjectToAxis( points1, axis );
            auto minmax2 = SAT::ProjectToAxis( points2, axis );

            auto overlap = SAT::CalculateOverlap( minmax1, minmax2 );
            if( overlap > 0 )
            {
                if( overlap < min_depth )
                {
                    min_depth = overlap;
                    min_depth_axis = axis_index;
                }
            }
            else
            {
                return false;
            }
        }

        min_depth_out = min_depth;
        min_depth_axis_out = min_depth_axis;
        return true;
    }


    bool CalculateOverlap(
        Range<Math::Float3 const *> edges1, Range<Math::Float3 const *> edges2,
        Range<Math::Float3 const *> points1, Range<Math::Float3 const *> points2,
        float const zero_axis_tolerance,
        float & min_depth_out, uint32_t & min_depth_edge1_out, uint32_t & min_depth_edge2_out, Math::Float3 & axis_out)
    {
        auto min_depth = std::numeric_limits<float>::max();
        auto min_depth_edge1 = 0u;
        auto min_depth_edge2 = 0u;
        Math::Float3 min_depth_axis = 0;

        auto edge_count1 = Size(edges1);
        auto edge_count2 = Size(edges2);
        for (auto i = 0u; i < edge_count1; ++i)
        {
            auto edge1 = edges1[i];
            for (auto j = 0u; j < edge_count2; ++j)
            {
                auto edge2 = edges2[j];
                auto axis = Cross(edge1, edge2);
                if( !Math::Equal(axis, Math::Float3(0), zero_axis_tolerance) )
                {
                    auto minmax1 = SAT::ProjectToAxis( points1, axis );
                    auto minmax2 = SAT::ProjectToAxis( points2, axis );

                    auto overlap = SAT::CalculateOverlap( minmax1, minmax2 );
                    if( overlap > 0 )
                    {
                        auto norm = Norm( axis );
                        overlap /= norm;
                        if( overlap < min_depth )
                        {
                            min_depth = overlap;
                            min_depth_edge1 = i;
                            min_depth_edge2 = j;
                            min_depth_axis = axis / norm;
                        }
                    }
                    else
                    {
                        return false;
                    }
                }
            }
        }
        min_depth_out = min_depth;
        min_depth_edge1_out = min_depth_edge1;
        min_depth_edge2_out = min_depth_edge2;
        axis_out = min_depth_axis;
        return true;
    }

}