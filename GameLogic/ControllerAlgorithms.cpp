#include "ControllerAlgorithms.h"

#include <Conventions\RotationConstraints.h>
#include <Conventions\VelocityConstraints.h>
#include <Conventions\Force.h>
#include <Conventions\Orientation.h>
#include <Conventions\CollisionEvent.h>

#include <Math\FloatOperators.h>
#include <Math\MathFunctions.h>
#include <Math\TransformFunctions.h>

#include <Utilities\Logger.h>
#include <Utilities\VectorHelper.h>
#include <Utilities\StdVectorFunctions.h>

#include <algorithm>

using namespace Logic;

void Logic::UpdateArrowkinematicControllers( Range<EntityID const *> entities, Range<Math::Float2 const *> horizontal_movements, IndexedOrientations const & indexed_orientations, float time_step, EntityPositions & new_positions )
    {
        assert( Size( entities ) == Size( horizontal_movements ) );
        new_positions.positions.reserve( new_positions.positions.size() + Size(entities) );
        auto size = Size(entities);
        for( auto i = 0u; i < size; ++i )
        {
            auto entity_id = entities[i];
            auto horizontal_movement = horizontal_movements[i];
            // Get current entities orientation from input vector
            auto const orientation_index = indexed_orientations.indices[entity_id.index];
            auto const & orientation = indexed_orientations.orientations[orientation_index];

            // TODO: Get proper movement distance for entity type
            auto distance = 20.0f * time_step;
            auto movement = Math::Float3( horizontal_movement.x*distance, horizontal_movement.y*distance, 0.f );
            auto local_movement = Math::Rotate( movement, orientation.rotation );
            auto new_position = orientation.position + local_movement;

            new_positions.positions.emplace_back( new_position );
        }

        Append(new_positions.entity_ids, entities );
    }


void Logic::UpdateMotionForceControllers( std::vector<EntityID> entity_ids,
                                       Range<Math::Float2 const *> horizontal_movements,
									  std::vector<float> const & target_speed_factors,
                                       IndexedOrientations const & indexed_orientations,
                                       IndexedVelocities const & indexed_velocities,
                                       Range<unsigned const *> entity_to_data,
                                       Range<float const*> powers,
                                       Range<Math::Float2 const*> positive_biases,
                                       Range<Math::Float2 const*> negative_biases,
                                       Range<float const*> target_speeds,
                                       VelocityConstraints & velocity_constraints  )
{
    assert(Size(entity_ids) == Size(horizontal_movements));
    assert(Size(entity_ids) == Size(target_speed_factors));

    assert(Size(velocity_constraints.directions) == Size(velocity_constraints.entity_ids));
    assert(Size(velocity_constraints.entity_ids) == Size(velocity_constraints.minmax_force));
    assert(Size(velocity_constraints.minmax_force) == Size(velocity_constraints.target_speeds));

    auto size = Size( entity_ids );
    auto directions = Grow(velocity_constraints.directions, size * 2);
    auto output_entity_ids = Grow(velocity_constraints.entity_ids, size * 2);
    auto minmax_force = Grow(velocity_constraints.minmax_force, size * 2);
    auto output_target_speeds = Grow(velocity_constraints.target_speeds, size * 2);
    // iterate
    for( auto i = 0u; i < size; i++)
    {
        auto entity_id = entity_ids[i];
        auto horizontal_movement = horizontal_movements[i];

        auto const velocity = GetOptionalVelocity( indexed_velocities, entity_id, 0 );

        // TODO: Get proper power for entity; Isn't this proper then?
        auto data_index = entity_to_data[entity_id.index];
        auto const power = powers[data_index];
        auto const speed = Norm( velocity );

        Log([speed, entity_id](){
            return "EntityID: " + std::to_string(entity_id.index) + " is moving at a speed of: " + std::to_string(speed) + " m/s.";
        } );

        // power / speed = force
        // energy / distance = force
        auto scalar_force = power / Math::Max( speed, 0.5f );
        Math::Float2 bias;
        for( auto j = 0u; j < 2; ++j )
        {
            bias[j] = horizontal_movement[j] >= 0 ? positive_biases[data_index][j] : negative_biases[data_index][j];
        }
        auto force = Math::Abs(scalar_force * bias);

        // Get current entities orientation from input vector
        auto const orientation_index = indexed_orientations.indices[entity_id.index];
        auto const orientation = indexed_orientations.orientations[orientation_index];

        auto target_speed = target_speeds[data_index];
		auto target_speed_factor = target_speed_factors[i];
		target_speed *= target_speed_factor;

        auto horizontal_target_speeds = horizontal_movement * target_speed;
        auto direction1 = Rotate( Math::Float3(1, 0, 0), orientation.rotation );
        auto direction2 = Rotate( Math::Float3(0, 1, 0), orientation.rotation );
        auto contraint_index1 = i * 2;
        auto contraint_index2 = contraint_index1 + 1;
        directions[contraint_index1] = direction1;
        directions[contraint_index2] = direction2;
        minmax_force[contraint_index1] = {-force.x, force.x};
        minmax_force[contraint_index2] = {-force.y, force.y};
        output_entity_ids[contraint_index2] = output_entity_ids[contraint_index1] = entity_id;
        output_target_speeds[contraint_index1] = horizontal_target_speeds.x;
        output_target_speeds[contraint_index2] = horizontal_target_speeds.y;
    }
}


void Logic::UpdatekinematicCameraRotationControllers( Range<EntityID const*> controllers, Range<float const *> camera_z_angles, EntityRotations & rotations )
{
    if(IsEmpty(controllers)) return;
    Append(rotations.entity_ids, controllers);
    auto added_rotations = Grow(rotations.rotations, Size(controllers));
    std::transform(begin(camera_z_angles), end(camera_z_angles), begin(added_rotations), Math::ZAngleToQuaternion);
}


void Logic::UpdateCameraRotationRotationConstraintsControllers(
    Range<EntityID const*> controllers,
    Range<float const*> target_z_angles,
    Range<float const*> torques,
    Range<unsigned const *> entity_to_torque,
    // IndexedVelocities const & indexed_velocities,
    RotationConstraints & constraints)
{
    assert(Size(controllers) == Size(target_z_angles));
    Append(constraints.entity_ids, controllers);
    Append(constraints.target_angles, target_z_angles);
    Append(constraints.rotation_normals, Math::Float3(0,0,1), Size(controllers));
    auto minmax_torques = Grow(constraints.minmax_torques, Size(controllers));
    for( auto i = 0u; i < Size(controllers); ++i )
    {
        auto torque = torques[entity_to_torque[controllers[i].index]];
        // auto angular_velocity = GetOptionalAngularVelocity(indexed_velocities, controllers[i], 0);
        // auto angular_speed = Math::Abs(angular_velocity.z);
        // auto max_angular_speed = Math::Max(angular_speed, +0.5f);
        // auto min_angular_speed = Math::Min(angular_speed, -0.5f);
        // auto max_force = power / max_angular_speed;
        // auto min_force = power / min_angular_speed;
        // minmax_torques[i] = {min_force, max_force};

        minmax_torques[i] = {-torque, torque};
    }
}


void Logic::UpdateCameraRotationTorqueControllers( Range<EntityID const *> controllers, IndexedOrientations const & indexed_orientations, Range<float const*> powers, Range<unsigned const *> entity_to_power, Range<float const *> target_z_angles, float const time_step, AngularVelocityConstraints & angular_velocity_constraints )
{
    using namespace Math;

    assert(Size(angular_velocity_constraints.entity_ids) == Size(angular_velocity_constraints.angular_directions));
    assert( Size( angular_velocity_constraints.angular_directions ) == Size( angular_velocity_constraints.angular_target_speeds ) );
    assert( Size( angular_velocity_constraints.angular_target_speeds ) == Size( angular_velocity_constraints.minmax_torque ) );

    auto old_size = Size( angular_velocity_constraints.entity_ids );
    auto new_size = old_size + Size( controllers );
    // reserve space
    angular_velocity_constraints.minmax_torque.reserve( new_size );
    angular_velocity_constraints.angular_target_speeds.reserve( new_size );
    // set to
    angular_velocity_constraints.angular_directions.resize( new_size, Math::Float3( 0, 0, 1 ) );
    // copy
    Append(angular_velocity_constraints.entity_ids, controllers );
    for( auto i = 0u; i < Size( controllers ); ++i )
    {
        auto entity_id = controllers[i];
        // Get current entities orientation from input vector
        auto const orientation_index = indexed_orientations.indices[entity_id.index];
        auto const orientation = indexed_orientations.orientations[orientation_index];
        auto const rotation = orientation.rotation;

        auto z_angle = ZAngle(rotation);

        auto relative_angle = std::remainder( target_z_angles[i] - z_angle, Math::c_2PI.f );
        auto target_speed = relative_angle / time_step;
        target_speed = Clamp(-3.f, 3.f, target_speed);
        auto const power = powers[entity_to_power[entity_id.index]];

        angular_velocity_constraints.minmax_torque.emplace_back( -power, power );
        angular_velocity_constraints.angular_target_speeds.emplace_back( target_speed );
    }
}


uint32_t Logic::FilterEntitiesWhoHaveFeetTouchingASurface(
    CollisionEvents const & collision_events,
    Range<EntityID *> entity_ids
    )
{
    // TODO: make a better implementation, looping over all events is crap
    return uint32_t(std::remove_if(begin(entity_ids), end(entity_ids), [&]
        (EntityID id)
        {
            for( auto i = 0u; i < Size(collision_events); ++i )
            {
                auto pair = collision_events.entities[i];
                if(pair.id1 == id)
                {
                    auto & manifold = collision_events.manifolds[i];
                    for( auto j = 0u; j < manifold.contact_point_count; ++j )
                    {
                        if(manifold.separation_axes[j].z > 0.5f)
                        {
                            return false;
                        }
                    }
                }
                else if(pair.id2 == id)
                {
                    auto & manifold = collision_events.manifolds[i];
                    for( auto j = 0u; j < manifold.contact_point_count; ++j )
                    {
                        if(manifold.separation_axes[j].z < -0.5f)
                        {
                            return false;
                        }
                    }
                }
            }
            return true;
        }) - begin(entity_ids));
}


void Logic::UpdateJumpControllers(
        Range<EntityID const *> entity_ids,
        Range<unsigned const *> entity_to_force,
        Range<float const*> forces,
        EntityForces & entity_forces)
    {
        Append(entity_forces.entity_ids, entity_ids);
        for( auto i = 0u; i < Size(entity_ids); ++i )
        {
            auto index = entity_to_force[entity_ids[i].index];
            auto up_force = forces[index];
            Math::Float3 force = {0, 0, up_force};
            Append(entity_forces.forces, force);
        }
    }
