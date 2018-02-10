#include "FrictionAlgorithms.h"

#include "Movement.h"
#include "Inertia.h"

#include <Math\MathFunctions.h>
#include <Math\VectorAlgorithms.h>

#include <Utilities\IntegerRange.h>

#include <algorithm>


namespace Physics
{

    namespace
    {
        Math::Float3 LimitImpulse( 
            Math::Float3 impulse, 
            Math::Float3 const & momentum 
            )
        {
            return CopySign( Min( Abs( impulse ), Abs( momentum ) ), impulse );
        }

        // does not take into account any friction coefficient!
        std::pair<Math::Float3, Math::Float3> CalculateMaximumContactForceAndTorque( 
            Math::Float3 const normal, 
            Math::Float3 const momentum, 
            Math::Float3 angular_momentum 
            )
        {
            // get the amount of force from the entity (id) on the plane (normal)
            auto const dot = Dot( normal, momentum );
            if( dot > 0 ) return{ 0.f, 0.f };
            // get the direction of the friction force, perpendicular to the plane normal, in the direction of the movement
            auto const direction = Normalize( dot * normal - momentum );
            // calculate force in the direction scaled by the friction coefficient
            auto const scale = -dot;
            auto const angular_scale = Dot( angular_momentum, normal ) > 0 ? -scale : scale;
            return{ direction * scale, normal * angular_scale };
        }


        Math::Float3 ScaleImpulse( 
            Math::Float3 impulse, 
            Math::Quaternion rotation, 
            Math::Float3 scale 
            )
        {
            impulse = Math::InverseRotate( impulse, rotation );
            impulse *= scale;
            impulse = Math::Rotate( impulse, rotation );
            return impulse;
        }
    }


    void ApplyDrag( 
        Range<Movement *> movements, 
        Range<Inertia const *> inverse_inertias, 
        Range<BoundingShapes::AxisAlignedBox const *> boxes, 
        float const time_step 
        )
    {
        assert( Size(movements) == Size(inverse_inertias) );
        auto const air_density = 1.2041f;
        const auto drag_coefficient = 0.8f;

        for( auto const i : CreateIntegerRange( Size(movements) ) )
        {
            auto size = boxes[i].extent;
            auto surface_area = std::pow( size.x * size.y * size.z, 2.f / 3 );
            auto inverse_mass = inverse_inertias[i].mass;
            auto momentum = movements[i].momentum;
            // see http://en.wikipedia.org/wiki/Drag_%28physics%29
            auto drag_factors_less_speed = .5f * air_density * drag_coefficient * surface_area;
            // solve the differential equation:
            //  dv    = -(c/m)v²dt
            //  dv/v² = -(c/m)  dt
            // integrate both sides
            //  -1/v  = -(c/m)t + K
            // calculate t1 - t0
            // 1/v0 -1/v1 = (c/m)(t0 - t1)
            // -1/v1 = (c/m)(t0 - t1) - 1/v0
            // 1/v1  = (c/m)(t1 - t0) + 1/v0
            // v1 = 1/((c/m)(t1 - t0) + 1/v0)
            // v1 = v0 / ((v0 c/m)(t1 - t0) + 1)
            // M1 = M0 / (M0 Δt c/m² + 1)
            momentum /= Norm(momentum) * time_step * drag_factors_less_speed * inverse_mass * inverse_mass + 1;
            // movements[i].momentum += impulse;
            movements[i].momentum = momentum;
        }
    }


    void ApplyInternalFriction( 
        Range<Movement const *> movements, 
        float const fraction_per_second, 
        float const time_step, 
        Range<Movement *> result_movements 
        )
    {
        auto coefficient = std::pow(fraction_per_second, time_step);

        auto in = ReinterpretRange<float const>( movements );
        auto out = ReinterpretRange<float>(result_movements );
        Math::Multiply( in, coefficient, out );
    }


    float CombineFrictionFactors( 
        float f1, 
        float f2 
        )
    {
        return Math::Sqrt(f1 * f2);
    }
}