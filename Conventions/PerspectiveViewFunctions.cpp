#include "PerspectiveViewFunctions.h"

#include <Math\TransformFunctions.h>
#include <Math\FloatMatrixOperators.h>
#include <Math\MathFunctions.h>

Math::Float4x4 PerspectiveFieldOfViewVertical(PerspectiveViewParameters perspective_view_parameters)
{
    return Math::PerspectiveFieldOfViewVertical(perspective_view_parameters.field_of_view, perspective_view_parameters.aspect_ratio, perspective_view_parameters.near_z, perspective_view_parameters.far_z);
}


Math::Float4x4 PerspectiveFieldOfViewHorizontal(PerspectiveViewParameters perspective_view_parameters)
{
    return Math::PerspectiveFieldOfViewHorizontal(perspective_view_parameters.field_of_view, perspective_view_parameters.aspect_ratio, perspective_view_parameters.near_z, perspective_view_parameters.far_z);
}


Math::Float3 TransformVertical(PerspectiveViewParameters params, Math::Float2 screen_coordinates)
{
    auto perspective_transform = PerspectiveFieldOfViewVertical(params);
    return TransformPosition(Math::Float3(screen_coordinates.x, screen_coordinates.y, 0), Inverse(perspective_transform));
}


Math::Float3 TransformHorizontal(PerspectiveViewParameters params, Math::Float2 screen_coordinates)
{
    return TransformPosition(Math::Float3(screen_coordinates.x, screen_coordinates.y, -1), Inverse(PerspectiveFieldOfViewHorizontal(params)));
}
