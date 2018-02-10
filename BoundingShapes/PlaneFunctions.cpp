#include "PlaneFunctions.h"

#include <Math\MathFunctions.h>

BoundingShapes::Plane BoundingShapes::CreatePlane( Math::Float4 plane_equation )
{
    return{ { plane_equation.x, plane_equation.y, plane_equation.z }, plane_equation.w };
}


BoundingShapes::Plane BoundingShapes::CreatePlane( Math::Float3 normal, Math::Float3 point )
{
    return{ normal, -Dot( point, normal ) };
}


float BoundingShapes::Distance( Plane const & plane, Math::Float3 point )
{
    return Dot( plane.normal, point ) + plane.distance;
}


BoundingShapes::Plane BoundingShapes::Lerp(Plane a, Plane b, float factor)
{
    static_assert(sizeof(Plane) == sizeof(Math::Float4), "A plane cannot be reinterpreted as a Float4.");
    return CreatePlane(Lerp(reinterpret_cast<Math::Float4 const&>(a), reinterpret_cast<Math::Float4 const&>(b), factor));
}


BoundingShapes::Plane BoundingShapes::Normalize(Plane p)
{
    auto norm = Norm(p.normal);
    p.normal /= norm;
    p.distance /= norm;
    return p;
}


BoundingShapes::Plane BoundingShapes::Transform(Plane p, Math::Float4x4 const & transform_matrix)
{
    // static_assert(sizeof(Plane) == sizeof(Math::Float4), "A plane cannot be reinterpreted as a Float4.");
    // return CreatePlane()
    auto normal = TransformNormal(p.normal, transform_matrix);
    auto point = TransformPosition(p.normal * p.distance, transform_matrix);
    return CreatePlane(normal, point);
}


Math::Float4 BoundingShapes::Float4FromPlane(Plane const & a)
{
    return reinterpret_cast<Math::Float4 const&>(a);
}