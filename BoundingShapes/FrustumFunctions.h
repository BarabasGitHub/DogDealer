#pragma once

#include "Plane.h"

#include <Math\ForwardDeclarations.h>
#include <Utilities\Range.h>

namespace BoundingShapes
{
    void GetFrustumCorners(Math::Float4x4 const & projection_matrix, Range<Math::Float3 *> corners);

    Plane GetNearPlane(Math::Float4x4 const & projection_matrix);
    Plane GetFarPlane(Math::Float4x4 const & projection_matrix);
    Plane GetRightPlane(Math::Float4x4 const & projection_matrix);
    Plane GetLeftPlane(Math::Float4x4 const & projection_matrix);
    Plane GetTopPlane(Math::Float4x4 const & projection_matrix);
    Plane GetBottomPlane(Math::Float4x4 const & projection_matrix);
}