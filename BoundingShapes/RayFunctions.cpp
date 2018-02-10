#include "RayFunctions.h"

#include <Math/MathFunctions.h>

using namespace BoundingShapes;


Ray BoundingShapes::RayFromStartAndDirection(Math::Float3 start, Math::Float3 direction)
{
    return {start, Normalize(direction)};
}


Ray BoundingShapes::TransformByOrientation( Ray const & ray, Orientation const & orientation )
{
    auto rotated_ray = Rotate(ray, orientation.rotation);
    rotated_ray.start += orientation.position;
    return rotated_ray;
}


Ray BoundingShapes::Rotate( Ray const & ray, Math::Quaternion const & rotation )
{
    return {Rotate(ray.start, rotation), Rotate(ray.direction, rotation)};
}


Math::Float3 BoundingShapes::PointAlongRay(Ray const & ray, float time)
{
    return ray.start + ray.direction * time;
}
