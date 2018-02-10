#pragma once

#include "OrientedBox.h"

#include "Implementation.h"

#include <Conventions\Orientation.h>

#include <Math\FloatTypes.h>
#include <Math\FloatMatrixTypes.h>

#include <Utilities\Range.h>

#include <vector>
#include <array>

namespace BoundingShapes
{

    OrientedBox CreateOrientedBox( Range<Math::Float3 const *> points );
    OrientedBox ReconstructOrientedBoxFromCorners( Range<Math::Float3 const *> points );


    OrientedBox Rotate( OrientedBox box, Math::Quaternion const & rotation );
    // rotates the box around its center.
    OrientedBox RotateAroundCenter( OrientedBox box, Math::Quaternion const & rotation );
    OrientedBox RotateAroundCenter( OrientedBox box, Math::Float3x3 const & rotation );

    OrientedBox Transform( OrientedBox box, Math::Float4x4 const & transform );
    OrientedBox TransformByOrientation( OrientedBox box, Orientation const & orientation );
    std::vector<OrientedBox> TransformByOrientation( Range<OrientedBox const*> const boxes, Range<Orientation const *> const orientations );

    std::array<Math::Float3, 8> GetCorners( OrientedBox const & box );

    std::array<Math::Float3, 6> GetAllFaceNormals( BoundingShapes::OrientedBox const & box );
    Math::Float3 GetFaceNormal( BoundingShapes::OrientedBox const & box, uint8_t face_index );
    std::array<Math::Float3, 4> GetFaceCorners( BoundingShapes::OrientedBox const &, uint8_t face_index );


    float Volume(OrientedBox const & box);
    // weighted centre, using the volume of the boxes
    Math::Float3 CalculateWeightedCenter(Range<OrientedBox const *> boxes);
    Math::Float3 CalculateWeightedCenter(Range<OrientedBox const *> boxes, float & total_volume);
}

