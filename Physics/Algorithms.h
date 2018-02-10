#pragma once
#include "BodyID.h"
#include <Conventions\EntityID.h>
#include <Conventions\Force.h>
#include <Conventions\Orientation.h>
#include <Conventions\Velocity.h>
#include <Math\FloatOperators.h>
#include <Utilities\Range.h>


namespace Physics
{
    struct Inertia;
    struct Movement;
    struct BodyEntityMapping;

    Math::Quaternion UpdateRotationWithAngularVelocity( 
        Math::Quaternion const & rotation, 
        Math::Float3 const & angular_velocity, 
        float const time_step 
        );

    Math::Quaternion UpdateQuaternionWithAngularVelocity(
        Math::Quaternion const & rotation,
        Math::Float3 const & angular_velocity,
        float const time_step
    );    

    void AddGravity( 
        Math::Float3 const gravity, 
        Range<Inertia const *> inverse_inertias, 
        Range<Math::Float3 *> forces 
        );

    void AddForces( 
        EntityForces const & entity_forces, 
        BodyEntityMapping const & mapping, 
        Range<uint32_t const *> indices, 
        Range<Math::Float3 *> forces 
        );

    Math::Float3 AngularVelocityFromRotation( 
        Math::Quaternion new_rotation, 
        Math::Quaternion original_rotation, 
        float time_step 
        );

    void ApplyMovement( 
        Range<Orientation*> new_orientation, 
        Range<Orientation const *> original_orientation, 
        Range<Movement const*> movement, 
        Range<Inertia const*> inverse_inertia, 
        float const time_step 
        );

    void UpdateOrientations( 
        Range<Orientation*> const orientation_range, 
        Range<uint32_t const*> const indices, 
        Range<Math::Float3 const *> const new_entity_positions, 
        Range<EntityID const*> const position_entity_ids 
        );

    void UpdateOrientations(
        Range<Orientation*> const orientation_range, 
        Range<uint32_t const*> const body_to_element, 
        BodyEntityMapping const & mapping, 
        Range<Math::Float3 const*> const new_entity_positions, 
        Range<EntityID const*> const position_entity_ids
        );

    void UpdateOrientations(
        Range<Orientation*> const orientation_range,
        Range<uint32_t const*> const body_to_element,
        BodyEntityMapping const & mapping,
        Range<Math::Quaternion const *> const new_entity_rotations,
        Range<EntityID const*> const entity_ids
        );

    void UpdateOrientations( 
        Range<Orientation*> const orientation_range, 
        Range<uint32_t const*> const indices, 
        Range<Math::Quaternion const *> const new_entity_rotations, 
        Range<EntityID const*> const entity_ids 
        );

    // correct the orientations, center of mass is in local coordinates
    // orientation - center of mass
    void CorrectForCenterOfMassBackward( 
        Range<Math::Float3 const *> center_of_mass, 
        Range<Orientation *> orientations 
        );

    // orientation + center of mass
    void CorrectForCenterOfMassForward( 
        Range<Math::Float3 const *> center_of_mass, 
        Range<Orientation *> orientations 
        );

    void CorrectForCenterOfMassForward( 
        Range<EntityID const *> ids, 
        Range<uint32_t const *> entity_to_data, 
        Range<Math::Float3 const *> center_of_mass, 
        Range<Orientation const *> orientation, 
        Range<Math::Float3 *> position 
        );

    void UpdateVelocities(
        std::vector<Velocity>& velocities, 
        std::vector<Math::Float3>& angular_velocities, 
        std::vector<EntityID>& velocity_entity_ids, 
        std::vector<uint32_t>& entity_to_element, 
        Range<Orientation const*> orientations, 
        Range<Orientation const*> previous_orientations, 
        Range<EntityID const*> entity_ids, 
        const float time_step
        );

    // for kinematic bodies
    void UpdateVelocities( 
        std::vector<Velocity>& velocities, 
        std::vector<Math::Float3>& angular_velocities, 
        std::vector<EntityID> & velocity_entity_ids, 
        std::vector<uint32_t>& entity_to_element, 
        Range<Orientation const * > orientations, 
        Range<Orientation const * > previous_orientations, 
        Range<BodyID const * > body_ids,
        BodyEntityMapping const & mapping, 
        const float time_step );
    
    // for rigidbodies
    void UpdateVelocities( 
        std::vector<Velocity>& velocities, 
        std::vector<Math::Float3>& angular_velocities, 
        std::vector<EntityID> & velocity_entity_ids, 
        std::vector<uint32_t>& entity_to_elements, 
        Range<Movement const*> movements, 
        Range<Inertia const *> inertias, 
        Range<BodyID const *> body_ids,
        BodyEntityMapping const & mapping
        );

    void ApplyForces( 
        Range<Movement const *> movements, 
        Range<Force const *> forces, 
        float const time_step, 
        Range<Movement *> result_movements 
        );

    std::vector<Movement> ApplyForces( 
        std::vector<Movement> movements, 
        std::vector<uint32_t> const & indices, 
        std::vector<Force> const & forces, 
        std::vector<EntityID> const & entity_ids, 
        float const time_step 
        );

    void ApplyTorques( 
        Range<Movement const *> movements, 
        Range<uint32_t const *> indices, 
        Range<Torque const *> torques, 
        Range<EntityID const *> entity_ids, 
        BodyEntityMapping const & mapping, 
        float const time_step, 
        Range<Movement *> result_movements 
        );

    Math::Float3 ImpulseFromMovement(
        Movement const & movement, 
        Math::Float3 const & position
        );

    Movement MovementFromImpulse(
        Math::Float3 const impulse, 
        Math::Float3 const position
        );

    Movement ApplyImpulse(
        Movement movement, 
        Math::Float3 const impulse, 
        Math::Float3 const position
        );

    void AddMovements(
        Range<Movement const *> movements1, 
        Range<Movement*> movements2, 
        float const angular_fraction
        );

    // position has to be in body coordinates
    Math::Float3 CalculateVelocity( 
        Movement const & movement, 
        Inertia const & inverse_inertia, 
        Math::Float3 const relative_position 
        );

    float CombineRestitutionFactors( 
        float factor1, 
        float factor2 
        );

    float CalculateInverseEffectiveMass( 
        Math::Float3 relative_position, 
        Math::Float3 normal, 
        Inertia const & inverse_inertia 
        );
}