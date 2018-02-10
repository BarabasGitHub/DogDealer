#pragma once

#include "AxisAlignedBox.h"

#include "Implementation.h"

#include <Conventions\Orientation.h>

#include <Math\FloatTypes.h>
#include <Math\FloatMatrixTypes.h>

#include <Utilities\Range.h>
#include <Utilities\MinMax.h>

#include <vector>
#include <tuple>
#include <array>

namespace BoundingShapes
{

    AxisAlignedBox CreateAxisAlignedBox( MinMax<Math::Float3> minmax );
    AxisAlignedBox CreateAxisAlignedBox( Range<MinMax<Math::Float3> const *> minmaxes );
    AxisAlignedBox CreateAxisAlignedBox( Range<MinMax<Math::Float3> const *> minmaxes, Range<uint32_t const *> indices );
    AxisAlignedBox CreateAxisAlignedBox( Range<Math::Float3 const *> points );
    /// create a box with only the points that are referenced by the indices
    AxisAlignedBox CreateAxisAlignedBox( Range<Math::Float3 const *> points, Range<unsigned const *> indices );

    // rotates the box around its center.
    AxisAlignedBox RotateAroundCenter( AxisAlignedBox box, Math::Quaternion rotation );
    AxisAlignedBox RotateAroundCenter( AxisAlignedBox box, Math::Float3x3 rotation );

    AxisAlignedBox Transform( AxisAlignedBox const & box, Math::Float4x4 const & transform );
    void Transform( Range<AxisAlignedBox const *> boxes, Range<Math::Float4x4 const *> transforms, Range<AxisAlignedBox *> transformed_boxes );
    AxisAlignedBox TransformByOrientation( AxisAlignedBox box, Orientation const & orientation );
    std::vector<AxisAlignedBox> TransformByOrientation( Range<AxisAlignedBox const*> boxes, Range<Orientation const *> orientations );
    void TransformByOrientation( Range<AxisAlignedBox const*> boxes, Range<Orientation const *> orientations, Range<AxisAlignedBox *> transformed_boxes);

    // create one axis aligned bounding box from multiple transformed instances of the input box
    AxisAlignedBox Merge( AxisAlignedBox const & box, Range<Orientation const*> const orientations );
    // create one axis aligned bounding box from multiple boxes
    AxisAlignedBox Merge( Range< AxisAlignedBox const *> boxes );
    AxisAlignedBox Merge( Range< AxisAlignedBox const *> boxes, Range<uint32_t const *> indices );

    MinMax<Math::Float3> GetMinMax( AxisAlignedBox const & box );


    std::array<Math::Float3, 8> GetCorners( AxisAlignedBox const & box );

    std::array<Math::Float3, 6> const & GetAllFaceNormals( AxisAlignedBox );
    std::array<Math::Float3, 6> const & GetAllAxisAlignedFaceNormals();
    Math::Float3 const & GetFaceNormal( BoundingShapes::AxisAlignedBox, uint8_t face_index );
    std::array<Math::Float3, 4> GetFaceCorners( BoundingShapes::AxisAlignedBox const &, uint8_t face_index );

    float Volume( AxisAlignedBox const & box );
}


// implementations
namespace BoundingShapes
{
    inline std::array<Math::Float3, 6> const & GetAllFaceNormals( AxisAlignedBox )
    {
        return Details::axis_aligned_face_normals;
    }


    inline std::array<Math::Float3, 6> const & GetAllAxisAlignedFaceNormals()
    {
        return Details::axis_aligned_face_normals;
    }


    inline Math::Float3 const & GetFaceNormal( BoundingShapes::AxisAlignedBox, uint8_t face_index )
    {
        return Details::axis_aligned_face_normals[face_index];
    }
}
