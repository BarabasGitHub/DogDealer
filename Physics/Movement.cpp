#include "Movement.h"

#include <Math\FloatOperators.h>
#include <Math\MathFunctions.h>
#include <Math\SSEFloatFunctions.h>
#include <Math\SSELoadStore.h>

using namespace Physics;

bool Physics::Equal( Movement const& a, Movement const& b, float const absolute_tolerance )
{
    return Math::Equal( a.momentum, b.momentum, absolute_tolerance ) && Math::Equal( a.angular_momentum, b.angular_momentum, absolute_tolerance );
}

Movement Physics::operator-(Movement const & a)
{
    using namespace Math::SSE;
    auto a0 = LoadFloat32Vector(begin(a.momentum));
    auto a1 = Load2(begin(a.momentum) + 4);
    a0 = Negate(a0);
    a1 = Negate(a1);
    Movement r;
    Store(a0, begin(r.momentum));
    Store2(a1, begin(r.momentum) + 4);
    return r;
}

namespace
{
    void AddSSE(Movement const & a, Movement const & b, Movement & c)
    {
        using namespace Math::SSE;
        auto a0 = LoadFloat32Vector(begin(a.momentum));
        auto b0 = LoadFloat32Vector(begin(b.momentum));
        auto a1 = Load2(begin(a.momentum) + 4);
        auto b1 = Load2(begin(b.momentum) + 4);
        a0 = Add(a0, b0);
        a1 = Add(a1, b1);
        Store(a0, begin(c.momentum));
        Store2(a1, begin(c.momentum) + 4);
    }
}


Movement Physics::operator+(Movement const & a, Movement const & b)
{
    Movement r;
    AddSSE(a, b, r);
    return r;
}


Movement& Physics::operator+=(Movement & a, Movement const & b)
{
    AddSSE(a, b, a);
    return a;
}


Movement Physics::operator-(Movement const & a, Movement const & b)
{
    Movement r;
    r.momentum = a.momentum - b.momentum;
    r.angular_momentum = a.angular_momentum - b.angular_momentum;
    return r;
}


Movement& Physics::operator-=(Movement & a, Movement const & b)
{
    return a = a - b;
}


Movement Physics::operator*(float a, Movement const & b)
{
    using namespace Math::SSE;
    auto s = SetAll(a);
    auto b0 = LoadFloat32Vector(begin(b.momentum));
    auto b1 = Load2(begin(b.momentum) + 4);
    b0 = Multiply(s, b0);
    b1 = Multiply(s, b1);
    Movement r;
    Store(b0, begin(r.momentum));
    Store2(b1, begin(r.momentum) + 4);
    return r;
}


Movement Physics::operator*(Movement const & a, float b)
{
    return b * a;
}


Movement& Physics::operator*=(Movement & a, float b)
{
    return a = a * b;
}