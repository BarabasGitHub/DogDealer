#pragma once

#include "..\ConstraintSolver.h"

#include "../BodyID.h"
#include <Utilities\MinMax.h>

#include <vector>
#include <array>

namespace Physics
{
    struct Constraint;
    struct SingleBodyConstraint;

    class ImplicitConstraintSolver final : public ConstraintSolver
    {
    public:
        ImplicitConstraintSolver();
        ImplicitConstraintSolver(ImplicitConstraintSolver const &) = delete;
        ImplicitConstraintSolver& operator=(ImplicitConstraintSolver const &) = delete;
        ~ImplicitConstraintSolver();

        void SetConfiguration(MinMax<float> relaxation_factor, float warm_start_factor, uint32_t minimal_island_size, bool solve_parallel);

        void SetConfigurationFromWorldConfiguration(WorldConfiguration const & world_config) override;

        void DoYourThing(float time_step) override;

    private:

        void MakeRigidStaticCollisionAndFrictionConstraints(float time_step);
        void MakeRigidRigidCollisionAndFrictionConstraints(float time_step);
        void MakeIslands();

        MinMax<float> relaxation_factor;
        float warm_start_factor;
        bool solve_parallel;
        uint32_t minimal_island_size;

        std::vector<Movement> position_correction, movement_correction;

        std::vector<BodyPair> body_pairs;
        std::vector<Math::Float3> collision_normals;
        std::vector<uint32_t> contact_point_data_offsets;
        std::vector<float> penetration_depths;
        std::vector<std::array<Math::Float3, 2>> relative_positions;
        std::vector<Math::Float3> old_relative_velocities, new_relative_velocities;

        struct SolverConstraints
        {
            std::vector<Constraint> constraints;
            std::vector<SingleBodyConstraint> single_body_constraints;
        };

        SolverConstraints velocity;
        SolverConstraints position;

        // island related stuff
        std::vector<uint32_t> island_offsets;
        std::vector<uint32_t> constraint_indices;
        std::vector<uint32_t> island_marks;
        std::vector<Constraint> temp;
        std::vector<uint32_t> non_penetration_island_offset;
        std::vector<uint32_t> single_body_island_offsets;
        std::vector<SingleBodyConstraint> single_body_temp;
        std::vector<uint32_t> non_penetrating_single_body_island_offsets;
    };


}
