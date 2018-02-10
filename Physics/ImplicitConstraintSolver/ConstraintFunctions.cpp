#include "ConstraintFunctions.h"

#include "Constraints.h"
#include "MovementToEffectiveMovement.h"
#include "EffectiveMass.h"
#include "..\Algorithms.h"
#include "..\Inertia.h"
#include "..\FrictionAlgorithms.h"

#include <Math\MathFunctions.h>
#include <Math\TransformFunctions.h>

using namespace Physics;


void Physics::CalculateNonPenetrationConstraints(
    Range<BodyPair const *> bodies,
    Range<Math::Float3 const *> collision_normals,
    Range<std::array<Math::Float3, 2> const * > relative_positions,
    Range<float const *> penetration_depths,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Inertia const *> inverse_inertias,
    float const position_correction_coefficient,
    float const penetration_depth_tolerance,
    float const time_step,
    float const warm_starting_factor,
    Range<Constraint *> constraints)
{
    assert(Size(bodies) == Size(collision_normals));
    assert(Size(bodies) == Size(relative_positions));
    assert(Size(bodies) == Size(penetration_depths));
    assert(Size(bodies) == Size(constraints));

    auto count = Size(bodies);
    for( auto i = 0u; i < count; ++i )
    {
        auto body_pair = bodies[i];

        Constraint c;
        c.body_indices = {{rigid_body_to_element[body_pair.id1.index], rigid_body_to_element[body_pair.id2.index]}};

        auto direction = collision_normals[i];
        auto inverse_inertia0 = inverse_inertias[c.body_indices[0]];
        auto inverse_inertia1 = inverse_inertias[c.body_indices[1]];
        auto movement0 = c.unit_impulses[0] = MovementFromImpulse(direction, relative_positions[i][0]);
        auto movement1 = MovementFromImpulse(direction, relative_positions[i][1]);
        c.unit_impulses[1] = -movement1;

        auto inverse_effective_mass = CalculateInverseEffectiveMass(movement0, movement1, inverse_inertia0, inverse_inertia1);
        c.movement_to_effective_movement[0] = CalculateMovementToEffectiveMovement(collision_normals[i], relative_positions[i][0], inverse_inertia0, inverse_effective_mass);
        c.movement_to_effective_movement[1] = CalculateMovementToEffectiveMovement(collision_normals[i], relative_positions[i][1], inverse_inertia1, -inverse_effective_mass);

        auto distance_to_correct = (penetration_depths[i] - penetration_depth_tolerance) * position_correction_coefficient;
        c.target_impulse = distance_to_correct / (time_step * inverse_effective_mass);
        c.impulse_limits = {0, std::numeric_limits<float>::max()};

        c.total_impulse = c.target_impulse * warm_starting_factor;

        c.feedback_coefficient = 0;

        constraints[i] = c;
    }
}


// Assumes only the first body is of interest
void Physics::CalculateNonPenetrationConstraints(
    Range<BodyPair const *> bodies,
    Range<Math::Float3 const *> collision_normals,
    Range<std::array<Math::Float3, 2> const * > relative_positions,
    Range<float const*> penetration_depths,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Inertia const *> inverse_inertias,
    float const position_correction_coefficient,
    float const penetration_depth_tolerance,
    float const time_step,
    float const warm_starting_factor,
    Range<SingleBodyConstraint *> constraints)
{
    assert(Size(bodies) == Size(collision_normals));
    assert(Size(bodies) == Size(relative_positions));
    assert(Size(bodies) == Size(penetration_depths));
    assert(Size(bodies) == Size(constraints));

    auto count = Size(bodies);
    for( auto i = 0u; i < count; ++i )
    {
        auto body_pair = bodies[i];

        SingleBodyConstraint c;
        c.body_index = rigid_body_to_element[body_pair.id1.index];

        auto direction = collision_normals[i];
        auto relative_position = relative_positions[i][0];
        auto inverse_inertia0 = inverse_inertias[c.body_index];
        auto movement0 = c.unit_impulse = MovementFromImpulse(direction, relative_position);
        auto inverse_effective_mass = CalculateInverseEffectiveMass(movement0, inverse_inertia0);
        c.movement_to_effective_movement = CalculateMovementToEffectiveMovement(direction, relative_position, inverse_inertia0, inverse_effective_mass);

        auto distance_to_correct = (penetration_depths[i] - penetration_depth_tolerance) * position_correction_coefficient;
        c.target_impulse = distance_to_correct / (time_step * inverse_effective_mass);
        c.impulse_limits = {0, std::numeric_limits<float>::max()};

        c.total_impulse = c.target_impulse * warm_starting_factor;

        c.feedback_coefficient = 0;

        constraints[i] = c;
    }
}


void Physics::CalculateContactConstraints(
    Range<BodyPair const *> bodies,
    Range<Math::Float3 const *> collision_normals,
    Range<std::array<Math::Float3, 2> const * > relative_positions,
    Range<Math::Float3 const *> old_relative_velocities,
    Range<Math::Float3 const *> new_relative_velocities,
    Range<uint32_t const *> body_to_restitution,
    Range<float const *> restitution_factors,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Inertia const *> inverse_inertias,
    float const warm_starting_factor,
    Range<Constraint *> constraints)
{
    assert(Size(bodies) == Size(collision_normals));
    assert(Size(bodies) == Size(relative_positions));
    assert(Size(bodies) == Size(old_relative_velocities));
    assert(Size(bodies) == Size(new_relative_velocities));
    assert(Size(bodies) == Size(constraints));

    auto count = Size(constraints);
    for( auto i = 0u; i < count; ++i )
    {
        auto body_pair = bodies[i];

        Constraint c;
        c.body_indices = {{rigid_body_to_element[body_pair.id1.index], rigid_body_to_element[body_pair.id2.index]}};
        auto inverse_inertia0 = inverse_inertias[c.body_indices[0]];
        auto inverse_inertia1 = inverse_inertias[c.body_indices[1]];
        auto direction = collision_normals[i];
        auto movement0 = c.unit_impulses[0] = MovementFromImpulse(direction, relative_positions[i][0]);
        auto movement1 = MovementFromImpulse(direction, relative_positions[i][1]);
        c.unit_impulses[1] = -movement1;
        auto inverse_effective_mass = CalculateInverseEffectiveMass(movement0, movement1, inverse_inertia0, inverse_inertia1);
        c.movement_to_effective_movement[0] = CalculateMovementToEffectiveMovement(direction, relative_positions[i][0], inverse_inertia0, inverse_effective_mass);
        c.movement_to_effective_movement[1] = CalculateMovementToEffectiveMovement(direction, relative_positions[i][1], inverse_inertia1, -inverse_effective_mass);

        auto coefficient_of_restitution = CombineRestitutionFactors( restitution_factors[body_to_restitution[body_pair.id1.index]],
                                                                           restitution_factors[body_to_restitution[body_pair.id2.index]] );
        auto old_relative_speed = Math::Dot( old_relative_velocities[i], direction );
        auto collision_correction = Math::Dot( new_relative_velocities[i], direction );
        auto bounce = coefficient_of_restitution * old_relative_speed;
        c.target_impulse = (collision_correction + bounce) / inverse_effective_mass;
        c.impulse_limits = {0, std::numeric_limits<float>::max()};

        c.total_impulse = c.target_impulse * warm_starting_factor;

        c.feedback_coefficient = 0;

        constraints[i] = c;
    }
}


// Assumes only the first body is of interest
void Physics::CalculateContactConstraints(
    Range<BodyPair const *> bodies,
    Range<Math::Float3 const *> collision_normals,
    Range<std::array<Math::Float3, 2> const * > relative_positions,
    Range<Math::Float3 const *> old_relative_velocities,
    Range<Math::Float3 const *> new_relative_velocities,
    Range<uint32_t const *> body_to_restitution,
    Range<float const *> restitution_factors,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Inertia const *> inverse_inertias,
    float const warm_starting_factor,
    Range<SingleBodyConstraint *> constraints)
{
    assert(Size(bodies) == Size(collision_normals));
    assert(Size(bodies) == Size(relative_positions));
    assert(Size(bodies) == Size(old_relative_velocities));
    assert(Size(bodies) == Size(new_relative_velocities));
    assert(Size(bodies) == Size(constraints));

    auto count = Size(constraints);

    for( auto i = 0u; i < count; ++i )
    {
        auto body_pair = bodies[i];

        SingleBodyConstraint c;
        c.body_index = rigid_body_to_element[body_pair.id1.index];

        auto inverse_inertia0 = inverse_inertias[c.body_index];
        auto direction = collision_normals[i];
        auto movement0 = c.unit_impulse = MovementFromImpulse(direction, relative_positions[i][0]);
        auto inverse_effective_mass = CalculateInverseEffectiveMass(movement0, inverse_inertia0);
        c.movement_to_effective_movement = CalculateMovementToEffectiveMovement(direction, relative_positions[i][0], inverse_inertia0, inverse_effective_mass);

        auto coefficient_of_restitution = CombineRestitutionFactors( restitution_factors[body_to_restitution[body_pair.id1.index]],
                                                                           restitution_factors[body_to_restitution[body_pair.id2.index]] );
        auto old_relative_speed = Math::Dot( old_relative_velocities[i], direction );
        auto collision_correction = Math::Dot( new_relative_velocities[i], direction );
        auto bounce = coefficient_of_restitution * old_relative_speed;
        c.target_impulse = (collision_correction + bounce) / inverse_effective_mass;
        c.impulse_limits = {0, std::numeric_limits<float>::max()};

        c.total_impulse = c.target_impulse * warm_starting_factor;

        c.feedback_coefficient = 0;

        constraints[i] = c;
    }
}


size_t Physics::CalculateFrictionConstraints(
    Range<BodyPair const *> bodies,
    Range<Math::Float3 const *> collision_normals,
    Range<std::array<Math::Float3, 2> const * > relative_positions,
    Range<Math::Float3 const *> relative_velocities,
    Range<uint32_t const *> body_to_friction,
    Range<float const *> friction_factors,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Math::Float3 const *> forces,
    Range<Inertia const *> inverse_inertias,
    float const time_step,
    float const warm_starting_factor,
    Range<Constraint *> constraints
    )
{
    assert(Size(bodies) == Size(collision_normals));
    assert(Size(bodies) == Size(relative_positions));
    assert(Size(bodies) == Size(relative_velocities));
    assert(Size(bodies) == Size(constraints));

    auto max_count = Size(bodies);
    auto constraint_count = 0u;
    for( auto i = 0u; i < max_count; ++i )
    {
        auto normal = collision_normals[i];
        auto relative_velocity = relative_velocities[i];
        auto perpendicular_velocity = ComponentPerpendicularToNormal( relative_velocity, normal );
        auto perpendicular_speed = Norm(perpendicular_velocity);
        if(perpendicular_speed != 0)
        {
            auto body_pair = bodies[i];
            Constraint c;
            c.body_indices = {{rigid_body_to_element[body_pair.id1.index], rigid_body_to_element[body_pair.id2.index]}};
            auto direction = perpendicular_velocity / perpendicular_speed;
            auto relative_position = relative_positions[i];
            auto inverse_inertia0 = inverse_inertias[c.body_indices[0]];
            auto inverse_inertia1 = inverse_inertias[c.body_indices[1]];
            auto movement0 = c.unit_impulses[0] = MovementFromImpulse(direction, relative_position[0]);
            auto movement1 = MovementFromImpulse(direction, relative_position[1]);
            c.unit_impulses[1] = -movement1;
            auto inverse_effective_mass = CalculateInverseEffectiveMass(movement0, movement1, inverse_inertia0, inverse_inertia1);
            c.movement_to_effective_movement[0] = CalculateMovementToEffectiveMovement(direction, relative_position[0], inverse_inertia0, inverse_effective_mass);
            c.movement_to_effective_movement[1] = CalculateMovementToEffectiveMovement(direction, relative_position[1], inverse_inertia1, -inverse_effective_mass);
            c.target_impulse = perpendicular_speed / inverse_effective_mass;

            auto friction_factor0 = friction_factors[body_to_friction[body_pair.id1.index]];
            auto friction_factor1 = friction_factors[body_to_friction[body_pair.id2.index]];
            auto friction_factor = CombineFrictionFactors(friction_factor0, friction_factor1);
            auto force0 = forces[c.body_indices[0]];
            auto force1 = forces[c.body_indices[1]];
            auto normal_force0 = Dot(-normal, force0);
            auto normal_force1 = Dot(+normal, force1);
            auto max_normal_force = Math::Max(normal_force0, normal_force1);
            auto max_friction_force = friction_factor * max_normal_force;
            auto max_friction_impulse = max_friction_force * time_step;
            max_friction_impulse = Math::Max(max_friction_impulse, 0.f);
            c.impulse_limits = {-max_friction_impulse, max_friction_impulse};
            c.total_impulse = Math::Clamp(-max_friction_impulse, max_friction_impulse, c.target_impulse * warm_starting_factor);

            c.feedback_coefficient = 0;
            if(c.impulse_limits.min != 0 && c.impulse_limits.max != 0)
            {
                constraints[constraint_count] = c;
                constraint_count += 1;
            }
        }
    }
    return constraint_count;
}

// Assumes only the first body is of interest
size_t Physics::CalculateFrictionConstraints(
    Range<BodyPair const *> bodies,
    Range<Math::Float3 const *> collision_normals,
    Range<std::array<Math::Float3, 2> const * > relative_positions,
    Range<Math::Float3 const *> relative_velocities,
    Range<uint32_t const *> body_to_friction,
    Range<float const *> friction_factors,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Math::Float3 const *> forces,
    Range<Inertia const *> inverse_inertias,
    float const time_step,
    float const warm_starting_factor,
    Range<SingleBodyConstraint *> constraints
    )
{
    assert(Size(bodies) == Size(collision_normals));
    assert(Size(bodies) == Size(relative_positions));
    assert(Size(bodies) == Size(relative_velocities));
    assert(Size(bodies) == Size(constraints));

    auto max_count = Size(bodies);
    auto constraint_count = 0u;
    for( auto i = 0u; i < max_count; ++i )
    {
        auto normal = collision_normals[i];
        auto relative_velocity = relative_velocities[i];
        auto perpendicular_velocity = ComponentPerpendicularToNormal( relative_velocity, normal );
        auto perpendicular_speed = Norm(perpendicular_velocity);
        if(perpendicular_speed != 0)
        {
            auto body_pair = bodies[i];

            SingleBodyConstraint c;
            c.body_index = rigid_body_to_element[body_pair.id1.index];
            auto direction = perpendicular_velocity / perpendicular_speed;
            auto inverse_inertia0 = inverse_inertias[c.body_index];
            auto relative_position = relative_positions[i][0];
            auto movement0 = c.unit_impulse = MovementFromImpulse(direction, relative_position);
            auto inverse_effective_mass = CalculateInverseEffectiveMass(movement0, inverse_inertia0);
            c.movement_to_effective_movement = CalculateMovementToEffectiveMovement(direction, relative_position, inverse_inertia0, inverse_effective_mass);
            c.target_impulse = perpendicular_speed / inverse_effective_mass;

            auto friction_factor0 = friction_factors[body_to_friction[body_pair.id1.index]];
            auto friction_factor1 = friction_factors[body_to_friction[body_pair.id2.index]];
            auto friction_factor = CombineFrictionFactors(friction_factor0, friction_factor1);
            auto force = forces[c.body_index];
            auto normal_force = Dot(-normal, force);
            auto max_friction_force = friction_factor * normal_force;
            auto max_friction_impulse = max_friction_force * time_step;
            max_friction_impulse = Math::Max(max_friction_impulse, 0.f);
            c.impulse_limits = {-max_friction_impulse, max_friction_impulse};
            c.total_impulse = Math::Clamp(-max_friction_impulse, max_friction_impulse, c.target_impulse * warm_starting_factor);

            c.feedback_coefficient = 0;

            if(c.impulse_limits.min != 0 && c.impulse_limits.max != 0)
            {
                constraints[constraint_count] = c;
                constraint_count += 1;
            }
        }
    }
    return constraint_count;
}


void Physics::CalculateVelocityConstraints(
    Range<BodyPair const *> bodies,
    Range<Math::Float3 const *> directions,
    Range<std::array<Math::Float3, 2> const * > local_positions,
    Range<float const *> target_speeds,
    Range<MinMax<float> const *> minmax_forces,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Orientation const *> orientations,
    Range<Movement const *> movements,
    Range<Inertia const *> inverse_inertias,
    float const time_step,
    float const warm_starting_factor,
    Range<Constraint *> constraints
    )
{
    assert(Size(bodies) == Size(directions));
    assert(Size(bodies) == Size(local_positions));
    assert(Size(bodies) == Size(target_speeds));
    assert(Size(bodies) == Size(minmax_forces));
    assert(Size(bodies) == Size(constraints));

    auto count = Size(bodies);
    for( auto i = 0u; i < count; ++i )
    {
        auto body_pair = bodies[i];

        Constraint c;
        c.body_indices = {{rigid_body_to_element[body_pair.id1.index], rigid_body_to_element[body_pair.id2.index]}};

        Orientation body_orientations[] = {orientations[c.body_indices[0]], orientations[c.body_indices[1]]};
        Math::Float3 relative_positions[] = {Rotate(local_positions[i][0], body_orientations[0].rotation), Rotate(local_positions[i][1], body_orientations[1].rotation)};

        auto direction = directions[i];
        direction = Rotate(direction, body_orientations[0].rotation);
        auto inverse_inertia0 = inverse_inertias[c.body_indices[0]];
        auto inverse_inertia1 = inverse_inertias[c.body_indices[1]];
        auto movement0 = c.unit_impulses[0] = MovementFromImpulse(direction, relative_positions[0]);
        auto movement1 = MovementFromImpulse(direction, relative_positions[1]);
        c.unit_impulses[1] = -movement1;

        auto inverse_effective_mass = CalculateInverseEffectiveMass(movement0, movement1, inverse_inertia0, inverse_inertia1);
        c.feedback_coefficient = 0;

        c.movement_to_effective_movement[0] = CalculateMovementToEffectiveMovement(direction, relative_positions[0], inverse_inertia0,  inverse_effective_mass);
        c.movement_to_effective_movement[1] = CalculateMovementToEffectiveMovement(direction, relative_positions[1], inverse_inertia1, -inverse_effective_mass);

        auto effective_movement = CalculateEffectiveMovement(movements[c.body_indices[0]], c.movement_to_effective_movement[0]) +
                                CalculateEffectiveMovement(movements[c.body_indices[1]], c.movement_to_effective_movement[1]);

        auto target_speed = target_speeds[i];
        c.target_impulse =  target_speed / inverse_effective_mass - effective_movement;

        c.impulse_limits = minmax_forces[i];
        c.impulse_limits.max *= time_step;
        c.impulse_limits.min *= time_step;

        c.total_impulse = c.target_impulse * warm_starting_factor;

        constraints[i] = c;
    }
}


void Physics::CalculateVelocityConstraints(
    Range<BodyID const *> bodies,
    Range<Math::Float3 const *> directions,
    Range<float const *> target_speeds,
    Range<MinMax<float> const *> minmax_forces,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Movement const *> movements,
    Range<Inertia const *> inverse_inertias,
    float const time_step,
    float const warm_starting_factor,
    Range<SingleBodyConstraint *> constraints
    )
{
    assert(Size(bodies) == Size(directions));
    assert(Size(bodies) == Size(target_speeds));
    assert(Size(bodies) == Size(minmax_forces));
    assert(Size(bodies) == Size(constraints));

    auto count = Size(bodies);
    for( auto i = 0u; i < count; ++i )
    {
        auto body_id = bodies[i];

        SingleBodyConstraint c;
        c.body_index = rigid_body_to_element[body_id.index];

        auto inverse_mass = inverse_inertias[c.body_index].mass;
        auto direction = directions[i];
        //auto linear0 = SquaredNorm(direction) * inverse_mass; // norm of the direction is always 1
        auto linear0 = inverse_mass;
        auto inverse_effective_mass = linear0;
        c.movement_to_effective_movement = CalculateMovementToEffectiveMovement(direction, 0, inverse_inertias[c.body_index], inverse_effective_mass);

        auto speed = inverse_mass * Dot(movements[c.body_index].momentum, direction);
        c.target_impulse = (target_speeds[i] - speed) / inverse_effective_mass;

        auto minmax_force = minmax_forces[i];
        c.impulse_limits = {minmax_force.min * time_step, minmax_force.max * time_step};

        c.total_impulse = Math::Clamp(c.impulse_limits.min, c.impulse_limits.max, c.target_impulse * warm_starting_factor);

        c.unit_impulse = {direction, 0};

        c.feedback_coefficient = 0;

        constraints[i] = c;
    }
}


void Physics::CalculateAngularVelocityConstraints(
    Range<BodyID const *> bodies,
    Range<Math::Float3 const *> angular_directions,
    Range<float const *> target_speeds,
    Range<MinMax<float> const *> minmax_torques,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Movement const *> movements,
    Range<Inertia const *> inverse_inertias,
    float const time_step,
    float const warm_starting_factor,
    Range<SingleBodyConstraint *> constraints
    )
{
    assert(Size(bodies) == Size(angular_directions));
    assert(Size(bodies) == Size(target_speeds));
    assert(Size(bodies) == Size(minmax_torques));
    assert(Size(bodies) == Size(constraints));

    auto count = Size(bodies);
    for( auto i = 0u; i < count; ++i )
    {
        auto body_id = bodies[i];

        SingleBodyConstraint c;
        c.body_index = rigid_body_to_element[body_id.index];

        auto angular_direction = angular_directions[i];
        auto inverse_inertia = inverse_inertias[c.body_index];
        auto angular = Dot(angular_direction, inverse_inertia.moment * angular_direction);
        auto inverse_effective_mass = angular;
        c.movement_to_effective_movement.linear = 0;
        c.movement_to_effective_movement.angular = angular_direction * inverse_inertia.moment / inverse_effective_mass;

        auto angular_velocity = inverse_inertia.moment * movements[c.body_index].angular_momentum;
        auto speed = Dot(angular_velocity, angular_direction);
        c.target_impulse = (target_speeds[i] - speed) / inverse_effective_mass;

        auto minmax_torque = minmax_torques[i];
        c.impulse_limits = {minmax_torque.min * time_step, minmax_torque.max * time_step};

        c.total_impulse = Math::Clamp(c.impulse_limits.min, c.impulse_limits.max, c.target_impulse * warm_starting_factor);

        c.unit_impulse.momentum = 0;
        c.unit_impulse.angular_momentum = angular_direction;

        c.feedback_coefficient = 0;

        constraints[i] = c;
    }
}

// ************** SOFT CONSTRAINTS ******************* //

namespace
{
    float CalulateSoftness(float const frequency, float const damping_ratio, float const inverse_effective_mass, float const time_step)
    {
        auto natural_frequency = Math::c_2PI.f * frequency;
        auto oscillation_ratio = time_step * natural_frequency;
        auto softness = inverse_effective_mass / ( oscillation_ratio * (damping_ratio + damping_ratio + oscillation_ratio));
        return softness;
    }
}

// void Physics::CalculateSoftConstraint(
//    Math::Float3 direction,
//    Math::Float3 relative_position,
//    Math::Float3 relative_velocity,
//    float const target_distance,
//    float const target_speed,
//    float const damping_ratio,
//    float const frequency,
//    MinMax<float> minmax_force,
//    float const time_step)
// {

//    SingleBodyConstraint c;
//    c.body_index = 0;
//    c.total_impulse = 0;
//    auto movement = c.unit_impulse = MovementFromImpulse(direction, relative_position);
//    auto const & inverse_inertia = inverse_inertias[c.body_index];
//    auto inverse_effective_mass = CalculateInverseEffectiveMass(movement, inverse_inertia);
//    auto softness = CalulateSoftness(frequency, damping_ratio, inverse_effective_mass, time_step);

//    auto softened_inverse_effective_mass = inverse_effective_mass + softness;
//    c.feedback_coefficient = softness / softened_inverse_effective_mass;

//    auto speed = Dot(relative_velocity, direction);

//    c.target_impulse = target_distance * time_step * k * c.feedback_coefficient + (target_speed - speed) / softened_inverse_effective_mass;
//    c.impulse_limits = {minmax_force.min * time_step, minmax_force.max * time_step};
//    c.movement_to_effective_movement = CalculateMovementToEffectiveMovement(direction, relative_position, inverse_inertia, softened_inverse_effective_mass);
//    return c;
//}

void Physics::CalculateSoftNonPenetrationConstraints(
    Range<BodyPair const *> bodies,
    Range<Math::Float3 const *> collision_normals,
    Range<std::array<Math::Float3, 2> const * > relative_positions,
    Range<float const *> penetration_depths,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Inertia const *> inverse_inertias,
    float const penetration_depth_tolerance,
    float const damping_ratio,
    float const frequency,
    float const time_step,
    float const warm_starting_factor,
    Range<Constraint *> constraints)
{
    assert(Size(bodies) == Size(collision_normals));
    assert(Size(bodies) == Size(relative_positions));
    assert(Size(bodies) == Size(penetration_depths));
    assert(Size(bodies) == Size(constraints));

    auto count = Size(bodies);
    for( auto i = 0u; i < count; ++i )
    {
        auto body_pair = bodies[i];

        Constraint c;
        c.body_indices = {{rigid_body_to_element[body_pair.id1.index], rigid_body_to_element[body_pair.id2.index]}};

        auto direction = collision_normals[i];
        auto inverse_inertia0 = inverse_inertias[c.body_indices[0]];
        auto inverse_inertia1 = inverse_inertias[c.body_indices[1]];
        auto movement0 = c.unit_impulses[0] = MovementFromImpulse(direction, relative_positions[i][0]);
        auto movement1 = MovementFromImpulse(direction, relative_positions[i][1]);
        c.unit_impulses[1] = -movement1;

        auto inverse_effective_mass = CalculateInverseEffectiveMass(movement0, movement1, inverse_inertia0, inverse_inertia1);
        auto softness = CalulateSoftness(frequency, damping_ratio, inverse_effective_mass, time_step);
        auto softened_inverse_effective_mass = inverse_effective_mass + softness;
        c.feedback_coefficient = softness / softened_inverse_effective_mass;

        c.movement_to_effective_movement[0] = CalculateMovementToEffectiveMovement(collision_normals[i], relative_positions[i][0], inverse_inertia0, softened_inverse_effective_mass);
        c.movement_to_effective_movement[1] = CalculateMovementToEffectiveMovement(collision_normals[i], relative_positions[i][1], inverse_inertia1, -softened_inverse_effective_mass);

        auto distance_to_correct = penetration_depths[i] - penetration_depth_tolerance;
        auto natural_frequency = Math::c_2PI.f * frequency;
        auto k = natural_frequency * natural_frequency / inverse_effective_mass;
        c.target_impulse = distance_to_correct * time_step * k * c.feedback_coefficient;

        c.impulse_limits = {0, std::numeric_limits<float>::max()};

        c.total_impulse = c.target_impulse * warm_starting_factor;

        constraints[i] = c;
    }
}


// Assumes only the first body is of interest
void Physics::CalculateSoftNonPenetrationConstraints(
    Range<BodyPair const *> bodies,
    Range<Math::Float3 const *> collision_normals,
    Range<std::array<Math::Float3, 2> const * > relative_positions,
    Range<float const*> penetration_depths,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Inertia const *> inverse_inertias,
    float const penetration_depth_tolerance,
    float const damping_ratio,
    float const frequency,
    float const time_step,
    float const warm_starting_factor,
    Range<SingleBodyConstraint *> constraints)
{
    assert(Size(bodies) == Size(collision_normals));
    assert(Size(bodies) == Size(relative_positions));
    assert(Size(bodies) == Size(penetration_depths));
    assert(Size(bodies) == Size(constraints));

    auto count = Size(bodies);
    for( auto i = 0u; i < count; ++i )
    {
        auto body_pair = bodies[i];

        SingleBodyConstraint c;
        c.body_index = rigid_body_to_element[body_pair.id1.index];

        auto direction = collision_normals[i];
        auto relative_position = relative_positions[i][0];
        auto inverse_inertia0 = inverse_inertias[c.body_index];
        auto movement0 = c.unit_impulse = MovementFromImpulse(direction, relative_position);
        auto inverse_effective_mass = CalculateInverseEffectiveMass(movement0, inverse_inertia0);

        auto softness = CalulateSoftness(frequency, damping_ratio, inverse_effective_mass, time_step);
        auto softened_inverse_effective_mass = inverse_effective_mass + softness;
        c.feedback_coefficient = softness / softened_inverse_effective_mass;

        c.movement_to_effective_movement = CalculateMovementToEffectiveMovement(direction, relative_position, inverse_inertia0, softened_inverse_effective_mass);

        auto distance_to_correct = penetration_depths[i] - penetration_depth_tolerance;
        auto natural_frequency = Math::c_2PI.f * frequency;
        auto k = natural_frequency * natural_frequency / inverse_effective_mass;
        c.target_impulse = distance_to_correct * time_step * k * c.feedback_coefficient;

        c.impulse_limits = {0, std::numeric_limits<float>::max()};

        c.total_impulse = c.target_impulse * warm_starting_factor;

        constraints[i] = c;
    }
}



void Physics::CalculateSoftVelocityConstraints(
    Range<BodyID const *> bodies,
    Range<Math::Float3 const *> directions,
    Range<float const *> target_speeds,
    Range<MinMax<float> const *> minmax_forces,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Movement const *> movements,
    Range<Inertia const *> inverse_inertias,
    float const time_step,
    float const warm_starting_factor,
    float const damping_ratio,
    float const frequency,
    Range<SingleBodyConstraint *> constraints
    )
{
    assert(Size(bodies) == Size(directions));
    assert(Size(bodies) == Size(target_speeds));
    assert(Size(bodies) == Size(minmax_forces));
    assert(Size(bodies) == Size(constraints));

    auto count = Size(bodies);
    for( auto i = 0u; i < count; ++i )
    {
        auto body_id = bodies[i];

        SingleBodyConstraint c;
        c.body_index = rigid_body_to_element[body_id.index];

        auto inverse_mass = inverse_inertias[c.body_index].mass;
        auto direction = directions[i];
        //auto linear0 = SquaredNorm(direction) * inverse_mass; // norm of the direction is always 1
        auto linear0 = inverse_mass;
        auto inverse_effective_mass = linear0;

        auto softness = CalulateSoftness(frequency, damping_ratio, inverse_effective_mass, time_step);
        auto softened_inverse_effective_mass = inverse_effective_mass + softness;

        c.feedback_coefficient = softness / softened_inverse_effective_mass;

        c.movement_to_effective_movement = CalculateMovementToEffectiveMovement(direction, 0, inverse_inertias[c.body_index], softened_inverse_effective_mass);

        auto speed = inverse_mass * Dot(movements[c.body_index].momentum, direction);
        c.target_impulse = (target_speeds[i] - speed) / softened_inverse_effective_mass;

        auto minmax_force = minmax_forces[i];
        c.impulse_limits = {minmax_force.min * time_step, minmax_force.max * time_step};

        c.total_impulse = Math::Clamp(c.impulse_limits.min, c.impulse_limits.max, c.target_impulse * warm_starting_factor);

        c.unit_impulse = {direction, 0};

        constraints[i] = c;
    }
}


void Physics::CalculateSoftRotationConstraints(
    Range<BodyID const *> bodies,
    Range<Math::Float3 const *> rotation_normals,
    Range<float const *> target_angles,
    Range<MinMax<float> const *> minmax_torques,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Orientation const *> orientations,
    Range<Movement const *> movements,
    Range<Inertia const *> inverse_inertias,
    float const time_step,
    float const warm_starting_factor,
    float const damping_ratio,
    float const frequency,
    Range<SingleBodyConstraint *> constraints
    )
{
    assert(Size(bodies) == Size(rotation_normals));
    assert(Size(bodies) == Size(target_angles));
    assert(Size(bodies) == Size(minmax_torques));
    assert(Size(bodies) == Size(constraints));

    auto count = Size(bodies);
    for( auto i = 0u; i < count; ++i )
    {
        auto body_id = bodies[i];

        SingleBodyConstraint c;
        c.body_index = rigid_body_to_element[body_id.index];

        auto rotation_normal = rotation_normals[i];
        c.unit_impulse = {0, rotation_normal};

        auto angle = AngleAroundNormal(orientations[c.body_index].rotation, rotation_normal);

        auto inverse_inertia = inverse_inertias[c.body_index];
        auto angular = Dot(rotation_normal, inverse_inertia.moment * rotation_normal);
        auto inverse_effective_mass = angular;
        // auto inverse_effective_mass = CalculateInverseEffectiveMass(c.unit_impulse, inverse_inertia);
        auto softness = CalulateSoftness(frequency, damping_ratio, inverse_effective_mass, time_step);
        auto softened_inverse_effective_mass = inverse_effective_mass + softness;
        c.feedback_coefficient = softness / softened_inverse_effective_mass;
        c.movement_to_effective_movement.linear = 0;
        c.movement_to_effective_movement.angular = rotation_normal * inverse_inertia.moment / softened_inverse_effective_mass;

        auto angular_velocity = inverse_inertia.moment * movements[c.body_index].angular_momentum;
        auto speed = Dot(angular_velocity, rotation_normal);

        auto natural_frequency = Math::c_2PI.f * frequency;
        auto k = natural_frequency * natural_frequency / inverse_effective_mass;
        auto angle_difference = Math::AddAngles(target_angles[i], -angle);
        c.target_impulse = angle_difference * time_step * k * c.feedback_coefficient - speed / softened_inverse_effective_mass;

        auto minmax_torque = minmax_torques[i];
        c.impulse_limits = {minmax_torque.min * time_step, minmax_torque.max * time_step};

        c.total_impulse = Math::Clamp(c.impulse_limits.min, c.impulse_limits.max, c.target_impulse * warm_starting_factor);


        constraints[i] = c;
    }
}


void Physics::CalculateRotationConstraints(
    Range<BodyID const *> bodies,
    Range<Math::Float3 const *> rotation_normals,
    Range<float const *> target_angles,
    Range<MinMax<float> const *> minmax_torques,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Orientation const *> orientations,
    Range<Movement const *> movements,
    Range<Inertia const *> inverse_inertias,
    float const time_step,
    float const warm_starting_factor,
    Range<SingleBodyConstraint *> constraints
    )
{
    assert(Size(bodies) == Size(rotation_normals));
    assert(Size(bodies) == Size(target_angles));
    assert(Size(bodies) == Size(minmax_torques));
    assert(Size(bodies) == Size(constraints));

    auto count = Size(bodies);
    for(auto i = 0u; i < count; ++i)
    {
        auto body_id = bodies[i];

        SingleBodyConstraint c;
        c.body_index = rigid_body_to_element[body_id.index];

        auto rotation_normal = rotation_normals[i];
        c.unit_impulse = {0, rotation_normal};

        auto inverse_inertia = inverse_inertias[c.body_index];
        auto angular = Dot(rotation_normal, inverse_inertia.moment * rotation_normal);
        auto inverse_effective_mass = angular;

        c.feedback_coefficient = 0;
        c.movement_to_effective_movement.linear = 0;
        c.movement_to_effective_movement.angular = rotation_normal * inverse_inertia.moment / inverse_effective_mass;

        auto angular_velocity = inverse_inertia.moment * movements[c.body_index].angular_momentum;
        auto angular_speed = Dot(angular_velocity, rotation_normal);

        auto angle = AngleAroundNormal(orientations[c.body_index].rotation, rotation_normal);
        assert(!isnan(angle) && !isinf(angle));
        auto angle_difference = Math::AddAngles(target_angles[i], -angle);
        auto target_angular_speed = angle_difference / time_step;
        c.target_impulse = (target_angular_speed - angular_speed) / inverse_effective_mass;

        auto minmax_torque = minmax_torques[i];
        c.impulse_limits = {minmax_torque.min * time_step, minmax_torque.max * time_step};

        c.total_impulse = Math::Clamp(c.impulse_limits.min, c.impulse_limits.max, c.target_impulse * warm_starting_factor);


        constraints[i] = c;
    }
}


void Physics::CalculateSoftPositionConstraints(
    Range<BodyPair const *> bodies,
    Range<Math::Float3 const *> directions,
    Range<std::array<Math::Float3, 2> const * > local_positions,
    Range<float const *> distances,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Orientation const*> orientations,
    Range<Movement const *> movements,
    Range<Inertia const *> inverse_inertias,
    float const damping_ratio,
    float const frequency,
    float const time_step,
    float const warm_starting_factor,
    Range<Constraint *> constraints)
{
    assert(Size(bodies) == Size(directions));
    assert(Size(bodies) == Size(local_positions));
    assert(Size(bodies) == Size(distances));
    assert(Size(bodies) == Size(constraints));

    auto count = Size(bodies);
    for( auto i = 0u; i < count; ++i )
    {
        auto body_pair = bodies[i];

        Constraint c;
        c.body_indices = {{rigid_body_to_element[body_pair.id1.index], rigid_body_to_element[body_pair.id2.index]}};

        Orientation body_orientations[] = {orientations[c.body_indices[0]], orientations[c.body_indices[1]]};
        Math::Float3 relative_positions[] = {Rotate(local_positions[i][0], body_orientations[0].rotation), Rotate(local_positions[i][1], body_orientations[1].rotation)};

        auto direction = directions[i];
        direction = Rotate(direction, body_orientations[0].rotation);
        auto inverse_inertia0 = inverse_inertias[c.body_indices[0]];
        auto inverse_inertia1 = inverse_inertias[c.body_indices[1]];
        auto movement0 = c.unit_impulses[0] = MovementFromImpulse(direction, relative_positions[0]);
        auto movement1 = MovementFromImpulse(direction, relative_positions[1]);
        c.unit_impulses[1] = -movement1;

        auto inverse_effective_mass = CalculateInverseEffectiveMass(movement0, movement1, inverse_inertia0, inverse_inertia1);
        auto softness = CalulateSoftness(frequency, damping_ratio, inverse_effective_mass, time_step);
        auto softened_inverse_effective_mass = inverse_effective_mass + softness;
        c.feedback_coefficient = softness / softened_inverse_effective_mass;

        c.movement_to_effective_movement[0] = CalculateMovementToEffectiveMovement(direction, relative_positions[0], inverse_inertia0, softened_inverse_effective_mass);
        c.movement_to_effective_movement[1] = CalculateMovementToEffectiveMovement(direction, relative_positions[1], inverse_inertia1, -softened_inverse_effective_mass);

        auto effective_movement = CalculateEffectiveMovement(movements[c.body_indices[0]], c.movement_to_effective_movement[0]) +
                                CalculateEffectiveMovement(movements[c.body_indices[1]], c.movement_to_effective_movement[1]);

        auto relative_position = body_orientations[0].position - body_orientations[1].position + relative_positions[0] - relative_positions[1];
        auto distance = Dot(relative_position, direction);
        auto distance_to_correct = distances[i] - distance;
        auto natural_frequency = Math::c_2PI.f * frequency;
        auto k = natural_frequency * natural_frequency / inverse_effective_mass;
        c.target_impulse = distance_to_correct * time_step * k * c.feedback_coefficient - effective_movement;

        c.impulse_limits = {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max()};

        c.total_impulse = c.target_impulse * warm_starting_factor;

        constraints[i] = c;
    }
}


void Physics::CalculatePositionConstraints(
    Range<BodyPair const *> bodies,
    Range<Math::Float3 const *> directions,
    Range<std::array<Math::Float3, 2> const * > local_positions,
    Range<float const *> distances,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Orientation const*> orientations,
    Range<Movement const *> movements,
    Range<Inertia const *> inverse_inertias,
    float const time_step,
    float const warm_starting_factor,
    Range<Constraint *> constraints)
{
    assert(Size(bodies) == Size(directions));
    assert(Size(bodies) == Size(local_positions));
    assert(Size(bodies) == Size(distances));
    assert(Size(bodies) == Size(constraints));

    auto count = Size(bodies);
    for( auto i = 0u; i < count; ++i )
    {
        auto body_pair = bodies[i];

        Constraint c;
        c.body_indices = {{rigid_body_to_element[body_pair.id1.index], rigid_body_to_element[body_pair.id2.index]}};

        Orientation body_orientations[] = {orientations[c.body_indices[0]], orientations[c.body_indices[1]]};
        Math::Float3 relative_positions[] = {Rotate(local_positions[i][0], body_orientations[0].rotation), Rotate(local_positions[i][1], body_orientations[1].rotation)};

        auto direction = directions[i];
        direction = Rotate(direction, body_orientations[0].rotation);
        auto inverse_inertia0 = inverse_inertias[c.body_indices[0]];
        auto inverse_inertia1 = inverse_inertias[c.body_indices[1]];
        auto movement0 = c.unit_impulses[0] = MovementFromImpulse(direction, relative_positions[0]);
        auto movement1 = MovementFromImpulse(direction, relative_positions[1]);
        c.unit_impulses[1] = -movement1;

        auto inverse_effective_mass = CalculateInverseEffectiveMass(movement0, movement1, inverse_inertia0, inverse_inertia1);
        c.feedback_coefficient = 0;

        c.movement_to_effective_movement[0] = CalculateMovementToEffectiveMovement(direction, relative_positions[0], inverse_inertia0,  inverse_effective_mass);
        c.movement_to_effective_movement[1] = CalculateMovementToEffectiveMovement(direction, relative_positions[1], inverse_inertia1, -inverse_effective_mass);

        auto effective_movement = CalculateEffectiveMovement(movements[c.body_indices[0]], c.movement_to_effective_movement[0]) +
                                CalculateEffectiveMovement(movements[c.body_indices[1]], c.movement_to_effective_movement[1]);

        auto relative_position = body_orientations[0].position - body_orientations[1].position + relative_positions[0] - relative_positions[1];
        auto distance = Dot(direction, relative_position);
        auto distance_to_correct = distances[i] - distance;
        c.target_impulse = distance_to_correct / (time_step * inverse_effective_mass) - effective_movement;

        c.impulse_limits = {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max()};

        c.total_impulse = c.target_impulse * warm_starting_factor;

        constraints[i] = c;
    }
}


void Physics::CalculateDistanceConstraints(
    Range<BodyPair const *> bodies,
    Range<std::array<Math::Float3, 2> const * > local_positions,
    Range<float const *> distances,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Orientation const *> orientations,
    Range<Movement const *> movements,
    Range<Inertia const *> inverse_inertias,
    float const time_step,
    float const warm_starting_factor,
    Range<Constraint *> constraints)
{
    assert(Size(bodies) == Size(local_positions));
    assert(Size(bodies) == Size(distances));
    assert(Size(bodies) == Size(constraints));

    auto count = Size(bodies);
    for(auto i = 0u; i < count; ++i)
    {
        auto body_pair = bodies[i];

        Constraint c;
        c.body_indices = {{rigid_body_to_element[body_pair.id1.index], rigid_body_to_element[body_pair.id2.index]}};

        Orientation body_orientations[] = {orientations[c.body_indices[0]], orientations[c.body_indices[1]]};
        Math::Float3 relative_positions[] = {Rotate(local_positions[i][0], body_orientations[0].rotation), Rotate(local_positions[i][1], body_orientations[1].rotation)};
        auto relative_position = body_orientations[0].position - body_orientations[1].position + relative_positions[0] - relative_positions[1];
        auto distance = Norm(relative_position);
        auto direction = relative_position / distance;
        auto inverse_inertia0 = inverse_inertias[c.body_indices[0]];
        auto inverse_inertia1 = inverse_inertias[c.body_indices[1]];
        auto movement0 = c.unit_impulses[0] = MovementFromImpulse(direction, relative_positions[0]);
        auto movement1 = MovementFromImpulse(direction, relative_positions[1]);
        c.unit_impulses[1] = -movement1;

        c.feedback_coefficient = 0;

        auto inverse_effective_mass = CalculateInverseEffectiveMass(movement0, movement1, inverse_inertia0, inverse_inertia1);
        c.movement_to_effective_movement[0] = CalculateMovementToEffectiveMovement(direction, relative_positions[0], inverse_inertia0, +inverse_effective_mass);
        c.movement_to_effective_movement[1] = CalculateMovementToEffectiveMovement(direction, relative_positions[1], inverse_inertia1, -inverse_effective_mass);

        auto effective_movement = CalculateEffectiveMovement(movements[c.body_indices[0]], c.movement_to_effective_movement[0]) +
            CalculateEffectiveMovement(movements[c.body_indices[1]], c.movement_to_effective_movement[1]);

        auto distance_to_correct = distances[i] - distance;
        c.target_impulse = distance_to_correct / (time_step * inverse_effective_mass) - effective_movement;

        c.impulse_limits = {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max()};

        c.total_impulse = c.target_impulse * warm_starting_factor;

        constraints[i] = c;
    }
}


void Physics::CalculateSoftDistanceConstraints(
    Range<BodyPair const *> bodies,
    Range<std::array<Math::Float3, 2> const * > local_positions,
    Range<float const *> distances,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Movement const *> movements,
    Range<Orientation const *> orientations,
    Range<Inertia const *> inverse_inertias,
    float const damping_ratio,
    float const frequency,
    float const time_step,
    float const warm_starting_factor,
    Range<Constraint *> constraints)
{
    assert(Size(bodies) == Size(local_positions));
    assert(Size(bodies) == Size(distances));
    assert(Size(bodies) == Size(constraints));

    auto count = Size(bodies);
    for( auto i = 0u; i < count; ++i )
    {
        auto body_pair = bodies[i];

        Constraint c;
        c.body_indices = {{rigid_body_to_element[body_pair.id1.index], rigid_body_to_element[body_pair.id2.index]}};

        Orientation body_orientations[] = {orientations[c.body_indices[0]], orientations[c.body_indices[1]]};
        Math::Float3 relative_positions[] = {Rotate(local_positions[i][0], body_orientations[0].rotation), Rotate(local_positions[i][1], body_orientations[1].rotation)};
        auto relative_position = body_orientations[0].position - body_orientations[1].position + relative_positions[0] - relative_positions[1];
        auto distance = Norm(relative_position);
        auto direction = relative_position / distance;
        auto inverse_inertia0 = inverse_inertias[c.body_indices[0]];
        auto inverse_inertia1 = inverse_inertias[c.body_indices[1]];
        auto movement0 = c.unit_impulses[0] = MovementFromImpulse(direction, relative_positions[0]);
        auto movement1 = MovementFromImpulse(direction, relative_positions[1]);
        c.unit_impulses[1] = -movement1;

        auto inverse_effective_mass = CalculateInverseEffectiveMass(movement0, movement1, inverse_inertia0, inverse_inertia1);
        auto softness = CalulateSoftness(frequency, damping_ratio, inverse_effective_mass, time_step);
        auto softened_inverse_effective_mass = inverse_effective_mass + softness;
        c.feedback_coefficient = softness / softened_inverse_effective_mass;

        c.movement_to_effective_movement[0] = CalculateMovementToEffectiveMovement(direction, relative_positions[0], inverse_inertia0, +softened_inverse_effective_mass);
        c.movement_to_effective_movement[1] = CalculateMovementToEffectiveMovement(direction, relative_positions[1], inverse_inertia1, -softened_inverse_effective_mass);

        auto effective_movement = CalculateEffectiveMovement(movements[c.body_indices[0]], c.movement_to_effective_movement[0]) +
                                CalculateEffectiveMovement(movements[c.body_indices[1]], c.movement_to_effective_movement[1]);

        auto distance_to_correct = distances[i] - distance;
        auto natural_frequency = Math::c_2PI.f * frequency;
        auto k = natural_frequency * natural_frequency / inverse_effective_mass;
        c.target_impulse = distance_to_correct * time_step * k * c.feedback_coefficient - effective_movement;

        c.impulse_limits = {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max()};

        c.total_impulse = c.target_impulse * warm_starting_factor;

        constraints[i] = c;
    }
}


void Physics::CalculateSoftRotationConstraints(
    Range<BodyPair const *> bodies,
    Range<Math::Float3 const *> rotation_normals,
    Range<float const *> target_angles,
    // Range<float const *> distances,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Orientation const *> orientations,
    Range<Movement const *> movements,
    Range<Inertia const *> inverse_inertias,
    float const damping_ratio,
    float const frequency,
    float const time_step,
    float const warm_starting_factor,
    Range<Constraint *> constraints)
{
    assert(Size(bodies) == Size(rotation_normals));
    assert(Size(bodies) == Size(target_angles));
    // assert(Size(bodies) == Size(distances));
    assert(Size(bodies) == Size(constraints));

    auto count = Size(bodies);
    for( auto i = 0u; i < count; ++i )
    {
        auto body_pair = bodies[i];

        Constraint c;
        c.body_indices = {{rigid_body_to_element[body_pair.id1.index], rigid_body_to_element[body_pair.id2.index]}};

        auto rotation_normal = rotation_normals[i];
        auto movement0 = c.unit_impulses[0] = {0, rotation_normal};
        auto movement1 = movement0;
        c.unit_impulses[1] = -movement1;

        auto inverse_inertia0 = inverse_inertias[c.body_indices[0]];
        auto inverse_inertia1 = inverse_inertias[c.body_indices[1]];
        auto inverse_effective_mass = CalculateInverseEffectiveMass(movement0, movement1, inverse_inertia0, inverse_inertia1);
        auto softness = CalulateSoftness(frequency, damping_ratio, inverse_effective_mass, time_step);
        auto softened_inverse_effective_mass = inverse_effective_mass + softness;
        c.feedback_coefficient = softness / softened_inverse_effective_mass;

        c.movement_to_effective_movement[0].linear = 0;
        c.movement_to_effective_movement[0].angular = rotation_normal * inverse_inertia0.moment / +softened_inverse_effective_mass;
        c.movement_to_effective_movement[1].linear = 0;
        c.movement_to_effective_movement[1].angular = rotation_normal * inverse_inertia1.moment / -softened_inverse_effective_mass;

        auto effective_movement = CalculateEffectiveMovement(movements[c.body_indices[0]], c.movement_to_effective_movement[0]) +
            CalculateEffectiveMovement(movements[c.body_indices[1]], c.movement_to_effective_movement[1]);


        Orientation body_orientations[] = {orientations[c.body_indices[0]], orientations[c.body_indices[1]]};
        auto relative_rotation = body_orientations[1].rotation * Conjugate(body_orientations[0].rotation);
        auto current_angle = AngleAroundNormal(relative_rotation, rotation_normal);
        auto target_angle = target_angles[i];
        auto angle_difference = Math::AddAngles(target_angle, -current_angle);
        auto natural_frequency = Math::c_2PI.f * frequency;
        auto k = natural_frequency * natural_frequency / inverse_effective_mass;
        c.target_impulse = angle_difference * time_step * k * c.feedback_coefficient - effective_movement;

        c.impulse_limits = {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max()};

        c.total_impulse = c.target_impulse * warm_starting_factor;
        constraints[i] = c;
    }
}

#pragma warning(push, 1)
void Physics::CalculateRotationConstraints(
    Range<BodyPair const *> bodies,
    Range<Math::Float3 const *> rotation_normals,
    Range<float const *> target_angles,
    // Range<float const *> distances,
    Range<uint32_t const *> rigid_body_to_element,
    Range<Orientation const *> orientations,
    Range<Movement const *> movements,
    Range<Inertia const *> inverse_inertias,
    float const time_step,
    float const warm_starting_factor,
    Range<Constraint *> constraints)
{
    assert(Size(bodies) == Size(rotation_normals));
    assert(Size(bodies) == Size(target_angles));
    // assert(Size(bodies) == Size(distances));
    assert(Size(bodies) == Size(constraints));

    auto count = Size(bodies);
    for(auto i = 0u; i < count; ++i)
    {
        auto body_pair = bodies[i];

        Constraint c;
        c.body_indices = {{rigid_body_to_element[body_pair.id1.index], rigid_body_to_element[body_pair.id2.index]}};

        Orientation body_orientations[] = {orientations[c.body_indices[0]], orientations[c.body_indices[1]]};
        auto rotation_normal = Rotate(rotation_normals[i], body_orientations[0].rotation);
        auto movement0 = c.unit_impulses[0] = {0, rotation_normal};
        auto movement1 = movement0;
        c.unit_impulses[1] = -movement1;

        auto inverse_inertia0 = inverse_inertias[c.body_indices[0]];
        auto inverse_inertia1 = inverse_inertias[c.body_indices[1]];
        auto inverse_effective_mass = CalculateInverseEffectiveMass(movement0, movement1, inverse_inertia0, inverse_inertia1);

        c.feedback_coefficient = 0;

        c.movement_to_effective_movement[0].linear = 0;
        c.movement_to_effective_movement[0].angular = rotation_normal * inverse_inertia0.moment / +inverse_effective_mass;
        c.movement_to_effective_movement[1].linear = 0;
        c.movement_to_effective_movement[1].angular = rotation_normal * inverse_inertia1.moment / -inverse_effective_mass;

        auto effective_movement = CalculateEffectiveMovement(movements[c.body_indices[0]], c.movement_to_effective_movement[0]) +
            CalculateEffectiveMovement(movements[c.body_indices[1]], c.movement_to_effective_movement[1]);

        auto relative_rotation = (body_orientations[1].rotation) * Conjugate(body_orientations[0].rotation);
        auto target_angle = target_angles[i];
        auto angle = Math::AngleAroundNormal(relative_rotation, rotation_normal);
        auto angle_difference = Math::AddAngles(target_angle, angle);
        auto target_angular_speed = angle_difference / time_step;
        c.target_impulse = target_angular_speed / inverse_effective_mass;
        c.target_impulse -= effective_movement;
        auto angular_speed = effective_movement * inverse_effective_mass; (void)angular_speed;
        c.impulse_limits = {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max()};

        c.total_impulse = c.target_impulse * warm_starting_factor;
        constraints[i] = c;
    }
}
