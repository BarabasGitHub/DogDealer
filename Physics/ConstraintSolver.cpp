#include "ConstraintSolver.h"

using namespace Physics;
// don't complain about hiding member variables
#pragma warning( disable : 4458 )

void ConstraintSolver::SetCollisionEvents(
        CollisionEvents const * collision_events,
        CollisionEventOffsets const * event_offsets)
{
    this->collision_events = collision_events;
    this->event_offsets = event_offsets;
}


void ConstraintSolver::SetConstraints(
    DistanceConstraints  const * distance_constraints,
    PositionConstraints const * position_constraints,
    RotationConstraints const * rotation_constraints,
    VelocityConstraints const * velocity_constraints,
    WorldVelocityConstraints const * world_velocity_constraints,
    WorldAngularVelocityConstraints const * angular_velocity_constraints,
    WorldRotationConstraints const * world_rotation_constraints
    )
{
    this->distance_constraints = distance_constraints;
    this->position_constraints = position_constraints;
    this->rotation_constraints = rotation_constraints;
    this->velocity_constraints = velocity_constraints;
    this->world_velocity_constraints = world_velocity_constraints;
    this->angular_velocity_constraints = angular_velocity_constraints;
    this->world_rotation_constraints = world_rotation_constraints;
}


void ConstraintSolver::SetRigidBodyData(
        Range<uint32_t const *> rigid_entity_to_element,
        Range<Movement const *> rigid_body_old_movements,
        Range<Movement *> rigid_body_new_movements,
        Range<Orientation *> rigid_body_orientations,
        Range<Inertia const *> rigid_body_inverse_inertias,
        Range<Math::Float3 const *> rigid_body_forces)
{
    this->rigid_body_to_element = rigid_entity_to_element;
    this->rigid_body_old_movements = rigid_body_old_movements;
    this->rigid_body_new_movements = rigid_body_new_movements;
    this->rigid_body_orientations = rigid_body_orientations;
    this->rigid_body_inverse_inertias = rigid_body_inverse_inertias;
    this->rigid_body_forces = rigid_body_forces;
}

void ConstraintSolver::SetCommonBodyData(
        Range<uint32_t const *> entity_to_element,
        Range<float const *> restitution_factors,
        Range<float const *> friction_factors)
{
    this->body_to_element = entity_to_element;
    this->restitution_factors = restitution_factors;
    this->friction_factors = friction_factors;
}


void ConstraintSolver::SetCommonConfiguration(
        float position_correction_fraction,
        float penetration_depth_tolerance,
        float angular_velocity_correction_fraction,
        uint32_t velocity_correction_iterations,
        uint32_t position_correction_iterations)
{
    this->position_correction_fraction = position_correction_fraction;
    this->penetration_depth_tolerance = penetration_depth_tolerance;
    this->angular_velocity_correction_fraction = angular_velocity_correction_fraction;
    this->velocity_correction_iterations = velocity_correction_iterations;
    this->position_correction_iterations = position_correction_iterations;
}


ConstraintSolver::~ConstraintSolver()
{}


ConstraintSolver::ConstraintSolver() :
    // collision event data
    collision_events(nullptr),
    event_offsets(nullptr),

    distance_constraints(nullptr),
    position_constraints(nullptr),
    rotation_constraints(nullptr),
    velocity_constraints(nullptr),

    // explicit velocity constraints
    world_velocity_constraints(nullptr),
    angular_velocity_constraints(nullptr),
    world_rotation_constraints(nullptr),

    // rigid body data
    rigid_body_to_element(nullptr, nullptr),
    rigid_body_old_movements(nullptr, nullptr),
    rigid_body_new_movements(nullptr, nullptr),
    rigid_body_inverse_inertias(nullptr, nullptr),
    rigid_body_forces(nullptr, nullptr),

    // common body data
    body_to_element(nullptr, nullptr),
    restitution_factors(nullptr, nullptr),
    friction_factors(nullptr, nullptr),

    // common configuration settings
    position_correction_fraction(0),
    penetration_depth_tolerance(0),
    angular_velocity_correction_fraction(0),

    velocity_correction_iterations(0),
    position_correction_iterations(0)
{}
