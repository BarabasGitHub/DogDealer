#include "SphereFunctions.h"

#include <Math\FloatOperators.h>
#include <Math\MathFunctions.h>
#include <Math\MathConstants.h>

#include <Utilities\IntegerRange.h>

namespace BoundingShapes
{
    namespace
    {
        template<typename IteratorType>
        Math::Float3 CalculateCenter( Range<IteratorType> indices, Range<Math::Float3 const *> points )
        {
            Math::Float3 center = 0;
            for( auto i : indices )
            {
                center += points[i];
            }
            center /= float( Size(indices) );
            return center;
        }


        template<typename IteratorType>
        Sphere CreateSphereImpl( Range<IteratorType> indices, Range<Math::Float3 const *> points )
        {
            Sphere sphere = { 0, 0 };

            sphere.center = CalculateCenter( indices, points );

            for( auto const index : indices )
            {
                auto const & point = points[index];
                auto square_distance = SquaredNorm( point - sphere.center );
                sphere.radius = Math::Max( square_distance, sphere.radius );
            }

            sphere.radius = Math::Sqrt( sphere.radius );

            return sphere;
        }
    }

    Sphere CreateSphere( Range<Math::Float3 const *> points )
    {
        return CreateSphereImpl( CreateIntegerRange( Size( points ) ), points );
    }

    Sphere CreateSphere( Range<Math::Float3 const *> points, Range<unsigned const *> indices )
    {
        return CreateSphereImpl( indices, points );
    }

    Sphere Transform( Sphere sphere, Math::Float4x4 const & transform )
    {
        return{ Math::TransformPosition( sphere.center, transform ), sphere.radius };
    }


    Sphere TransformByOrientation( Sphere sphere, Orientation const & orientation )
    {
        sphere.center = Math::Rotate( sphere.center, orientation.rotation );
        sphere.center += orientation.position;
        return sphere;
    }


    Sphere Rotate( Sphere sphere, Math::Quaternion const & rotation )
    {
        sphere.center = Math::Rotate( sphere.center, rotation );
        return sphere;
    }


    float Volume(Sphere const & s)
    {
        // 4/3 pi r³
        return 4.f/3.f * Math::c_PI.f * s.radius * s.radius * s.radius;
    }
}