#pragma once
#include <Conventions\EntityID.h>
#include <Conventions\Velocity.h>

#include <Utilities\Range.h>

struct EntityForces;
struct EntityPositions;
struct EntityTorques;
struct RotationConstraints;
struct VelocityConstraints;
struct AngularVelocityConstraints;
struct EntityRotations;
struct IndexedOrientations;
struct CollisionEvents;

namespace Logic
{
    void UpdatekinematicCameraRotationControllers( Range<EntityID const*> controllers, Range<float const *> camera_z_angles, EntityRotations & rotations );

    void UpdateCameraRotationRotationConstraintsControllers(
        Range<EntityID const*> controllers,
        Range<float const*> camera_z_angles,
        Range<float const*> torques,
        Range<unsigned const *> entity_to_torque,
        // IndexedVelocities const & indexed_velocities,
        RotationConstraints & constraints);

    void UpdateCameraRotationTorqueControllers( Range<EntityID const *> controllers, IndexedOrientations const & indexed_orientations, Range<float const*> powers, Range<unsigned const *> entity_to_power, Range<float const *> target_z_angles, float const time_step, AngularVelocityConstraints & angular_velocity_constraints );

    void UpdateMotionForceControllers(
        std::vector<EntityID> entity_ids,
        Range<Math::Float2 const *> horizontal_movements,
		std::vector<float> const & target_speed_factors,
        IndexedOrientations const & indexed_orientations,
        IndexedVelocities const & indexed_velocities,
        Range<unsigned const *> entity_to_power,
        Range<float const*> powers,
        Range<Math::Float2 const*> positive_biases,
        Range<Math::Float2 const*> negative_biases,
        Range<float const*> target_speeds,
        // output
        VelocityConstraints & velocity_constraints );


    // Puts all entities that have a surface beneath their feet at the front of the range and
    // returns the number of entities.
    uint32_t FilterEntitiesWhoHaveFeetTouchingASurface(
        CollisionEvents const & collision_events,
        Range<EntityID *> entity_ids
        );


    void UpdateJumpControllers(
        Range<EntityID const *> entity_ids,
        Range<unsigned const *> entity_to_force,
        Range<float const*> forces,
        EntityForces & entity_forces);


    void UpdateArrowkinematicControllers( Range<EntityID const *> entities, Range<Math::Float2 const *> horizontal_movements, IndexedOrientations const & indexed_orientations, float time_step, EntityPositions & positions );
}
