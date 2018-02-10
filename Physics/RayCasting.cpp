#include "RayCasting.h"

#include <BoundingShapes\BoxConversion.h>
#include <BoundingShapes\AxisAlignedBoxFunctions.h>
#include <BoundingShapes\AxisAlignedBoxSSEFunctions.h>
#include <BoundingShapes\RayFunctions.h>

#include <Math\SSE.h>
#include <Math\SSEMathConversions.h>
#include <Math\MathFunctions.h>

#include <limits>

float Physics::IntersectionTime(BoundingShapes::Ray const & ray, DensityFunctionType const & density_function, float accuracy)
{
    auto const max_iterations = 1000;
    assert( accuracy > 0 );
    float density;
    Math::Float3 gradient;
    auto position = ray.start;
    auto direction = ray.direction;
    size_t iteration = 0;
    float time = 0;
    for( density_function(position, density, gradient); Math::Abs(density) > accuracy && iteration < max_iterations; density_function(position, density, gradient) )
    {
        auto a = Dot(gradient, direction);
        auto time_step = (-density / a);
        time += time_step;
        position += time_step * direction;
        iteration += 1;
    }
    if(iteration == max_iterations || time < 0)
    {
        time = std::numeric_limits<float>::infinity();
    }
    return time;
}


Math::Float3 Physics::IntersectionPoint(BoundingShapes::Ray const & ray, DensityFunctionType const & density_function, float accuracy)
{
    assert( accuracy > 0 );
    float density;
    Math::Float3 gradient;
    auto position = ray.start;
    auto direction = ray.direction;
    size_t iteration = 0;
    for( density_function(position, density, gradient); Math::Abs(density) > accuracy && iteration < 1000; density_function(position, density, gradient) )
    {
        auto a = Dot(gradient, direction);
        auto time_step = (-density / a);
        auto step = time_step * direction;
        position += step;
        iteration += 1;
    }

    return position;
}


namespace
{
    using namespace Math::SSE;

    Float32Vector IntersectionTimeImpl(Float32Vector ray_start, Float32Vector ray_direction, Float32Vector box_center, Float32Vector box_extent)
    {
        using namespace Math::SSE;
        auto box_minmax = BoundingShapes::GetMinMax(box_center, box_extent);
        auto rcp_ray_direction = Blend<0,0,0,1>(Reciproce(ray_direction), ZeroFloat32Vector());
        auto t0 = Multiply(Subtract(box_minmax.min, ray_start), rcp_ray_direction);
        auto t1 = Multiply(Subtract(box_minmax.max, ray_start), rcp_ray_direction);
        auto tmin = Min(t0, t1);
        auto tmax = Max(t0, t1);

        // reduce the three min to one common min (in all three positions)
        tmin = Max(Swizzle<1, 2, 0>(tmin), tmin);
        tmin = Max(Swizzle<2, 0, 1>(tmin), tmin);

        // check if all tmax values are larger than the lowest tmin
        auto is_negative = MaskSignBits(Subtract(tmax, tmin));
        if(is_negative)
        {
            // no intersection
            return SetAll(std::numeric_limits<float>::infinity());
        }
        else
        {
            return tmin;
        }
    }
}


float Physics::IntersectionTime(BoundingShapes::Ray const & ray, BoundingShapes::AxisAlignedBox const & box)
{
    using namespace Math::SSE;
    auto box_center = SSEFromFloat3(box.center);
    auto box_extent = SSEFromFloat3(box.extent);
    auto ray_start = SSEFromFloat3(ray.start);
    auto ray_direction = SSEFromFloat3(ray.direction);
    return Math::SSE::GetSingle(IntersectionTimeImpl(ray_start, ray_direction, box_center, box_extent));
}


Math::Float3 Physics::IntersectionPoint(BoundingShapes::Ray const & ray, BoundingShapes::AxisAlignedBox const & box)
{
    using namespace Math::SSE;
    auto box_center = SSEFromFloat3(box.center);
    auto box_extent = SSEFromFloat3(box.extent);
    auto ray_start = SSEFromFloat3(ray.start);
    auto ray_direction = SSEFromFloat3(ray.direction);
    auto tmin = IntersectionTimeImpl(ray_start, ray_direction, box_center, box_extent);
    auto position = MultiplyAdd(ray_direction, tmin, ray_start);
    return Float3FromSSE(position);
}


float Physics::IntersectionTime(BoundingShapes::Ray const & ray, BoundingShapes::OrientedBox const & box)
{
    auto transformed_ray = Rotate(ray, Conjugate(box.rotation));
    return IntersectionTime(ray, AxisAlignedBoxPartOfOrientedBox(box));
}


Math::Float3 Physics::IntersectionPoint(BoundingShapes::Ray const & ray, BoundingShapes::OrientedBox const & box)
{
    auto time = IntersectionTime(ray, box);
    return PointAlongRay(ray, time);
}


float Physics::IntersectionTime(BoundingShapes::Ray const & ray, Range<BoundingShapes::OrientedBox const *> boxes)
{
    auto time = IntersectionTime(ray, First(boxes));
    PopFirst(boxes);
    for(auto const & box : boxes)
    {
        auto new_time = IntersectionTime(ray, box);
        time = Math::Min(new_time, time);
    }
    return time;
}


Math::Float3 Physics::IntersectionPoint(BoundingShapes::Ray const & ray, Range<BoundingShapes::OrientedBox const *> boxes)
{
    auto time = IntersectionTime(ray, boxes);
    return PointAlongRay(ray, time);
}


float Physics::IntersectionTime(BoundingShapes::Ray const & ray, BoundingShapes::Sphere const & sphere)
{
    auto ray_start = ray.start - sphere.center;
    auto center_time = Dot(ray.direction, -ray_start);
    auto closet_point = ray_start + ray.direction * center_time;
    auto distance² = SquaredNorm(closet_point);
    auto extra_time² = distance² - sphere.radius * sphere.radius;
    if(extra_time² < 0)
    {
        return std::numeric_limits<float>::infinity();
    }
    auto extra_time = Math::Sqrt(extra_time²);
    return center_time - extra_time;
}


float Physics::IntersectionTime(BoundingShapes::Ray const & ray, Range<BoundingShapes::Sphere const *> spheres)
{
    auto time = IntersectionTime(ray, First(spheres));
    PopFirst(spheres);
    for(auto const & sphere : spheres)
    {
        auto new_time = IntersectionTime(ray, sphere);
        time = Math::Min(new_time, time);
    }
    return time;
}


Math::Float3 Physics::IntersectionPoint(BoundingShapes::Ray const & ray, Range<BoundingShapes::Sphere const *> spheres)
{
    auto time = IntersectionTime(ray, spheres);
    return PointAlongRay(ray, time);
}
