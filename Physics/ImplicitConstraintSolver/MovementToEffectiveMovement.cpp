#include "MovementToEffectiveMovement.h"
#include "..\Inertia.h"
#include "..\Movement.h"

#include <Math\SSELoadStore.h>
#include <Math\SSEFloatFunctions.h>
#include <Math\SSEMathConversions.h>
#include <Math\SSEMatrixFunctions.h>
#include <Math\MathFunctions.h>

using namespace Physics;

MovementToEffectiveMovement Physics::operator/(MovementToEffectiveMovement const & a, float b)
{
    using namespace Math::SSE;
    auto factor = SetAll(b);
    auto part0 = LoadFloat32Vector(begin(a.linear));
    part0 = Divide(part0, factor);
    auto part1 = Load2(begin(a.linear) + 4);
    part1 = Divide(part1, factor);
    MovementToEffectiveMovement r;
    Store(part0, begin(r.linear));
    Store2(part1, begin(r.linear) + 4);
    // r.linear = a.linear / b;
    // r.angular = a.angular / b;
    return r;
}


MovementToEffectiveMovement Physics::CalculateMovementToEffectiveMovement(Math::Float3 const & direction_in, Math::Float3 const & position_in, Inertia const & inverse_inertia, float inverse_effective_mass)
{
    using namespace Math::SSE;
    auto factor = SetAll(inverse_effective_mass);
    auto direction = SSEFromFloat3(direction_in);
    auto position = SSEFromFloat3(position_in);
    MovementToEffectiveMovement result;
    result.linear = Float3FromSSE(Divide(Multiply(direction, SetAll(inverse_inertia.mass)), factor));
    auto moment = SSEFromFloat3x3(inverse_inertia.moment);
    result.angular = Float3FromSSE(Divide(Multiply3D(Cross(position, direction), moment), factor));
    return result;
}



float Physics::CalculateEffectiveMovement(Movement const & movement, MovementToEffectiveMovement const & movement_to_effective_movement)
{
    return Dot(movement.momentum, movement_to_effective_movement.linear) + Dot(movement.angular_momentum, movement_to_effective_movement.angular);
}


    // MovementToSpeed CalculateMovementToSpeed(Math::Float3 const & direction, Math::Float3 const & position, Inertia const & inverse_inertia)
    // {
    //     MovementToSpeed result;
    //     result.linear = direction * inverse_inertia.mass;
    //     result.angular = Cross(position, direction) * inverse_inertia.moment;
    //     return result;
    // }

    // MovementToSpeed CalculateMovementToSpeed(Math::Float3 const & direction_in, Math::Float3 const & position_in, Inertia const & inverse_inertia)
    // {
    //     using namespace Math::SSE;
    //     auto direction = SSEFromFloat3(direction_in);
    //     auto position = SSEFromFloat3(position_in);
    //     MovementToSpeed result;
    //     result.linear = Float3FromSSE(Multiply(direction, LoadAll(inverse_inertia.mass)));
    //     auto moment = SSEFromFloat3x3(inverse_inertia.moment);
    //     result.angular = Float3FromSSE(Multiply3D(Cross(position, direction), moment));
    //     return result;
    // }
