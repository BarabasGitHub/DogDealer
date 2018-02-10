#pragma once

#include <Math\ForwardDeclarations.h>
#include <Utilities\Range.h>

#include <cstdint>

struct Orientation;

namespace Physics
{
    struct Movement;
    struct Inertia;
    struct CollisionEventOffsets;
    struct WorldConfiguration;
    struct CollisionEvents;
    struct DistanceConstraints;
    struct PositionConstraints;
    struct RotationConstraints;
    struct VelocityConstraints;
    struct WorldVelocityConstraints;
    struct WorldAngularVelocityConstraints;
    struct WorldRotationConstraints;

    // interface for rigid body constraint solvers
    class ConstraintSolver
    {
    public:
        void SetCollisionEvents(
            CollisionEvents const * collision_events,
            CollisionEventOffsets const * event_offsets);

        void SetConstraints(
            DistanceConstraints  const * distance_constraints,
            PositionConstraints const * position_constraints,
            RotationConstraints const * rotation_constraints,
            VelocityConstraints const * velocity_constraints,
            WorldVelocityConstraints const * world_velocity_constraints,
            WorldAngularVelocityConstraints const * angular_velocity_constraints,
            WorldRotationConstraints const * world_rotation_constraints
            );


        void SetRigidBodyData(
            Range<uint32_t const *> rigid_entity_to_element,
            Range<Movement const *> rigid_body_old_movements,
            Range<Movement *> rigid_body_new_movements,
            Range<Orientation *> rigid_body_orientations,
            Range<Inertia const *> rigid_body_inverse_inertias,
            Range<Math::Float3 const *> rigid_body_forces);

        void SetCommonBodyData(
            Range<uint32_t const *> entity_to_element,
            Range<float const *> restitution_factors,
            Range<float const *> friction_factors);

        void SetCommonConfiguration(
            float position_correction_fraction,
            float penetration_depth_tolerance,
            float angular_velocity_correction_fraction,
            uint32_t velocity_correction_iterations,
            uint32_t position_correction_iterations);


        virtual void SetConfigurationFromWorldConfiguration(WorldConfiguration const & world_config) = 0;

        virtual void DoYourThing(float time_step) = 0;

        virtual ~ConstraintSolver();

    protected:
        // collision event data
        CollisionEvents const * collision_events;
        CollisionEventOffsets const * event_offsets;

        // constraints between bodies
        DistanceConstraints const * distance_constraints;
        PositionConstraints const * position_constraints;
        RotationConstraints const * rotation_constraints;
        VelocityConstraints const * velocity_constraints;

        // explicit velocity constraints
        WorldVelocityConstraints const * world_velocity_constraints;
        WorldAngularVelocityConstraints const * angular_velocity_constraints;

        // explicit orientation constraints
        WorldRotationConstraints const * world_rotation_constraints;

        // rigid body data
        Range<uint32_t const *> rigid_body_to_element;
        Range<Movement const *> rigid_body_old_movements;
        Range<Movement *> rigid_body_new_movements;
        Range<Orientation *> rigid_body_orientations;
        Range<Inertia const *> rigid_body_inverse_inertias;
        Range<Math::Float3 const *> rigid_body_forces;

        // common body data
        Range<uint32_t const *> body_to_element;
        Range<float const *> restitution_factors;
        Range<float const *> friction_factors;

        // common configuration settings
        float position_correction_fraction;
        float penetration_depth_tolerance;
        float angular_velocity_correction_fraction;

        uint32_t velocity_correction_iterations;
        uint32_t position_correction_iterations;

        ConstraintSolver();
        ConstraintSolver(ConstraintSolver const &) = delete;
        ConstraintSolver& operator=(ConstraintSolver const &) = delete;
    };
}
