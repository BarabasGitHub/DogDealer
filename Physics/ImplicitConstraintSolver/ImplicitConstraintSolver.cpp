#include "ImplicitConstraintSolver.h"

#include "MovementToEffectiveMovement.h"
#include "Constraints.h"
#include "ConstraintFunctions.h"
#include "EffectiveMass.h"
#include "Solve.h"

#include "../Algorithms.h"
#include "..\WorldConfiguration.h"
#include "..\CollisionEventOffsets.h"
#include "../Inertia.h"
#include "..\CollisionEventDecomposition.h"
#include "../CollisionEvent.h"

#include "..\Constraints.h"

#include <Math\MathFunctions.h>
#include <Math\VectorAlgorithms.h>

#include <Utilities\VectorHelper.h>
#include <Utilities\StdVectorFunctions.h>
#include <Utilities\Memory.h>
// #include <Utilities\HRTimer.h>
#include <Utilities\UnionFind.h>

#include <numeric>

using namespace Physics;

// don't complain about hiding member variables
#pragma warning( disable : 4458 )

ImplicitConstraintSolver::ImplicitConstraintSolver() :
    ConstraintSolver(),
    relaxation_factor(1, 1),
    warm_start_factor(1.f),
    solve_parallel(false),
    minimal_island_size(128)
{}


ImplicitConstraintSolver::~ImplicitConstraintSolver()
{}


void ImplicitConstraintSolver::SetConfiguration(MinMax<float> relaxation_factor, float warm_start_factor, uint32_t minimal_island_size, bool solve_parallel)
{
    this->relaxation_factor = relaxation_factor;
    this->warm_start_factor = warm_start_factor;
    this->minimal_island_size = minimal_island_size;
    this->solve_parallel = solve_parallel;
}


void ImplicitConstraintSolver::SetConfigurationFromWorldConfiguration(WorldConfiguration const & world_config)
{
    SetCommonConfiguration(
        world_config.position_correction_fraction,
        world_config.penetration_depth_tolerance,
        world_config.angular_velocity_correction_fraction,
        world_config.velocity_correction_iterations,
        world_config.position_correction_iterations);
    SetConfiguration(world_config.solver_relaxation_factor, world_config.warm_start_factor, world_config.minimal_island_size, world_config.solve_parallel);
}

namespace
{
    void ApplyWarmStartImpulse(Range<Constraint const*> constraints, Range<Movement*> movements)
    {
        auto count = Size(constraints);
        for( auto i = 0u; i < count; ++i )
        {
            auto & c = constraints[i];
            movements[c.body_indices[0]] += c.total_impulse * c.unit_impulses[0];
            movements[c.body_indices[1]] += c.total_impulse * c.unit_impulses[1];
        }
    }


    void ApplyWarmStartImpulse(Range<SingleBodyConstraint const*> constraints, Range<Movement*> movements)
    {
        auto count = Size(constraints);
        for( auto i = 0u; i < count; ++i )
        {
            auto & c = constraints[i];
            movements[c.body_index] += c.total_impulse * c.unit_impulse;
        }
    }

    void ExpandBodiesReverse(Range<BodyPair const*> bodies, Range<uint32_t const*> offsets, Range<BodyPair *> output)
    {
        assert(Size(bodies) + 1 == Size(offsets));
        assert(Size(output) == Last(offsets));
        auto count = Size(bodies);
        for( auto i = count; i > 0; )
        {
            i--;
            auto body_pair = bodies[i];
            for( auto j = offsets[i + 1]; j > offsets[i]; )
            {
                --j;
                output[j] = body_pair;
            }
        }
    }


    void ExpandBodies(Range<BodyPair const*> bodies, Range<uint32_t const*> offsets, Range<BodyPair *> output)
    {
        assert(Size(bodies) + 1 == Size(offsets));
        assert(Size(output) == Last(offsets));
        auto count = Size(bodies);
        for( auto i = 0u; i < count; ++i )
        {
            auto body_pair = bodies[i];
            for( auto j = offsets[i]; j < offsets[i + 1]; ++j)
            {
                output[j] = body_pair;
            }
        }
    }


    void CalculateRelativeVelocities(
        Range<BodyPair const*> body_pairs,
        Range<std::array<Math::Float3, 2> const*> relative_positions,
        Range<uint32_t const*> rigid_body_to_element,
        Range<Movement const*> movements,
        Range<Inertia const*> inverse_inertias,
        Range<Math::Float3 *> relative_velocities
        )
    {
        assert(Size(body_pairs) == Size(relative_positions));
        assert(Size(body_pairs) == Size(relative_velocities));
        assert(Size(movements) == Size(inverse_inertias));
        auto count = Size(body_pairs);
        for( auto i = 0u; i < count; ++i )
        {
            auto body_pair = body_pairs[i];
            auto index0 = rigid_body_to_element[body_pair.id1.index];
            auto index1 = GetOptional(rigid_body_to_element, body_pair.id2.index);
            auto velocity0 = CalculateVelocity(movements[index0], inverse_inertias[index0], relative_positions[i][0]);
            Math::Float3 velocity1 = 0;
            if(index1 != c_invalid_index)
            {
                velocity1 = CalculateVelocity(movements[index1], inverse_inertias[index1], relative_positions[i][1]);
            }
            relative_velocities[i] = velocity1 - velocity0;
        }
    }
}


void ImplicitConstraintSolver::MakeRigidStaticCollisionAndFrictionConstraints(float const time_step)
{
    auto rigid_static_bodies = CreateRigidStaticRange(*this->event_offsets, this->collision_events->bodies);
    auto rigid_static_relative_positions = CreateRigidStaticRange(*this->event_offsets, this->collision_events->relative_positions);
    auto rigid_static_manifolds = CreateRigidStaticRange(*this->event_offsets, this->collision_events->manifolds);

    // calculate contact_point_data_offsets for the static collision events
    {
        ResetSize(this->contact_point_data_offsets, Size(rigid_static_bodies) + 1);
        First(this->contact_point_data_offsets) = 0;
        GetContactPointCounts(rigid_static_manifolds, CreateRange(this->contact_point_data_offsets, 1, Size(rigid_static_bodies) + 1));
        std::partial_sum(begin(this->contact_point_data_offsets), end(this->contact_point_data_offsets), begin(this->contact_point_data_offsets));
    }
    auto total_contacts = Last(this->contact_point_data_offsets);
    {
        ResetSize(this->body_pairs, total_contacts);
        ExpandBodies(rigid_static_bodies, this->contact_point_data_offsets, this->body_pairs);

        ResetSize(this->collision_normals, total_contacts);
        ResetSize(this->penetration_depths, total_contacts);
        ResetSize(this->relative_positions, total_contacts);
        CopyPerPointProperties(
            rigid_static_manifolds,
            rigid_static_relative_positions,
            this->collision_normals,
            this->penetration_depths,
            this->relative_positions
            );
    }

    {
        ResetSize(this->old_relative_velocities, total_contacts);
        CalculateRelativeVelocities(
                this->body_pairs,
                this->relative_positions,
                this->rigid_body_to_element,
                this->rigid_body_old_movements,
                this->rigid_body_inverse_inertias,
                this->old_relative_velocities
                );
        ResetSize(this->new_relative_velocities, total_contacts);
        CalculateRelativeVelocities(
                this->body_pairs,
                this->relative_positions,
                this->rigid_body_to_element,
                this->rigid_body_new_movements,
                this->rigid_body_inverse_inertias,
                this->new_relative_velocities
                );
    }

    auto added_constraints = Grow(this->velocity.single_body_constraints, total_contacts);
    CalculateContactConstraints(
        this->body_pairs,
        this->collision_normals,
        this->relative_positions,
        this->old_relative_velocities,
        this->new_relative_velocities,
        this->body_to_element,
        this->restitution_factors,
        this->rigid_body_to_element,
        this->rigid_body_inverse_inertias,
        this->warm_start_factor,
        added_constraints
        );

    added_constraints = Grow(this->position.single_body_constraints, total_contacts);
    // CalculateSoftNonPenetrationConstraints(
    //     this->body_pairs,
    //     this->collision_normals,
    //     this->relative_positions,
    //     this->penetration_depths,
    //     this->rigid_body_to_element,
    //     this->rigid_body_inverse_inertias,
    //     this->penetration_depth_tolerance,
    //     1.f,
    //     1.f / this->position_correction_fraction,
    //     time_step,
    //     this->warm_start_factor,
    //     added_constraints
    //     );
    CalculateNonPenetrationConstraints(
        this->body_pairs,
        this->collision_normals,
        this->relative_positions,
        this->penetration_depths,
        this->rigid_body_to_element,
        this->rigid_body_inverse_inertias,
        this->position_correction_fraction,
        this->penetration_depth_tolerance,
        time_step,
        this->warm_start_factor,
        added_constraints
        );

    added_constraints = Grow(this->velocity.single_body_constraints, total_contacts);
    auto added_count = CalculateFrictionConstraints(
        this->body_pairs,
        this->collision_normals,
        this->relative_positions,
        this->new_relative_velocities,
        this->body_to_element,
        this->friction_factors,
        this->rigid_body_to_element,
        this->rigid_body_forces,
        this->rigid_body_inverse_inertias,
        time_step,
        this->warm_start_factor,
        added_constraints
        );
    Shrink(this->velocity.single_body_constraints, total_contacts - added_count);
}


void ImplicitConstraintSolver::MakeRigidRigidCollisionAndFrictionConstraints(float const time_step)
{
    auto rigid_rigid_entities = CreateRigidRigidRange(*this->event_offsets, this->collision_events->bodies);
    auto rigid_rigid_relative_positions = CreateRigidRigidRange(*this->event_offsets, this->collision_events->relative_positions);
    auto rigid_rigid_manifolds = CreateRigidRigidRange(*this->event_offsets, this->collision_events->manifolds);

    // calculate contact_point_data_offsets for the static collision events
    {
        ResetSize(this->contact_point_data_offsets, Size(rigid_rigid_entities) + 1);
        First(this->contact_point_data_offsets) = 0;
        GetContactPointCounts(rigid_rigid_manifolds, CreateRange(this->contact_point_data_offsets, 1, Size(rigid_rigid_entities) + 1));
        std::partial_sum(begin(this->contact_point_data_offsets), end(this->contact_point_data_offsets), begin(this->contact_point_data_offsets));
    }
    auto total_contacts = Last(this->contact_point_data_offsets);
    {
        ResetSize(this->body_pairs, total_contacts);
        ExpandBodies(rigid_rigid_entities, this->contact_point_data_offsets, this->body_pairs);

        ResetSize(this->collision_normals, total_contacts);
        ResetSize(this->penetration_depths, total_contacts);
        ResetSize(this->relative_positions, total_contacts);
        CopyPerPointProperties(
            rigid_rigid_manifolds,
            rigid_rigid_relative_positions,
            this->collision_normals,
            this->penetration_depths,
            this->relative_positions
            );
    }

    {
        ResetSize(this->old_relative_velocities, total_contacts);
        CalculateRelativeVelocities(
                this->body_pairs,
                this->relative_positions,
                this->rigid_body_to_element,
                this->rigid_body_old_movements,
                this->rigid_body_inverse_inertias,
                this->old_relative_velocities
                );
        ResetSize(this->new_relative_velocities, total_contacts);
        CalculateRelativeVelocities(
                this->body_pairs,
                this->relative_positions,
                this->rigid_body_to_element,
                this->rigid_body_new_movements,
                this->rigid_body_inverse_inertias,
                this->new_relative_velocities
                );
    }

    auto added_constraints = Grow(this->velocity.constraints, total_contacts);
    CalculateContactConstraints(
        this->body_pairs,
        this->collision_normals,
        this->relative_positions,
        this->old_relative_velocities,
        this->new_relative_velocities,
        this->body_to_element,
        this->restitution_factors,
        this->rigid_body_to_element,
        this->rigid_body_inverse_inertias,
        this->warm_start_factor,
        added_constraints
        );

    added_constraints = Grow(this->position.constraints, total_contacts);
    // CalculateSoftNonPenetrationConstraints(
    //     this->body_pairs,
    //     this->collision_normals,
    //     this->relative_positions,
    //     this->penetration_depths,
    //     this->rigid_body_to_element,
    //     this->rigid_body_inverse_inertias,
    //     this->penetration_depth_tolerance,
    //     1.f,
    //     1.f / this->position_correction_fraction,
    //     time_step,
    //     this->warm_start_factor,
    //     added_constraints
    //     );
    CalculateNonPenetrationConstraints(
        this->body_pairs,
        this->collision_normals,
        this->relative_positions,
        this->penetration_depths,
        this->rigid_body_to_element,
        this->rigid_body_inverse_inertias,
        this->position_correction_fraction,
        this->penetration_depth_tolerance,
        time_step,
        this->warm_start_factor,
        added_constraints
        );

    added_constraints = Grow(this->velocity.constraints, total_contacts);
    auto added_count = CalculateFrictionConstraints(
        this->body_pairs,
        this->collision_normals,
        this->relative_positions,
        this->new_relative_velocities,
        this->body_to_element,
        this->friction_factors,
        this->rigid_body_to_element,
        this->rigid_body_forces,
        this->rigid_body_inverse_inertias,
        time_step,
        this->warm_start_factor,
        added_constraints
        );
    Shrink(this->velocity.constraints, total_contacts - added_count);
}



namespace
{
    uint32_t GetBodyIndexForGrouping(SingleBodyConstraint const & c)
    {
        return c.body_index;
    }


    uint32_t GetBodyIndexForGrouping(Constraint const & c)
    {
        return c.body_indices[0];
    }


    // assumes the offsets are two larger than the maximum group number (one larger than it actually has to be, but this is easier)
    template <typename ConstraintType>
    void GroupAndReorderConstraints(
        Range<ConstraintType const*> constraints,
        Range<uint32_t const *> island_group_numbers,
        Range<uint32_t *> offsets,
        Range<ConstraintType *> reordered_constraints
        )
    {
        assert(Size(constraints) == Size(reordered_constraints));
        Zero(begin(offsets), Size(offsets));
        // count the constraints per group
        auto count = Size(constraints);
        for( auto i = 0u; i < count; ++i )
        {
            offsets[island_group_numbers[GetBodyIndexForGrouping(constraints[i])] + 1] += 1;
        }
        // turn them into offsets and move one forward
        {
            auto sum = 0u;
            for(auto i = 1u; i < Size(offsets); i++)
            {
                auto current = offsets[i];
                offsets[i] = sum;
                sum += current;
            }
        }

        // move to the new positions
        for( auto i = 0u; i < count; ++i )
        {
            auto & c = constraints[i];
            // get island number
            auto island_number = island_group_numbers[GetBodyIndexForGrouping(c)];
            // get the new position for this constraint
            auto offset = offsets[island_number + 1];
            // increase the position for the next
            offsets[island_number + 1] += 1;
            // copy the constraint to it's new position
            reordered_constraints[offset] = c;
        }
    }


    void GroupRigidBodies(Range<Constraint const*> constraints, Range<uint32_t *> groups)
    {
        // do union find stuff
        InitializeUnionFind(groups);
        auto constraint_count = Size(constraints);
        for( auto i = 0u; i < constraint_count; ++i )
        {
            auto body_indices = constraints[i].body_indices;
            Unite(body_indices[0], body_indices[1], groups);
        }

        // set all island groups to their root, so they point to their group number directly
        for( auto i = 0u; i < Size(groups); ++i )
        {
            groups[i] = GetRoot(groups, i);
        }
    }


    // renumber the islands to correspond with the offsets
    void RenumberGroups(Range<Constraint const *> constraints, Range<uint32_t const*> offsets, Range<uint32_t *> groups)
    {
        Zero(begin(groups), Size(groups));
        for( auto i = 0u; i < Size(offsets) - 1; ++i )
        {
            for( auto j = offsets[i]; j < offsets[i + 1]; ++j )
            {
                auto body_indices = constraints[j].body_indices;
                groups[body_indices[0]] = groups[body_indices[1]] = i;
            }
        }
    }

    // returns the number of removed islands
    uint32_t RemoveSmallIslands(uint32_t minimal_island_size, Range<uint32_t *> island_offsets)
    {
        auto source = begin(island_offsets) + 1;
        auto offsets_end = end(island_offsets);
        while(source < offsets_end && (source[0] - source[-1]) >= minimal_island_size)
        {
            ++source;
        }
        auto destination = begin(island_offsets) + 1;
        while(source < offsets_end)
        {
            while(source < (offsets_end - 1) && (source[0] - destination[-1]) < minimal_island_size)
            {
                ++source;
            }
            *destination = *source;
            ++source;
            ++destination;
        }
        return uint32_t(offsets_end - destination);
    }
}


void ImplicitConstraintSolver::MakeIslands()
{
    // static HRTimer island_timer;
    // island_timer.Start();

    ResetSize(this->island_marks, Size(this->rigid_body_orientations));
    GroupRigidBodies(this->velocity.constraints, this->island_marks);

    ResetSize(this->island_offsets, Size(this->rigid_body_orientations) + 1);
    ResetSize(this->temp, Size(this->velocity.constraints));
    GroupAndReorderConstraints<Constraint>(
        this->velocity.constraints,
        this->island_marks,
        this->island_offsets,
        this->temp
        );
    swap(this->temp, this->velocity.constraints);
    // remove empty groups
    auto small_island_count = RemoveSmallIslands(this->minimal_island_size, this->island_offsets);
    Shrink(this->island_offsets, small_island_count);
    RenumberGroups(this->velocity.constraints, this->island_offsets, this->island_marks);

    auto number_of_islands = Size(this->island_offsets) - 1;
    ResetSize(this->non_penetration_island_offset, number_of_islands + 1);
    ResetSize(this->temp, Size(this->position.constraints));
    GroupAndReorderConstraints<Constraint>(
            this->position.constraints,
            this->island_marks,
            this->non_penetration_island_offset,
            this->temp);
    swap(this->temp, this->position.constraints);

    ResetSize(this->single_body_island_offsets, number_of_islands + 1);
    ResetSize(this->single_body_temp, Size(this->velocity.single_body_constraints));
    GroupAndReorderConstraints<SingleBodyConstraint>(
            this->velocity.single_body_constraints,
            this->island_marks,
            this->single_body_island_offsets,
            this->single_body_temp);
    swap(this->single_body_temp, this->velocity.single_body_constraints);

    ResetSize(this->non_penetrating_single_body_island_offsets, number_of_islands + 1);
    ResetSize(this->single_body_temp, Size(this->position.single_body_constraints));
    GroupAndReorderConstraints<SingleBodyConstraint>(
            this->position.single_body_constraints,
            this->island_marks,
            this->non_penetrating_single_body_island_offsets,
            this->single_body_temp);
    swap(this->single_body_temp, this->position.single_body_constraints);

    // island_timer.Stop();
    // auto islanding_time = island_timer.GetMilliSeconds();
    // Log([&]()
    // {
    //     return "Found " + std::to_string(island_offsets.size() - 1) + " islands from " + std::to_string(this->constraints.size()) + " constraints in " + std::to_string(islanding_time) + " milliseconds.";
    // });
}


void ImplicitConstraintSolver::DoYourThing(float const time_step)
{
    // clear
    this->position_correction.clear();
    this->movement_correction.clear();
    this->body_pairs.clear();
    this->collision_normals.clear();
    this->contact_point_data_offsets.clear();
    this->penetration_depths.clear();
    this->relative_positions.clear();
    this->old_relative_velocities.clear();
    this->new_relative_velocities.clear();
    this->velocity.constraints.clear();
    this->velocity.single_body_constraints.clear();
    this->position.constraints.clear();
    this->position.single_body_constraints.clear();

    // make constraints
    MakeRigidStaticCollisionAndFrictionConstraints(time_step);
    MakeRigidRigidCollisionAndFrictionConstraints(time_step);

    auto added_constraints = Grow(this->position.constraints, Size(this->position_constraints->bodies));
	
    //CalculateSoftPositionConstraints(
    //   this->position_constraints->bodies,
    //   this->position_constraints->directions,
    //   this->position_constraints->attachment_points,
    //   this->position_constraints->distances,
    //   this->rigid_body_to_element,
    //   this->rigid_body_orientations,
    //   this->rigid_body_new_movements,
    //   this->rigid_body_inverse_inertias,
    //   1.0f,
    //   10.0f,
    //   time_step,
    //   this->warm_start_factor,
    //   added_constraints);
	   
     CalculatePositionConstraints(
        this->position_constraints->bodies,
        this->position_constraints->directions,
        this->position_constraints->attachment_points,
        this->position_constraints->distances,
        this->rigid_body_to_element,
        this->rigid_body_orientations,
        this->rigid_body_new_movements,
        this->rigid_body_inverse_inertias,
        time_step,
        this->warm_start_factor,
        added_constraints);


    added_constraints = Grow(this->velocity.constraints, Size(this->distance_constraints->bodies));
    CalculateDistanceConstraints(
        this->distance_constraints->bodies,
        this->distance_constraints->attachment_points,
        this->distance_constraints->distances,
        this->rigid_body_to_element,
        this->rigid_body_orientations,
        this->rigid_body_new_movements,
        this->rigid_body_inverse_inertias,
        //0.5f,
        //5.f,
        time_step,
        this->warm_start_factor,
        added_constraints);

    added_constraints = Grow(this->velocity.constraints, Size(this->rotation_constraints->bodies));
    CalculateRotationConstraints(
        this->rotation_constraints->bodies,
        this->rotation_constraints->rotation_normals,
        this->rotation_constraints->target_angles,
        // Range<float const *> distances,
        this->rigid_body_to_element,
        this->rigid_body_orientations,
        this->rigid_body_new_movements,
        this->rigid_body_inverse_inertias,
        //0.5f,
        //5.f,
        time_step,
        this->warm_start_factor,
        added_constraints);

    added_constraints = Grow(this->velocity.constraints, Size(this->velocity_constraints->bodies));
    CalculateVelocityConstraints(
        this->velocity_constraints->bodies,
        this->velocity_constraints->directions,
        this->velocity_constraints->attachment_points,
        this->velocity_constraints->target_speeds,
        this->velocity_constraints->minmax_forces,
        this->rigid_body_to_element,
        this->rigid_body_orientations,
        this->rigid_body_new_movements,
        this->rigid_body_inverse_inertias,
        time_step,
        this->warm_start_factor,
        added_constraints
        );

    auto added_single_body_constraints = Grow(this->velocity.single_body_constraints, Size(this->world_velocity_constraints->body_ids));
    CalculateSoftVelocityConstraints(
        this->world_velocity_constraints->body_ids,
        this->world_velocity_constraints->directions,
        this->world_velocity_constraints->target_speeds,
        this->world_velocity_constraints->minmax_force,
        this->rigid_body_to_element,
        this->rigid_body_new_movements,
        this->rigid_body_inverse_inertias,
        time_step,
        this->warm_start_factor,
        0.0f,
        1.f/time_step,
        added_single_body_constraints
        );
    // CalculateVelocityConstraints(
    //     this->world_velocity_constraints->entity_ids,
    //     this->world_velocity_constraints->directions,
    //     this->world_velocity_constraints->target_speeds,
    //     this->world_velocity_constraints->minmax_force,
    //     this->rigid_body_to_element,
    //     this->rigid_body_new_movements,
    //     this->rigid_body_inverse_inertias,
    //     time_step,
    //     this->warm_start_factor,
    //     added_constraints
    //     );


    added_single_body_constraints = Grow(this->velocity.single_body_constraints, Size(this->angular_velocity_constraints->body_ids));
    CalculateAngularVelocityConstraints(
        this->angular_velocity_constraints->body_ids,
        this->angular_velocity_constraints->angular_directions,
        this->angular_velocity_constraints->angular_target_speeds,
        this->angular_velocity_constraints->minmax_torque,
        this->rigid_body_to_element,
        this->rigid_body_new_movements,
        this->rigid_body_inverse_inertias,
        time_step,
        this->warm_start_factor,
        added_single_body_constraints
        );


    added_single_body_constraints = Grow(this->velocity.single_body_constraints, Size(this->world_rotation_constraints->body_ids));
    CalculateSoftRotationConstraints(
        this->world_rotation_constraints->body_ids,
        this->world_rotation_constraints->rotation_normals,
        this->world_rotation_constraints->target_angles,
        this->world_rotation_constraints->minmax_torques,
        this->rigid_body_to_element,
        this->rigid_body_orientations,
        this->rigid_body_new_movements,
        this->rigid_body_inverse_inertias,
        time_step,
        this->warm_start_factor,
        1.f /*damping_ratio*/,
        5.f /*frequency*/,
        added_single_body_constraints
        );

    auto use_parallel = this->solve_parallel && Size(this->velocity.constraints) > this->minimal_island_size;
    if(use_parallel)
    {
        MakeIslands();
    }

    // solve non-penetration position correction constraints
    ResetSize(this->position_correction, Size(this->rigid_body_inverse_inertias));
    Zero(this->position_correction.data(), Size(this->position_correction));
    if( this->warm_start_factor != 0 )
    {
        ApplyWarmStartImpulse(this->position.constraints, this->position_correction);
        ApplyWarmStartImpulse(this->position.single_body_constraints, this->position_correction);
    }

    if(use_parallel)
    {
        SolveIslands(
            this->position_correction_iterations,
            this->relaxation_factor,
            this->non_penetration_island_offset,
            this->non_penetrating_single_body_island_offsets,
            this->position.constraints,
            this->position.single_body_constraints,
            this->position_correction
            );
    }
    else
    {
#if _DEBUG
        Solve(
#else
        SolveSSE(
#endif
            this->position_correction_iterations,
            this->relaxation_factor,
            this->position.constraints,
            this->position.single_body_constraints,
            this->position_correction
            );
    }


    ResetSize(this->movement_correction, Size(this->rigid_body_inverse_inertias));
    Zero(this->movement_correction.data(), Size(this->movement_correction));

    if( this->warm_start_factor != 0 )
    {
        ApplyWarmStartImpulse(this->velocity.constraints, this->movement_correction);
        ApplyWarmStartImpulse(this->velocity.single_body_constraints, this->movement_correction);
    }

    if(use_parallel)
    {
        SolveIslands(
            this->velocity_correction_iterations,
            this->relaxation_factor,
            this->island_offsets,
            this->single_body_island_offsets,
            this->velocity.constraints,
            this->velocity.single_body_constraints,
            this->movement_correction
            );
    }
    else
    {
#if _DEBUG
        Solve(
#else
        SolveSSE(
#endif
            this->velocity_correction_iterations,
            this->relaxation_factor,
            this->velocity.constraints,
            this->velocity.single_body_constraints,
            this->movement_correction
            );
    }


    AddMovements(this->movement_correction, this->rigid_body_new_movements, 1);
    AddMovements(this->rigid_body_new_movements, this->position_correction, this->angular_velocity_correction_fraction);

    ApplyMovement(this->rigid_body_orientations, this->rigid_body_orientations, this->position_correction, this->rigid_body_inverse_inertias, time_step);
}
