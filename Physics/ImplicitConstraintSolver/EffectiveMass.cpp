#include "EffectiveMass.h"

#include "..\Movement.h"
#include "..\Inertia.h"

#include <Math\SSEMathConversions.h>
#include <Math\SSEMatrixFunctions.h>
#include <Math\SSEFloatFunctions.h>
#include <Math\MathFunctions.h>

using namespace Physics;

float Physics::CalculateInverseEffectiveMass(Movement const & movement, Inertia const & inverse_inertia)
{
    using namespace Math::SSE;
    auto linear = SquaredNorm(movement.momentum) * inverse_inertia.mass;
    auto moment = SSEFromFloat3x3(inverse_inertia.moment);
    auto angular_momentum = SSEFromFloat3(movement.angular_momentum);
    auto angular = Dot3Single(Multiply3D(angular_momentum, moment), angular_momentum);
    return linear + GetSingle(angular);
}


float Physics::CalculateInverseEffectiveMass(Movement const & movement0, Movement const & movement1, Inertia const & inverse_inertia0, Inertia const & inverse_inertia1)
{
    auto factor0 = CalculateInverseEffectiveMass(movement0, inverse_inertia0);
    auto factor1 = CalculateInverseEffectiveMass(movement1, inverse_inertia1);
    return factor0 + factor1;
}

