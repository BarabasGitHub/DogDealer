#pragma once

#include <Utilities\MinMax.h>

namespace Physics
{
    enum struct ConstraintSolverType : uint8_t;

    struct WorldConfiguration
    {
        // The fraction of the penetration that is resolved each tick
        float position_correction_fraction;
        // The amount of penetration that won't be resolved
        // (or should we enlarge the bounds to make it like: The invisible extra layer applied to the bounds to detect collisions)
        float penetration_depth_tolerance;
        // maximum number of iterations for the position correction step (resolving the penetrations)
        uint32_t position_correction_iterations;
        // maximum number of iterations for the velocity correction step (preventing further penetration and bouncing)
        uint32_t velocity_correction_iterations;
        // the fraction of angular velocity resulting from the position correction that is actually applied
        float angular_velocity_correction_fraction;
        // the fraction of velocity loss per second
        // this allows everything to gradually slow down, such that thing scan properly come to rest and such.
        float fixed_fraction_velocity_loss_per_second;
        // the number of ticks a contact point is kept alive
        uint8_t persitent_contact_expiry_age;
        // type of the constraint solver
        ConstraintSolverType constraint_solver_type;
        // not always available, depends on the solver
        bool solve_parallel;
        // only for parallel solving
        uint32_t minimal_island_size;
        // how much of the initial guess should be started with when solving the impulses for the constraints
        float warm_start_factor;
        // relaxation factors, starting at max and gradually moving towards min at the max iteration
        MinMax<float> solver_relaxation_factor;
    };
}