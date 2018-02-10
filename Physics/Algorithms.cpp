#include "Algorithms.h"
#include "Movement.h"
#include "Inertia.h"
#include "BodyID.h"
#include "BodyEntityMappingFunctions.h"

#include <Conventions\Velocity.h>
#include <Math\MathFunctions.h>
#include <Math\SSEFloatFunctions.h>
#include <Math\SSEMatrixFunctions.h>
#include <Math\SSEMathConversions.h>
#include <Math\TransformFunctions.h>
#include <Utilities\VectorHelper.h>
#include <Utilities\IndexedHelp.h>
#include <Utilities\IntegerRange.h>


using namespace Math;
using namespace Physics;

namespace
{
    Orientation UpdateOrientation(
        Orientation orientation, 
        Movement const & movement, 
        Inertia const & inverse_inertia, 
        const float time_step
        )
    {
        auto const velocity = movement.momentum * inverse_inertia.mass;
        auto const angular_velocity = inverse_inertia.moment * movement.angular_momentum;

        orientation.position += velocity * time_step;
        orientation.rotation = UpdateRotationWithAngularVelocity(orientation.rotation, angular_velocity, time_step);

        return orientation;
    }
}


Quaternion Physics::UpdateRotationWithAngularVelocity(
    Quaternion const & rotation, 
    Float3 const & angular_velocity, 
    float const time_step
    )
{
    auto norm = Norm(angular_velocity);
    if(norm != 0)
    {
        auto angle = norm * time_step;
        return NormalAndAngleToQuaternion(angular_velocity / norm, angle) * rotation;
    }
    else
    {
        return rotation;
    }
    //auto spin = (Quaternion(angular_velocity, 0) * rotation);
    //return Normalize(rotation + rotation + spin * time_step);
}


Quaternion Physics::UpdateQuaternionWithAngularVelocity(
    Quaternion const & quaternion,
    Float3 const & angular_velocity,
    float const time_step
)
{
    auto spin = (Quaternion(angular_velocity, 0) * quaternion);
    return (quaternion + spin * time_step / 2);
}


void Physics::AddGravity(
    Math::Float3 const gravity, 
    Range<Inertia const *> inverse_inertias, 
    Range<Math::Float3 * > forces
    )
{
    assert(Size(inverse_inertias) == Size(forces));
    for(auto i = 0u; i < Size(inverse_inertias); ++i)
    {
        forces[i] += gravity / inverse_inertias[i].mass;
    }
}


void Physics::AddForces(
    EntityForces const & entity_forces, 
    BodyEntityMapping const & mapping, 
    Range<uint32_t const *> indices, 
    Range<Math::Float3 * > forces
    )
{
    assert(Size(entity_forces.entity_ids) == Size(entity_forces.forces));

    for(auto i = 0u; i < Size(entity_forces.entity_ids); ++i)
    {
        auto entity = entity_forces.entity_ids[i];
        // TODO: First or all?
        auto body = First(Bodies(entity, mapping));
        auto index = body.index;
        if(index < Size(indices))
        {
            index = indices[index];
            if(index < Size(forces))
            {
                forces[index] += entity_forces.forces[i];
            }
        }
    }
}


void Physics::ApplyMovement(
    Range<Orientation*> new_orientation, 
    Range<Orientation const *> original_orientation, 
    Range<Movement const*> movement, 
    Range<Inertia const*> inverse_inertia, 
    float const time_step
    )
{
    // I assume the ranges have the same length
    assert(Size(new_orientation) == Size(original_orientation));
    assert(Size(new_orientation) == Size(movement));
    assert(Size(new_orientation) == Size(inverse_inertia));

    for(auto i = 0u; i < Size(new_orientation); ++i)
    {
        new_orientation[i] = UpdateOrientation(original_orientation[i], movement[i], inverse_inertia[i], time_step);
    }
}


void Physics::UpdateOrientations(
    Range<Orientation*> const orientation_range, 
    Range<uint32_t const*> const indices, 
    Range<Math::Float3 const *> const new_entity_positions, 
    Range<EntityID const*> const position_entity_ids
    )
{
    assert(Size(new_entity_positions) == Size(position_entity_ids));

    for(auto i : CreateIntegerRange(Size(new_entity_positions)))
    {
        auto const entity_id = position_entity_ids[i];
        if(entity_id.index < Size(indices))
        {
            auto const index = indices[entity_id.index];
            if(index < Size(orientation_range))
            {
                orientation_range[index].position = new_entity_positions[i];
            }
        }
    }
}


void Physics::UpdateOrientations(
    Range<Orientation*> const orientation_range, 
    Range<uint32_t const*> const body_to_element,
    BodyEntityMapping const & mapping,
    Range<Math::Float3 const *> const new_entity_positions, 
    Range<EntityID const*> const position_entity_ids
    )
{
    assert(Size(new_entity_positions) == Size(position_entity_ids));

    for(auto i : CreateIntegerRange(Size(new_entity_positions)))
    {
        auto const entity_id = position_entity_ids[i];
        auto body_id = First(Bodies(entity_id, mapping));
        if(body_id.index < Size(body_to_element))
        {
            auto const index = body_to_element[body_id.index];
            if(index < Size(orientation_range))
            {
                orientation_range[index].position = new_entity_positions[i];
            }
        }
    }
}


void Physics::UpdateOrientations(
    Range<Orientation*> const orientation_range, 
    Range<uint32_t const*> const body_to_element, 
    BodyEntityMapping const & mapping,
    Range<Math::Quaternion const *> const new_entity_rotations, 
    Range<EntityID const*> const entity_ids
    )
{
    assert(Size(new_entity_rotations) == Size(entity_ids));

    for(auto i : CreateIntegerRange(Size(new_entity_rotations)))
    {
        auto const entity_id = entity_ids[i];
        auto body_id = First(Bodies(entity_id, mapping));
        if(body_id.index < Size(body_to_element))
        {
            auto const index = body_to_element[body_id.index];

            if(index < Size(orientation_range))
            {
                orientation_range[index].rotation = new_entity_rotations[i];
            }
        }
    }
}



void Physics::UpdateOrientations(
    Range<Orientation*> const orientation_range, 
    Range<uint32_t const*> const indices, 
    Range<Math::Quaternion const *> const new_entity_rotations, 
    Range<EntityID const*> const entity_ids
    )
{
    assert(Size(new_entity_rotations) == Size(entity_ids));

    for(auto i : CreateIntegerRange(Size(new_entity_rotations)))
    {
        auto const entity_id = entity_ids[i];

        if(entity_id.index < Size(indices))
        {
            auto const index = indices[entity_id.index];

            if(index < Size(orientation_range))
            {
                orientation_range[index].rotation = new_entity_rotations[i];
            }
        }
    }
}


void Physics::CorrectForCenterOfMassBackward(
    Range<Math::Float3 const *> center_of_mass, 
    Range<Orientation *> orientations
    )
{
    assert(Size(center_of_mass) == Size(orientations));
    if(IsEmpty(center_of_mass)) return;
    std::transform(begin(center_of_mass), end(center_of_mass), begin(orientations), begin(orientations), []
                   (Math::Float3 center_of_mass, Orientation orientation)
    {
        orientation.position -= Rotate(center_of_mass, orientation.rotation);
        return orientation;
    });
}


void Physics::CorrectForCenterOfMassForward(
    Range<Math::Float3 const *> center_of_mass, 
    Range<Orientation *> orientations
    )
{
    assert(Size(center_of_mass) == Size(orientations));
    if(IsEmpty(center_of_mass)) return;
    std::transform(begin(center_of_mass), end(center_of_mass), begin(orientations), begin(orientations), []
                   (Math::Float3 center_of_mass, Orientation orientation)
    {
        orientation.position += Rotate(center_of_mass, orientation.rotation);
        return orientation;
    });
}


void Physics::CorrectForCenterOfMassForward(
    Range<EntityID const *> ids, 
    Range<uint32_t const *> entity_to_data, 
    Range<Math::Float3 const *> center_of_mass, 
    Range<Orientation const *> orientation, 
    Range<Math::Float3 *> position
    )
{
    assert(Size(ids) == Size(position));
    assert(Size(center_of_mass) == Size(orientation));

    for(auto i = 0u; i < Size(ids); ++i)
    {
        auto index = entity_to_data[ids[i].index];
        position[i] += Rotate(center_of_mass[index], orientation[index].rotation);
    }
}


void Physics::ApplyForces(
    Range<Movement const *> movements, 
    Range<Force const *> forces, 
    float const time_step, 
    Range<Movement *> result_movements
    )
{
    assert(Size(movements) == Size(forces));
    assert(Size(movements) == Size(result_movements));
    for(auto i = 0u; i < Size(movements); ++i)
    {
        auto movement = movements[i];
        movement.momentum += forces[i] * time_step;
        result_movements[i] = movement;
    }
}


std::vector<Movement> Physics::ApplyForces(
    std::vector<Movement> movements, 
    std::vector<uint32_t> const & indices, 
    std::vector<Force> const & forces, 
    std::vector<EntityID> const & entity_ids, 
    float const time_step
    )
{
    assert(forces.size() == entity_ids.size());

    auto const size = forces.size();
    for(size_t i = 0u; i < size; ++i)
    {
        auto const entity_id = entity_ids[i];
        auto const movement_index = indices[entity_id.index];

        movements[movement_index].momentum += forces[i] * time_step;
    }

    return movements;
}


void Physics::ApplyTorques(
    Range<Movement const *> movements, 
    Range<uint32_t const *> indices, 
    Range<Torque const *> torques, 
    Range<EntityID const *> entity_ids, 
    BodyEntityMapping const & mapping,
    float const time_step, 
    Range<Movement *> result_movements
    )
{
    assert(Size(torques) == Size(entity_ids));
    assert(Size(movements) == Size(result_movements));

    for(auto i = 0u; i < Size(entity_ids); ++i)
    {
        auto entity_id = entity_ids[i];
        // TODO: First or all?
        auto index = First(Bodies(entity_id, mapping)).index;
        auto movement_index = indices[index];
        auto movement = movements[movement_index];
        movement.angular_momentum += torques[i] * time_step;
        result_movements[movement_index] = movement;
    }
}

Math::Float3 Physics::AngularVelocityFromRotation(
    Math::Quaternion new_rotation, 
    Math::Quaternion original_rotation, 
    float time_step
    )
{
    auto diff = new_rotation * Conjugate(original_rotation);
    return ConditionalNormalize(GetAxis(diff)) * (GetAngle(diff) / time_step);
}




void Physics::UpdateVelocities(
    std::vector<Velocity>& velocities,
    std::vector<Math::Float3>& angular_velocities,
    std::vector<EntityID> & velocity_entity_ids,
    std::vector<uint32_t>& entity_to_element,
    Range<Orientation const * > orientations,
    Range<Orientation const * > previous_orientations,
    Range<EntityID const * > entity_ids,
    const float time_step
    )
{
    // Assuming orientations are in sync with entity_ids
    assert(Size(orientations) == Size(previous_orientations));
    assert(Size(orientations) == Size(entity_ids));

    for(auto i : CreateIntegerRange(Size(orientations)))
    {
        // Compare current and previous rotation
        auto offset = orientations[i].position - previous_orientations[i].position;
        // or - ?
        auto const current_rotation = orientations[i].rotation;
        auto const previous_rotation = previous_orientations[i].rotation;

        // Ignore if no movement
        if(Math::Equal(offset, Float3{0}, 0.001f) &&
           Math::Equal(GetAngle(current_rotation), GetAngle(previous_rotation), 0.001f))
        {
            continue;
        }

        auto const index = uint32_t(velocities.size());
        auto entity_id = entity_ids[i];

        AddIndexToIndices(entity_to_element, entity_id.index, index);

        velocity_entity_ids.push_back(entity_id);

        Velocity velocity = offset / time_step;
        velocities.push_back(velocity);

        auto angular_velocity = AngularVelocityFromRotation(current_rotation, previous_rotation, time_step);
        angular_velocities.push_back(angular_velocity);

    }
}


void Physics::UpdateVelocities(
    std::vector<Velocity>& velocities, 
    std::vector<Math::Float3>& angular_velocities, 
    std::vector<EntityID> & velocity_entity_ids, 
    std::vector<uint32_t>& entity_to_element, 
    Range<Orientation const * > orientations, 
    Range<Orientation const * > previous_orientations, 
    Range<BodyID const * > body_ids, 
    BodyEntityMapping const & mapping,
    const float time_step
    )
{
    // Assuming orientations are in sync with entity_ids
    assert(Size(orientations) == Size(previous_orientations));
    assert(Size(orientations) == Size(body_ids));

    for(auto i : CreateIntegerRange(Size(orientations)))
    {
        // Compare current and previous rotation
        auto offset = orientations[i].position - previous_orientations[i].position;
        // or - ?
        auto const current_rotation = orientations[i].rotation;
        auto const previous_rotation = previous_orientations[i].rotation;

        // Ignore if no movement
        if(Math::Equal(offset, Float3{0}, 0.001f) &&
           Math::Equal(GetAngle(current_rotation), GetAngle(previous_rotation), 0.001f))
        {
            continue;
        }

        auto const index = uint32_t(velocities.size());
        auto const entity_id = Entity(body_ids[i], mapping);
        // only process the first body
        if(First(Bodies(entity_id, mapping)) == body_ids[i])
        {
            AddIndexToIndices(entity_to_element, entity_id.index, index);

            velocity_entity_ids.push_back(entity_id);

            Velocity velocity = offset / time_step;
            velocities.push_back(velocity);

            auto angular_velocity = AngularVelocityFromRotation(current_rotation, previous_rotation, time_step);
            angular_velocities.push_back(angular_velocity);
        }
    }
}


void Physics::UpdateVelocities(
    std::vector<Velocity>& velocities,
    std::vector<Math::Float3>& angular_velocities,
    std::vector<EntityID> & velocity_entity_ids,
    std::vector<uint32_t>& entity_to_elements,
    Range<Movement const*> movements,
    Range<Inertia const *> inverse_inertia,
    Range<BodyID const *> body_ids,
    BodyEntityMapping const & mapping
    )
{
    for(auto i = 0u; i < Size(movements); ++i)
    {
        auto const & movement = movements[i];
        if(Math::Equal(movement.momentum, Float3{0}, 0.001f) &&
           Math::Equal(movement.angular_momentum, Float3{0}, 0.001f))
        {
            continue;
        }

        auto const index = uint32_t(velocities.size());
        auto const entity_id = Entity(body_ids[i], mapping);
        
        // only process the first body
        if(First(Bodies(entity_id, mapping)) == body_ids[i])
        {
            AddIndexToIndices(entity_to_elements, entity_id.index, index);

            velocity_entity_ids.push_back(entity_id);

            velocities.emplace_back(movement.momentum * inverse_inertia[i].mass);
            angular_velocities.emplace_back(movement.angular_momentum * inverse_inertia[i].moment);
        }
    }
}


Math::Float3 Physics::ImpulseFromMovement(
    Movement const & movement, 
    Math::Float3 const & position
    )
{
    // return movement.momentum + Cross(movement.angular_momentum, position);
    using namespace Math::SSE;
    auto p = SSEFromFloat3(position);
    auto momentum = LoadFloat32Vector(begin(movement.momentum));
    auto angular_momentum = SSEFromFloat3(movement.angular_momentum);
    return Float3FromSSE(SSE::Add(momentum, Cross(angular_momentum, p)));
}


Movement Physics::MovementFromImpulse(
    Math::Float3 const impulse, 
    Math::Float3 const position
    )
{
    return{impulse, Cross(position, impulse)};
}


Movement Physics::ApplyImpulse(
    Movement movement, 
    Math::Float3 const impulse, 
    Math::Float3 const position
    )
{
    movement.momentum += impulse;
    movement.angular_momentum += Cross(position, impulse);
    return movement;
}


void Physics::AddMovements(
    Range<Movement const *> movements1, 
    Range<Movement *> movements2, 
    float const angular_fraction
    )
{
    assert(Size(movements1) == Size(movements2));
    if(IsEmpty(movements1)) return;
    std::transform(begin(movements2), end(movements2), begin(movements1), begin(movements2), [angular_fraction](Movement a, Movement b)
    {
        a.momentum += b.momentum;
        a.angular_momentum = a.angular_momentum * angular_fraction + b.angular_momentum;
        return a;
    });
}


// position has to be in body coordinates
// Math::Float3 CalculateVelocity( Movement const & movement, Inertia const & inverse_inertia, Math::Float3 const position )
// {
//     auto linear = inverse_inertia.mass * movement.momentum; // [m s-¹]
//     auto angular_velocity = inverse_inertia.moment * movement.angular_momentum; // [s-¹]
//     auto rotational = Cross(angular_velocity, position); // [m s-¹]
//     return linear + rotational;
// }

// position has to be in body coordinates
Math::Float3 Physics::CalculateVelocity(
    Movement const & movement, 
    Inertia const & inverse_inertia, 
    Math::Float3 const position
    )
{
    using namespace Math;
    using namespace SSE;
    auto linear = Multiply(SetAll(inverse_inertia.mass), SSEFromFloat3(movement.momentum)); // [m s-¹]
    FloatMatrix moment = SSEFromFloat3x3(inverse_inertia.moment);
    auto angular_velocity = Multiply3D(moment, SSEFromFloat3(movement.angular_momentum)); // [s-¹]
    auto rotational = Cross(angular_velocity, SSEFromFloat3(position)); // [m s-¹]
    return Float3FromSSE(SSE::Add(linear, rotational));
}


float Physics::CombineRestitutionFactors(
    float factor1, 
    float factor2
    )
{
    return Math::Sqrt(factor1 * factor2);
}


float Physics::CalculateInverseEffectiveMass(
    Math::Float3 relative_position, 
    Math::Float3 normal, 
    Inertia const & inverse_inertia
    )
{
    auto torque = Cross(relative_position, normal); // [m]
    auto angular_component = inverse_inertia.moment * torque; // [kg-¹ m-¹]
    // triple scalar product
    //auto angular_velocity = Cross( angular_component, relative_position );
    //auto angular_inertia_factor = Dot( normal, angular_velocity );
    // can also write as Dot( angular_component, Cross( relative_position, normal ) ) -> Dot( angular_component, torque)
    auto angular_inertia_factor = Dot(angular_component, torque); // [kg-¹]
    return angular_inertia_factor + inverse_inertia.mass; // [kg-¹]
}
