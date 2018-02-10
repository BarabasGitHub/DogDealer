#include "AxisAlignedBoxFunctions.h"

#include "AxisAlignedBoxSSEFunctions.h"

#include "Implementation.h"

#include <Math\FloatOperators.h>
#include <Math\MathFunctions.h>
#include <Math\FloatMatrixOperators.h>
#include <Math\TransformFunctions.h>
#include <Math\SSEMathConversions.h>
#include <Math\SSEFloatFunctions.h>

#include <Utilities\IntegerRange.h>

// implementations

using namespace Math;

namespace BoundingShapes
{
    AxisAlignedBox CreateAxisAlignedBox( MinMax<Math::Float3> minmax )
    {
        auto extent = ( minmax.max - minmax.min ) / 2;
        auto center = ( minmax.max + minmax.min ) / 2;

        AxisAlignedBox box;
        box.extent = extent;
        box.center = center;
        return box;
    }


    namespace
    {
        template<typename IteratorType>
        AxisAlignedBox CreateAxisAlignedBoxImpl( Range<MinMax<Math::Float3> const * > minmaxes, Range<IteratorType> indices)
        {
            using namespace SSE;
            assert(!IsEmpty(indices));
            auto & first_minmax = minmaxes[First( indices )];
            MinMax<Float32Vector> final_minmax{ SSEFromFloat3( first_minmax.min ), SSEFromFloat3( first_minmax.max ) };
            PopFirst(indices);
            for( auto i : indices )
            {
                final_minmax.min = SSE::Min( final_minmax.min, SSEFromFloat3( minmaxes[i].min ) );
                final_minmax.max = SSE::Max( final_minmax.max, SSEFromFloat3( minmaxes[i].max ) );
            }

            auto extent = Subtract( final_minmax.max, final_minmax.min );
            auto center = Add( final_minmax.max, final_minmax.min );

            extent = Divide( extent, SetAll( 2.f ) );
            center = Divide( center, SetAll( 2.f ) );
            AxisAlignedBox box;
            box.extent = Float3FromSSE( extent );
            box.center = Float3FromSSE( center );
            return box;
        }
    }


    AxisAlignedBox CreateAxisAlignedBox( Range<MinMax<Math::Float3> const *> minmaxes )
    {
        return CreateAxisAlignedBoxImpl(minmaxes, CreateIntegerRange(Size(minmaxes)));
    }


    AxisAlignedBox CreateAxisAlignedBox( Range<MinMax<Math::Float3> const *> minmaxes, Range<uint32_t const *> indices )
    {
        return CreateAxisAlignedBoxImpl(minmaxes, indices);
    }


    namespace
    {
        template<typename IteratorType>
        AxisAlignedBox CreateAxisAlignedBoxImpl( Range<Math::Float3 const *> points, Range<IteratorType> indices )
        {
            using namespace Math::SSE;
            assert( !IsEmpty( indices ) );
            // use the first point to initialize min and max
            auto max = SSEFromFloat3(points[First(indices)]);
            auto min = max;
            PopFirst(indices);

            for( auto const i : indices )
            {
                auto const point = SSEFromFloat3( points[i] );
                max = Max( max, point );
                min = Min( min, point );
            }

            return CreateAxisAlignedBox( MinMax<Math::Float3>{ Float3FromSSE( min ), Float3FromSSE( max ) } );
        }
    }


    AxisAlignedBox CreateAxisAlignedBox( Range<Math::Float3 const *> points )
    {
        return CreateAxisAlignedBoxImpl( points, CreateIntegerRange( Size( points ) ) );
    }


    AxisAlignedBox CreateAxisAlignedBox( Range<Math::Float3 const *> points, Range<unsigned const *> indices )
    {
        return CreateAxisAlignedBoxImpl( points, indices );
    }


    AxisAlignedBox RotateAroundCenter( AxisAlignedBox box, Math::Quaternion rotation )
    {
        return RotateAroundCenter( box, RotationToFloat3x3( rotation ) );
    }


    AxisAlignedBox RotateAroundCenter( AxisAlignedBox box, Math::Float3x3 rotation )
    {
        // used idea from http://www.akshayloke.com/2012/10/22/optimized-transformations-for-aabbs/ and http://dev.theomader.com/transform-bounding-boxes/
        // but adapted it to extent instead of min, max
        box.extent = Abs( rotation ) * box.extent;
        return box;
    }


    AxisAlignedBox Transform( AxisAlignedBox const & box, Math::Float4x4 const & m )
    {
        using namespace SSE;
        auto center = SSEFromFloat3( box.center );
        auto extent = SSEFromFloat3( box.extent );
        auto transform = SSEFromFloat4x4( m );
        TransformAxisAlignedBoxSSE( center, extent, transform );
        AxisAlignedBox transformed_box;
        transformed_box.center = Float3FromSSE( center );
        transformed_box.extent = Float3FromSSE( extent );
        return transformed_box;
    }


    void Transform( Range<AxisAlignedBox const *> boxes, Range<Math::Float4x4 const *> transforms, Range<AxisAlignedBox *> transformed_boxes )
    {
        assert( Size( boxes ) == Size( transforms ) );
        assert( Size( boxes ) == Size( transformed_boxes ) );
        auto size = Size( boxes );
        for( size_t i = 0; i < size; i++ )
        {
            transformed_boxes[i] = Transform( boxes[i], transforms[i] );
        }
    }


    AxisAlignedBox TransformByOrientation( AxisAlignedBox box, Orientation const & orientation )
    {
        box.center = Rotate( box.center, orientation.rotation );
        box.center += orientation.position;
        return RotateAroundCenter( box, orientation.rotation );
    }


    std::vector<AxisAlignedBox> TransformByOrientation( Range<AxisAlignedBox const*> const boxes, Range<Orientation const *> const orientations )
    {
        assert( Size(boxes) == Size(orientations) );
        std::vector<AxisAlignedBox> output( Size(boxes) );
        TransformByOrientation(boxes, orientations, output);
        return output;
    }


    void TransformByOrientation( Range<AxisAlignedBox const*> boxes, Range<Orientation const *> orientations, Range<AxisAlignedBox *> output)
    {
        assert(Size(boxes) == Size(orientations));
        assert(Size(boxes) == Size(output));
        for( auto i = 0u; i < Size(boxes); ++i )
        {
            output[i] = TransformByOrientation( boxes[i], orientations[i] );
        }
    }


    AxisAlignedBox Merge( AxisAlignedBox const & box, Range<Orientation const*> const orientations )
    {
        MinMax<Math::Float3> minmax{ std::numeric_limits < float > ::max(), std::numeric_limits<float>::lowest() };

        for( auto i : CreateIntegerRange( Size(orientations) ) )
        {
            auto transformed_box = TransformByOrientation( box, orientations[i] );
            auto local_minmax = GetMinMax( transformed_box );
            minmax.min = Min( minmax.min, local_minmax.min );
            minmax.max = Max( minmax.max, local_minmax.max );
        }

        return CreateAxisAlignedBox( minmax );
    }

    // alternate implementationm not transforming the box but the corners
    //AxisAlignedBox Merge( AxisAlignedBox const & box, Range<Orientation const*> const orientations )
    //{
    //    Math::Float3 minimum = std::numeric_limits < float > ::max(), maximum = std::numeric_limits<float>::lowest();

    //    auto corners = GetCorners( box );

    //    for( auto i : CreateIntegerRange( orientations.size() ) )
    //    {
    //        auto orientation = orientations[i];
    //        for( auto j : CreateIntegerRange( corners.size() ) )
    //        {
    //            auto transformed_corner = Rotate( corners[j], orientation.rotation ) + orientation.position;
    //            minimum = Min( minimum, transformed_corner );
    //            maximum = Max( maximum, transformed_corner );
    //        }
    //    }

    //    return CreateAxisAlignedBox( {minimum, maximum} );
    //}


    namespace
    {
        template<typename IndexIteratorType>
        AxisAlignedBox MergeImpl( Range< AxisAlignedBox const *> boxes, Range<IndexIteratorType> indices )
        {
            MinMax<Math::Float3> minmax{ std::numeric_limits < float > ::max(), std::numeric_limits<float>::lowest() };
            for( auto i : indices )
            {
                auto local_minmax = GetMinMax( boxes[i] );
                minmax.min = Min( minmax.min, local_minmax.min );
                minmax.max = Max( minmax.max, local_minmax.max );
            }
            return CreateAxisAlignedBox( minmax );
        }
    }


    AxisAlignedBox Merge( Range< AxisAlignedBox const *> boxes )
    {
        return MergeImpl( boxes, CreateIntegerRange( Size( boxes ) ) );
    }


    AxisAlignedBox Merge( Range< AxisAlignedBox const *> boxes, Range<uint32_t const *> indices )
    {
        return MergeImpl( boxes, indices );
    }


    MinMax<Math::Float3> GetMinMax( AxisAlignedBox const & box )
    {
        return{ box.center - box.extent, box.center + box.extent };
    }


    std::array<Math::Float3, 8> GetCorners( AxisAlignedBox const & box )
    {
        std::array<Float3, 8> corners;
        auto center = SSE::SSEFromFloat3( box.center );
        auto extent = SSE::SSEFromFloat3( box.extent );
        for( auto i = 0; i < 8; ++i )
        {
            corners[i] = SSE::Float3FromSSE( SSE::Add( center, SSE::Multiply( Details::box_corner_factors_sse[i], extent ) ) );
        }
        return corners;
    }


    std::array<Math::Float3, 4> GetFaceCorners( BoundingShapes::AxisAlignedBox const & box, uint8_t face_index )
    {
        std::array<Float3, 4> corners;
        for( auto i = 0; i < 4; ++i )
        {
            corners[i] = box.center + Details::axis_aligned_face_corner_factors[face_index][i] * box.extent;
        }
        return corners;
    }


    float Volume( AxisAlignedBox const & box )
    {
        auto size = box.extent + box.extent;
        return size.x * size.y * size.z;
    }
}
