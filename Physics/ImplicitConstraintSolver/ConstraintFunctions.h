#pragma once

#include "../BodyID.h"
#include <Math\ForwardDeclarations.h>
#include <Utilities\Range.h>
#include <Utilities\MinMax.h>

#include <array>

struct Orientation;

namespace Physics
{
    struct Inertia;
    struct Movement;
    struct Constraint;
    struct SingleBodyConstraint;

    void CalculateNonPenetrationConstraints(
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
        Range<Constraint *> constraints);

    // Assumes only the first body is of interest
    void CalculateNonPenetrationConstraints(
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
        Range<SingleBodyConstraint *> constraints);


    void CalculateContactConstraints(
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
        Range<Constraint *> constraints);

    // Assumes only the first body is of interest
    void CalculateContactConstraints(
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
        Range<SingleBodyConstraint *> constraints);


    size_t CalculateFrictionConstraints(
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
        );

    size_t CalculateFrictionConstraints(
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
        );


    void CalculateDistanceConstraints(
        Range<BodyPair const *> bodies,
        Range<std::array<Math::Float3, 2> const * > local_positions,
        Range<float const *> distances,
        Range<uint32_t const *> rigid_body_to_element,
        Range<Orientation const *> orientations,
        Range<Movement const *> movements,
        Range<Inertia const *> inverse_inertias,
        float const time_step,
        float const warm_starting_factor,
        Range<Constraint *> constraints);


    void CalculateVelocityConstraints(
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
        );


    void CalculateVelocityConstraints(
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
        );


    void CalculateAngularVelocityConstraints(
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
        );


    void CalculateSoftNonPenetrationConstraints(
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
        Range<Constraint *> constraints);


    void CalculateSoftNonPenetrationConstraints(
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
        Range<SingleBodyConstraint *> constraints);


    void CalculateSoftVelocityConstraints(
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
        );


    void CalculateSoftRotationConstraints(
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
        );

    void CalculateRotationConstraints(
        Range<BodyID const*> bodies,
        Range<Math::Float3 const*> rotation_normals,
        Range<float const*> target_angles,
        Range<MinMax<float>const*> minmax_torques,
        Range<uint32_t const*> rigid_body_to_element,
        Range<Orientation const*> orientations,
        Range<Movement const *> movements,
        Range<Inertia const*> inverse_inertias,
        float const time_step,
        float const warm_starting_factor,
        Range<SingleBodyConstraint*> constraints
        );


    void CalculateSoftPositionConstraints(
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
        Range<Constraint *> constraints);


    void CalculatePositionConstraints(
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
        Range<Constraint *> constraints);


    void CalculateSoftDistanceConstraints(
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
        Range<Constraint *> constraints);


    void CalculateSoftRotationConstraints(
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
        Range<Constraint *> constraints);


    void CalculateRotationConstraints(
        Range<BodyPair const *> bodies,
        Range<Math::Float3 const *> rotation_normals,
        Range<float const *> target_angles,
        // Range<float const *> distances,
        Range<uint32_t const *> rigid_body_to_element,
        Range<Orientation const *> orientations,
        Range<Movement const*> movements,
        Range<Inertia const *> inverse_inertias,
        float const time_step,
        float const warm_starting_factor,
        Range<Constraint *> constraints);
}
