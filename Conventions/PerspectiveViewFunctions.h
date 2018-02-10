#pragma once

#include "PerspectiveViewParameters.h"

#include <Math\FloatTypes.h>
#include <Math\FloatMatrixTypes.h>

Math::Float4x4 PerspectiveFieldOfViewVertical(PerspectiveViewParameters params);
Math::Float4x4 PerspectiveFieldOfViewHorizontal(PerspectiveViewParameters params);

Math::Float3 TransformVertical(PerspectiveViewParameters params, Math::Float2 screen_coordinates);
Math::Float3 TransformHorizontal(PerspectiveViewParameters params, Math::Float2 screen_coordinates);
