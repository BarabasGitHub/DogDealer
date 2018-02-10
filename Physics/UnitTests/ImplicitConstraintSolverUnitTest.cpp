#include "CppUnitTest.h"

#include "Physics\ImplicitConstraintSolver\ImplicitConstraintSolver.h"
#include <Conventions\OrientationFunctions.h>
#include <Conventions\RotationConstraints.h>
#include <Conventions\VelocityConstraints.h>
#include "Conventions\OrientationFunctions.h"
#include "Math\MathFunctions.h"
#include "Math\UnitTests\ToString.h"
#include "Physics\Movement.h"
#include "Physics\Algorithms.h"
#include "Physics\InertiaFunctions.h"
#include "Physics\Constraints.h"
#include "Physics\PersistentConstraints.h"
#include "Physics\CollisionEvent.h"
#include "Physics\CollisionEventOffsets.h"
#include "Physics\WorldConfiguration.h"
#include <memory>
#include <limits>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Physics
{
	TEST_CLASS(ImplicitConstraitnSolverUnitTest)
	{
	public:

        TEST_METHOD(TestSomethingWithRotationSimple)
        {
            WorldConfiguration world_config;
            world_config.position_correction_fraction = 0.9f;
            world_config.penetration_depth_tolerance = 0.001f;
            world_config.position_correction_iterations = 100;
            world_config.velocity_correction_iterations = 150;
            world_config.angular_velocity_correction_fraction = 0.7f;
            world_config.fixed_fraction_velocity_loss_per_second = 0.1f;
            world_config.persitent_contact_expiry_age = 5;
            world_config.constraint_solver_type = ConstraintSolverType(0);
            world_config.solve_parallel = true;
            world_config.minimal_island_size = 32;
            world_config.warm_start_factor = 0.75f;
            world_config.solver_relaxation_factor = {1.f, 1.f};
            Physics::WorldRotationConstraints rotation_constraints;
            //rotation_constraints.body_ids = {{1, 0}};
            //rotation_constraints.rotation_normals = {{0,0,1}};
            //rotation_constraints.target_angles = {2.35456347f};
            //rotation_constraints.minmax_torques = {{-2000, 2000}};
            Physics::WorldVelocityConstraints velocity_constraints;
            //velocity_constraints.body_ids = {{1,0}, {1,0}};
            //velocity_constraints.directions = {{-0.721529841f, 0.692383587f, 0.000000000f}, {-0.692383587f, -0.721529841f, 0.000000000f}};
            //velocity_constraints.target_speeds = {0,0};
            //velocity_constraints.minmax_force = {{-297.262665f, 297.262665f}, {-371.578339f, 371.578339f}};
            Physics::WorldAngularVelocityConstraints angular_velocity_constraints; // empty
            CollisionEvents collision_events;
            //collision_events.bodies = {{{2, 0}, {0, 0}}};
            //collision_events.relative_positions = {{-15.6076097f, -10.6465940f, 0.447334439f}};
            //collision_events.manifolds.emplace_back();
            //auto & manifold = collision_events.manifolds.back();
            //manifold.penetration_depths = {{0.000999987125f, 9.99987387e-05f, 9.99987606e-06f, 9.99987833e-07f}};
            //manifold.separation_axes = {{{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}};
            //manifold.positions = {{{0.0f, 0.0f, -0.448334426f}, {0.0f, 0.0f, -0.448334426f}, {0.0f, 0.0f, -0.448334426f}, {0.0f, 0.0f, -0.448334426f}}};
            //manifold.ages = {{0, 1, 2, 3}};
            //manifold.contact_point_count = 4;
            CollisionEventOffsets event_offsets;
            event_offsets.kinematic_static = 0;
            event_offsets.kinematic_kinematic = 0;
            event_offsets.rigid_kinematic = 0;
            event_offsets.rigid_static = 0;
            //event_offsets.rigid_rigid = 1;
            event_offsets.rigid_rigid = 0;
            PerstistentConstraints persistent_constraints;
            //persistent_constraints.position_constraints.bodies = {{{1,0},{2,0}},{{1,0},{2,0}},{{1,0},{2,0}}};
            //persistent_constraints.position_constraints.directions = {{1.0f, 0.0f, 0.0},{0.0f, 1.0f, 0.0f},{0.0f, 0.0f, 1.0f}};
            //persistent_constraints.position_constraints.attachment_points.resize(3, {{{0, 0, 0}, {0, 0, 0}}});
            //persistent_constraints.position_constraints.distances = {7.10742185e-08f, 9.03112429e-09f, 0.861539185f};
            //persistent_constraints.position_constraints.minmax_forces = {3, {-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
            persistent_constraints.rotation_constraints.bodies = {{{1,0},{2,0}}};
            persistent_constraints.rotation_constraints.rotation_normals = {{0.0f, 0.0f, 1.0f}};
            persistent_constraints.rotation_constraints.target_angles = {0};
            persistent_constraints.rotation_constraints.minmax_torques = {{-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
            std::vector<uint32_t> rigid_body_to_element = {4294967295,0,1};
            //std::vector<Movement> old_movements = {{{-12.2258148f,-12.4220171f,-3.55243683e-05f},{0.0f, 0.0f, 7.82894230f}}, {{-12.2257786f, -12.4220428f, 0.0f}, {6.63351059f, -5.60163307f, -28.2315369f}}};
            //std::vector<Movement> rigid_body_movements = {{{-12.2011471f, -12.3969536f, -1.63173687f}, {0.0f, 0.0f, 7.81520653f}},{{-12.2008839f,-12.3967485f,-1.63167095f},{6.62187243f,-5.59180498f,-28.1820049f}}};
            std::vector<Movement> old_movements = {{{0.f,0.f,0.f},{0.0f, 0.0f, 0.f}}, {{0.f,0.f,0.f},{0.0f, 0.0f, 0.f}}};
            std::vector<Movement> rigid_body_movements = old_movements; //{{{0.f,0.f,0.f},{0.0f, 0.0f, 0.f}},{{0.f,0.f,0.f},{0.0f, 0.0f, 0.f}}};
            std::vector<Orientation> rigid_body_orientations = {{{0.f, 0.f, 0.f}, {0.f, 0.f, 1.f, 0.f}}, {{0.f, 0.f, 0.f}, {0.f, 0.f, 0.f, 1.f}}};
            //std::vector<Orientation> rigid_body_orientations = {{{-15.6076097f, -10.6465940f, 1.30887365f}, {0.000000000f, 0.000000000f, 0.927774191f, 0.373142302f}}, {{-15.6076097f, -10.6465940f, 0.447334439f}, Normalize(Math::Quaternion{0, 0,0.0337252915f, 0.0108235506f})}};
            std::vector<Inertia> rigid_body_inverse_inertias = {{{1, 0, 0, 0, 1, 0, 0, 0, 1}, 1.f}, {{1, 0, 0, 0, 1, 0, 0, 0, 1}, 1.f}};
            //rigid_body_inverse_inertias[1] = rigid_body_inverse_inertias[0];
            std::vector<Math::Float3> rigid_body_forces = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
            std::vector<uint32_t> body_to_element = {0, 1, 2};
            std::vector<float> bounce_factors = {0.0f, 0.0f, 0.000000000};
            std::vector<float> friction_factors = {1, 1, 1};
            auto constraint_solver = std::make_unique<ImplicitConstraintSolver>();
            constraint_solver->SetConfigurationFromWorldConfiguration(world_config);
            constraint_solver->SetCollisionEvents(&collision_events, &event_offsets);
            constraint_solver->SetConstraints(
                &persistent_constraints.distance_constraints,
                &persistent_constraints.position_constraints,
                &persistent_constraints.rotation_constraints,
                &velocity_constraints,
                &angular_velocity_constraints,
                &rotation_constraints);
            constraint_solver->SetRigidBodyData(rigid_body_to_element, old_movements, rigid_body_movements, rigid_body_orientations, rigid_body_inverse_inertias, rigid_body_forces);
            constraint_solver->SetCommonBodyData(
                body_to_element,
                bounce_factors,
                friction_factors
                );
            float time_step = 0.1f;
            constraint_solver->DoYourThing(time_step);

            auto normal = Rotate(persistent_constraints.rotation_constraints.rotation_normals[0], rigid_body_orientations[0].rotation);
            auto diff = rigid_body_orientations[1].rotation * Conjugate(rigid_body_orientations[0].rotation);
            auto angle = Math::AngleAroundNormal(diff, normal);
            Assert::AreEqual(0.f, angle, 1e-4f);
        }



        TEST_METHOD(TestSomethingWithRotationSimple2)
        {
            WorldConfiguration world_config;
            world_config.position_correction_fraction = 0.9f;
            world_config.penetration_depth_tolerance = 0.001f;
            world_config.position_correction_iterations = 100;
            world_config.velocity_correction_iterations = 150;
            world_config.angular_velocity_correction_fraction = 0.7f;
            world_config.fixed_fraction_velocity_loss_per_second = 0.1f;
            world_config.persitent_contact_expiry_age = 5;
            world_config.constraint_solver_type = ConstraintSolverType(0);
            world_config.solve_parallel = true;
            world_config.minimal_island_size = 32;
            world_config.warm_start_factor = 0.75f;
            world_config.solver_relaxation_factor = {1.f, 1.f};
            Physics::WorldRotationConstraints rotation_constraints;
            //rotation_constraints.body_ids = {{1, 0}};
            //rotation_constraints.rotation_normals = {{0,0,1}};
            //rotation_constraints.target_angles = {2.35456347f};
            //rotation_constraints.minmax_torques = {{-2000, 2000}};
            Physics::WorldVelocityConstraints velocity_constraints;
            //velocity_constraints.body_ids = {{1,0}, {1,0}};
            //velocity_constraints.directions = {{-0.721529841f, 0.692383587f, 0.000000000f}, {-0.692383587f, -0.721529841f, 0.000000000f}};
            //velocity_constraints.target_speeds = {0,0};
            //velocity_constraints.minmax_force = {{-297.262665f, 297.262665f}, {-371.578339f, 371.578339f}};
            Physics::WorldAngularVelocityConstraints angular_velocity_constraints; // empty
            CollisionEvents collision_events;
            //collision_events.bodies = {{{2, 0}, {0, 0}}};
            //collision_events.relative_positions = {{-15.6076097f, -10.6465940f, 0.447334439f}};
            //collision_events.manifolds.emplace_back();
            //auto & manifold = collision_events.manifolds.back();
            //manifold.penetration_depths = {{0.000999987125f, 9.99987387e-05f, 9.99987606e-06f, 9.99987833e-07f}};
            //manifold.separation_axes = {{{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}};
            //manifold.positions = {{{0.0f, 0.0f, -0.448334426f}, {0.0f, 0.0f, -0.448334426f}, {0.0f, 0.0f, -0.448334426f}, {0.0f, 0.0f, -0.448334426f}}};
            //manifold.ages = {{0, 1, 2, 3}};
            //manifold.contact_point_count = 4;
            CollisionEventOffsets event_offsets;
            event_offsets.kinematic_static = 0;
            event_offsets.kinematic_kinematic = 0;
            event_offsets.rigid_kinematic = 0;
            event_offsets.rigid_static = 0;
            //event_offsets.rigid_rigid = 1;
            event_offsets.rigid_rigid = 0;
            PerstistentConstraints persistent_constraints;
            //persistent_constraints.position_constraints.bodies = {{{1,0},{2,0}},{{1,0},{2,0}},{{1,0},{2,0}}};
            //persistent_constraints.position_constraints.directions = {{1.0f, 0.0f, 0.0},{0.0f, 1.0f, 0.0f},{0.0f, 0.0f, 1.0f}};
            //persistent_constraints.position_constraints.attachment_points.resize(3, {{{0, 0, 0}, {0, 0, 0}}});
            //persistent_constraints.position_constraints.distances = {7.10742185e-08f, 9.03112429e-09f, 0.861539185f};
            //persistent_constraints.position_constraints.minmax_forces = {3, {-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
            persistent_constraints.rotation_constraints.bodies = {{{1, 0}, {2, 0}}};
            persistent_constraints.rotation_constraints.rotation_normals = {{0.0f, 0.0f, 1.0f}};
            persistent_constraints.rotation_constraints.target_angles = {0};
            persistent_constraints.rotation_constraints.minmax_torques = {{-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
            std::vector<uint32_t> rigid_body_to_element = {4294967295, 0, 1};
            //std::vector<Movement> old_movements = {{{-12.2258148f,-12.4220171f,-3.55243683e-05f},{0.0f, 0.0f, 7.82894230f}}, {{-12.2257786f, -12.4220428f, 0.0f}, {6.63351059f, -5.60163307f, -28.2315369f}}};
            //std::vector<Movement> rigid_body_movements = {{{-12.2011471f, -12.3969536f, -1.63173687f}, {0.0f, 0.0f, 7.81520653f}},{{-12.2008839f,-12.3967485f,-1.63167095f},{6.62187243f,-5.59180498f,-28.1820049f}}};
            std::vector<Movement> old_movements = {{{0.f, 0.f, 0.f}, {0.0f, 0.0f, 0.f}}, {{0.f, 0.f, 0.f}, {0.0f, 0.0f, 0.f}}};
            std::vector<Movement> rigid_body_movements = old_movements; //{{{0.f,0.f,0.f},{0.0f, 0.0f, 0.f}},{{0.f,0.f,0.f},{0.0f, 0.0f, 0.f}}};
            std::vector<Orientation> rigid_body_orientations = {{{0.f, 0.f, 0.f}, Normalize(Math::Quaternion{0.f, 1.f, 0.f, 1.f})}, {{0.f, 0.f, 0.f}, Normalize(Math::Quaternion{0.f, 1.f, 1.f, 0.f})}};
            //std::vector<Orientation> rigid_body_orientations = {{{-15.6076097f, -10.6465940f, 1.30887365f}, {0.000000000f, 0.000000000f, 0.927774191f, 0.373142302f}}, {{-15.6076097f, -10.6465940f, 0.447334439f}, Normalize(Math::Quaternion{0, 0,0.0337252915f, 0.0108235506f})}};
            std::vector<Inertia> rigid_body_inverse_inertias = {{{1, 0, 0, 0, 1, 0, 0, 0, 1}, 1.f}, {{1, 0, 0, 0, 1, 0, 0, 0, 1}, 1.f}};
            //rigid_body_inverse_inertias[1] = rigid_body_inverse_inertias[0];
            std::vector<Math::Float3> rigid_body_forces = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
            std::vector<uint32_t> body_to_element = {0, 1, 2};
            std::vector<float> bounce_factors = {0.0f, 0.0f, 0.000000000};
            std::vector<float> friction_factors = {1, 1, 1};
            auto constraint_solver = std::make_unique<ImplicitConstraintSolver>();
            constraint_solver->SetConfigurationFromWorldConfiguration(world_config);
            constraint_solver->SetCollisionEvents(&collision_events, &event_offsets);
            constraint_solver->SetConstraints(
                &persistent_constraints.distance_constraints,
                &persistent_constraints.position_constraints,
                &persistent_constraints.rotation_constraints,
                &velocity_constraints,
                &angular_velocity_constraints,
                &rotation_constraints);
            constraint_solver->SetRigidBodyData(rigid_body_to_element, old_movements, rigid_body_movements, rigid_body_orientations, rigid_body_inverse_inertias, rigid_body_forces);
            constraint_solver->SetCommonBodyData(
                body_to_element,
                bounce_factors,
                friction_factors
            );
            float time_step = 0.1f;
            constraint_solver->DoYourThing(time_step);

            auto normal = Rotate(persistent_constraints.rotation_constraints.rotation_normals[0], rigid_body_orientations[0].rotation);
            auto diff = rigid_body_orientations[1].rotation * Conjugate(rigid_body_orientations[0].rotation);
            auto angle = Math::AngleAroundNormal(diff, normal);
            Assert::AreEqual(0.f, angle, 1e-4f);
        }
        TEST_METHOD(TestSomethingWithRotationSimple3)
        {
            WorldConfiguration world_config;
            world_config.position_correction_fraction = 0.9f;
            world_config.penetration_depth_tolerance = 0.001f;
            world_config.position_correction_iterations = 100;
            world_config.velocity_correction_iterations = 150;
            world_config.angular_velocity_correction_fraction = 0.7f;
            world_config.fixed_fraction_velocity_loss_per_second = 0.1f;
            world_config.persitent_contact_expiry_age = 5;
            world_config.constraint_solver_type = ConstraintSolverType(0);
            world_config.solve_parallel = true;
            world_config.minimal_island_size = 32;
            world_config.warm_start_factor = 0.75f;
            world_config.solver_relaxation_factor = {1.f, 1.f};
            Physics::WorldRotationConstraints rotation_constraints;
            //rotation_constraints.body_ids = {{1, 0}};
            //rotation_constraints.rotation_normals = {{0,0,1}};
            //rotation_constraints.target_angles = {2.35456347f};
            //rotation_constraints.minmax_torques = {{-2000, 2000}};
            Physics::WorldVelocityConstraints velocity_constraints;
            //velocity_constraints.body_ids = {{1,0}, {1,0}};
            //velocity_constraints.directions = {{-0.721529841f, 0.692383587f, 0.000000000f}, {-0.692383587f, -0.721529841f, 0.000000000f}};
            //velocity_constraints.target_speeds = {0,0};
            //velocity_constraints.minmax_force = {{-297.262665f, 297.262665f}, {-371.578339f, 371.578339f}};
            Physics::WorldAngularVelocityConstraints angular_velocity_constraints; // empty
            CollisionEvents collision_events;
            //collision_events.bodies = {{{2, 0}, {0, 0}}};
            //collision_events.relative_positions = {{-15.6076097f, -10.6465940f, 0.447334439f}};
            //collision_events.manifolds.emplace_back();
            //auto & manifold = collision_events.manifolds.back();
            //manifold.penetration_depths = {{0.000999987125f, 9.99987387e-05f, 9.99987606e-06f, 9.99987833e-07f}};
            //manifold.separation_axes = {{{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}};
            //manifold.positions = {{{0.0f, 0.0f, -0.448334426f}, {0.0f, 0.0f, -0.448334426f}, {0.0f, 0.0f, -0.448334426f}, {0.0f, 0.0f, -0.448334426f}}};
            //manifold.ages = {{0, 1, 2, 3}};
            //manifold.contact_point_count = 4;
            CollisionEventOffsets event_offsets;
            event_offsets.kinematic_static = 0;
            event_offsets.kinematic_kinematic = 0;
            event_offsets.rigid_kinematic = 0;
            event_offsets.rigid_static = 0;
            //event_offsets.rigid_rigid = 1;
            event_offsets.rigid_rigid = 0;
            PerstistentConstraints persistent_constraints;
            //persistent_constraints.position_constraints.bodies = {{{1,0},{2,0}},{{1,0},{2,0}},{{1,0},{2,0}}};
            //persistent_constraints.position_constraints.directions = {{1.0f, 0.0f, 0.0},{0.0f, 1.0f, 0.0f},{0.0f, 0.0f, 1.0f}};
            //persistent_constraints.position_constraints.attachment_points.resize(3, {{{0, 0, 0}, {0, 0, 0}}});
            //persistent_constraints.position_constraints.distances = {7.10742185e-08f, 9.03112429e-09f, 0.861539185f};
            //persistent_constraints.position_constraints.minmax_forces = {3, {-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
            persistent_constraints.rotation_constraints.bodies = {{{1, 0}, {2, 0}}};
            persistent_constraints.rotation_constraints.rotation_normals = {{0.0f, 0.0f, 1.0f}};
            persistent_constraints.rotation_constraints.target_angles = {0};
            persistent_constraints.rotation_constraints.minmax_torques = {{-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
            std::vector<uint32_t> rigid_body_to_element = {4294967295, 0, 1};
            //std::vector<Movement> old_movements = {{{-12.2258148f,-12.4220171f,-3.55243683e-05f},{0.0f, 0.0f, 7.82894230f}}, {{-12.2257786f, -12.4220428f, 0.0f}, {6.63351059f, -5.60163307f, -28.2315369f}}};
            //std::vector<Movement> rigid_body_movements = {{{-12.2011471f, -12.3969536f, -1.63173687f}, {0.0f, 0.0f, 7.81520653f}},{{-12.2008839f,-12.3967485f,-1.63167095f},{6.62187243f,-5.59180498f,-28.1820049f}}};
            std::vector<Movement> old_movements = {{{0.f, 0.f, 0.f}, {0.0f, 0.0f, 0.f}}, {{0.f, 0.f, 0.f}, {0.0f, 0.0f, 0.f}}};
            std::vector<Movement> rigid_body_movements = old_movements; //{{{0.f,0.f,0.f},{0.0f, 0.0f, 0.f}},{{0.f,0.f,0.f},{0.0f, 0.0f, 0.f}}};
            std::vector<Orientation> rigid_body_orientations = {{{0.f, 0.f, 0.f}, Normalize(Math::Quaternion{0.f, 0.f, 0.f, 1.f})}, {{0.f, 0.f, 0.f}, Normalize(Math::Quaternion{0.f, 1.f, 0.f, 0.f})}};
            //std::vector<Orientation> rigid_body_orientations = {{{-15.6076097f, -10.6465940f, 1.30887365f}, {0.000000000f, 0.000000000f, 0.927774191f, 0.373142302f}}, {{-15.6076097f, -10.6465940f, 0.447334439f}, Normalize(Math::Quaternion{0, 0,0.0337252915f, 0.0108235506f})}};
            std::vector<Inertia> rigid_body_inverse_inertias = {{{1, 0, 0, 0, 1, 0, 0, 0, 1}, 1.f}, {{1, 0, 0, 0, 1, 0, 0, 0, 1}, 1.f}};
            //rigid_body_inverse_inertias[1] = rigid_body_inverse_inertias[0];
            std::vector<Math::Float3> rigid_body_forces = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
            std::vector<uint32_t> body_to_element = {0, 1, 2};
            std::vector<float> bounce_factors = {0.0f, 0.0f, 0.000000000};
            std::vector<float> friction_factors = {1, 1, 1};
            auto constraint_solver = std::make_unique<ImplicitConstraintSolver>();
            constraint_solver->SetConfigurationFromWorldConfiguration(world_config);
            constraint_solver->SetCollisionEvents(&collision_events, &event_offsets);
            constraint_solver->SetConstraints(
                &persistent_constraints.distance_constraints,
                &persistent_constraints.position_constraints,
                &persistent_constraints.rotation_constraints,
                &velocity_constraints,
                &angular_velocity_constraints,
                &rotation_constraints);
            constraint_solver->SetRigidBodyData(rigid_body_to_element, old_movements, rigid_body_movements, rigid_body_orientations, rigid_body_inverse_inertias, rigid_body_forces);
            constraint_solver->SetCommonBodyData(
                body_to_element,
                bounce_factors,
                friction_factors
            );
            float time_step = 0.1f;
            constraint_solver->DoYourThing(time_step);

            auto normal = Rotate(persistent_constraints.rotation_constraints.rotation_normals[0], rigid_body_orientations[0].rotation);
            auto diff = rigid_body_orientations[1].rotation * Conjugate(rigid_body_orientations[0].rotation);
            auto angle = Math::AngleAroundNormal(diff, normal);
            Assert::AreEqual(0.f, angle, 1e-4f);
            //Assert::IsTrue( Equal( output, expected_output, 1e-5f ) );
        }

        TEST_METHOD(TestSomethingWithRotationSimple4)
        {
            WorldConfiguration world_config;
            world_config.position_correction_fraction = 0.9f;
            world_config.penetration_depth_tolerance = 0.001f;
            world_config.position_correction_iterations = 100;
            world_config.velocity_correction_iterations = 150;
            world_config.angular_velocity_correction_fraction = 0.7f;
            world_config.fixed_fraction_velocity_loss_per_second = 0.1f;
            world_config.persitent_contact_expiry_age = 5;
            world_config.constraint_solver_type = ConstraintSolverType(0);
            world_config.solve_parallel = true;
            world_config.minimal_island_size = 32;
            world_config.warm_start_factor = 0.75f;
            world_config.solver_relaxation_factor = {1.f, 1.f};
            Physics::WorldRotationConstraints rotation_constraints;
            //rotation_constraints.body_ids = {{1, 0}};
            //rotation_constraints.rotation_normals = {{0,0,1}};
            //rotation_constraints.target_angles = {2.35456347f};
            //rotation_constraints.minmax_torques = {{-2000, 2000}};
            Physics::WorldVelocityConstraints velocity_constraints;
            //velocity_constraints.body_ids = {{1,0}, {1,0}};
            //velocity_constraints.directions = {{-0.721529841f, 0.692383587f, 0.000000000f}, {-0.692383587f, -0.721529841f, 0.000000000f}};
            //velocity_constraints.target_speeds = {0,0};
            //velocity_constraints.minmax_force = {{-297.262665f, 297.262665f}, {-371.578339f, 371.578339f}};
            Physics::WorldAngularVelocityConstraints angular_velocity_constraints; // empty
            CollisionEvents collision_events;
            //collision_events.bodies = {{{2, 0}, {0, 0}}};
            //collision_events.relative_positions = {{-15.6076097f, -10.6465940f, 0.447334439f}};
            //collision_events.manifolds.emplace_back();
            //auto & manifold = collision_events.manifolds.back();
            //manifold.penetration_depths = {{0.000999987125f, 9.99987387e-05f, 9.99987606e-06f, 9.99987833e-07f}};
            //manifold.separation_axes = {{{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}};
            //manifold.positions = {{{0.0f, 0.0f, -0.448334426f}, {0.0f, 0.0f, -0.448334426f}, {0.0f, 0.0f, -0.448334426f}, {0.0f, 0.0f, -0.448334426f}}};
            //manifold.ages = {{0, 1, 2, 3}};
            //manifold.contact_point_count = 4;
            CollisionEventOffsets event_offsets;
            event_offsets.kinematic_static = 0;
            event_offsets.kinematic_kinematic = 0;
            event_offsets.rigid_kinematic = 0;
            event_offsets.rigid_static = 0;
            //event_offsets.rigid_rigid = 1;
            event_offsets.rigid_rigid = 0;
            PerstistentConstraints persistent_constraints;
            //persistent_constraints.position_constraints.bodies = {{{1,0},{2,0}},{{1,0},{2,0}},{{1,0},{2,0}}};
            //persistent_constraints.position_constraints.directions = {{1.0f, 0.0f, 0.0},{0.0f, 1.0f, 0.0f},{0.0f, 0.0f, 1.0f}};
            //persistent_constraints.position_constraints.attachment_points.resize(3, {{{0, 0, 0}, {0, 0, 0}}});
            //persistent_constraints.position_constraints.distances = {7.10742185e-08f, 9.03112429e-09f, 0.861539185f};
            //persistent_constraints.position_constraints.minmax_forces = {3, {-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
            persistent_constraints.rotation_constraints.bodies = {{{1, 0}, {2, 0}}};
            persistent_constraints.rotation_constraints.rotation_normals = {{0.0f, 0.0f, 1.0f}};
            persistent_constraints.rotation_constraints.target_angles = {0};
            persistent_constraints.rotation_constraints.minmax_torques = {{-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
            std::vector<uint32_t> rigid_body_to_element = {4294967295, 0, 1};
            //std::vector<Movement> old_movements = {{{-12.2258148f,-12.4220171f,-3.55243683e-05f},{0.0f, 0.0f, 7.82894230f}}, {{-12.2257786f, -12.4220428f, 0.0f}, {6.63351059f, -5.60163307f, -28.2315369f}}};
            //std::vector<Movement> rigid_body_movements = {{{-12.2011471f, -12.3969536f, -1.63173687f}, {0.0f, 0.0f, 7.81520653f}},{{-12.2008839f,-12.3967485f,-1.63167095f},{6.62187243f,-5.59180498f,-28.1820049f}}};
            std::vector<Movement> old_movements = {{{0.f, 0.f, 0.f}, {0.0f, 0.0f, 1.f}}, {{0.f, 0.f, 0.f}, {0.0f, 0.0f, 1.f}}};
            std::vector<Movement> rigid_body_movements = old_movements; //{{{0.f,0.f,0.f},{0.0f, 0.0f, 0.f}},{{0.f,0.f,0.f},{0.0f, 0.0f, 0.f}}};
            std::vector<Orientation> rigid_body_orientations = {{{0.f, 0.f, 0.f}, Normalize(Math::Quaternion{0.f, 1.f, 0.f, 1.f})}, {{0.f, 0.f, 0.f}, Normalize(Math::Quaternion{0.f, 1.f, 1.f, 0.f})}};
            //std::vector<Orientation> rigid_body_orientations = {{{-15.6076097f, -10.6465940f, 1.30887365f}, {0.000000000f, 0.000000000f, 0.927774191f, 0.373142302f}}, {{-15.6076097f, -10.6465940f, 0.447334439f}, Normalize(Math::Quaternion{0, 0,0.0337252915f, 0.0108235506f})}};
            std::vector<Inertia> rigid_body_inverse_inertias = {{{1, 0, 0, 0, 1, 0, 0, 0, 1}, 1.f}, {{1, 0, 0, 0, 1, 0, 0, 0, 1}, 1.f}};
            //rigid_body_inverse_inertias[1] = rigid_body_inverse_inertias[0];
            std::vector<Math::Float3> rigid_body_forces = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
            std::vector<uint32_t> body_to_element = {0, 1, 2};
            std::vector<float> bounce_factors = {0.0f, 0.0f, 0.000000000};
            std::vector<float> friction_factors = {1, 1, 1};
            auto constraint_solver = std::make_unique<ImplicitConstraintSolver>();
            constraint_solver->SetConfigurationFromWorldConfiguration(world_config);
            constraint_solver->SetCollisionEvents(&collision_events, &event_offsets);
            constraint_solver->SetConstraints(
                &persistent_constraints.distance_constraints,
                &persistent_constraints.position_constraints,
                &persistent_constraints.rotation_constraints,
                &velocity_constraints,
                &angular_velocity_constraints,
                &rotation_constraints);
            constraint_solver->SetRigidBodyData(rigid_body_to_element, old_movements, rigid_body_movements, rigid_body_orientations, rigid_body_inverse_inertias, rigid_body_forces);
            constraint_solver->SetCommonBodyData(
                body_to_element,
                bounce_factors,
                friction_factors
            );
            float time_step = 0.01f;
            constraint_solver->DoYourThing(time_step);

            auto normal = Rotate(persistent_constraints.rotation_constraints.rotation_normals[0], rigid_body_orientations[0].rotation);
            auto diff = rigid_body_orientations[1].rotation * Conjugate(rigid_body_orientations[0].rotation);
            auto angle = Math::AngleAroundNormal(diff, normal);
            Assert::AreEqual(0.f, angle, 1e-2f);
        }

        TEST_METHOD(TestSomethingWithRotationSimple5)
        {
            WorldConfiguration world_config;
            world_config.position_correction_fraction = 0.899999976f;
            world_config.penetration_depth_tolerance = 0.00100000005f;
            world_config.position_correction_iterations = 1000;
            world_config.velocity_correction_iterations = 1500;
            world_config.angular_velocity_correction_fraction = 0.699999988f;
            world_config.fixed_fraction_velocity_loss_per_second = 0.100000001f;
            world_config.persitent_contact_expiry_age = 5;
            world_config.constraint_solver_type = ConstraintSolverType(0);
            world_config.solve_parallel = true;
            world_config.minimal_island_size = 32;
            world_config.warm_start_factor = 0.750000000f;
            world_config.solver_relaxation_factor = {1.00000000f, 1.50000000f};
            Physics::WorldRotationConstraints rotation_constraints;
            //rotation_constraints.body_ids = {{1, 0}};
            //rotation_constraints.rotation_normals = {{0,0,1}};
            //rotation_constraints.target_angles = {2.35456347f};
            //rotation_constraints.minmax_torques = {{-2000, 2000}};
            Physics::WorldVelocityConstraints velocity_constraints;
            //velocity_constraints.body_ids = {{1,0}, {1,0}};
            //velocity_constraints.directions = {{-0.721529841f, 0.692383587f, 0.000000000f}, {-0.692383587f, -0.721529841f, 0.000000000f}};
            //velocity_constraints.target_speeds = {0,0};
            //velocity_constraints.minmax_force = {{-297.262665f, 297.262665f}, {-371.578339f, 371.578339f}};
            Physics::WorldAngularVelocityConstraints angular_velocity_constraints; // empty
            CollisionEvents collision_events;
            //collision_events.bodies = {{{2, 0}, {0, 0}}};
            //collision_events.relative_positions = {{-15.6076097f, -10.6465940f, 0.447334439f}};
            //collision_events.manifolds.emplace_back();
            //auto & manifold = collision_events.manifolds.back();
            //manifold.penetration_depths = {{0.000999987125f, 9.99987387e-05f, 9.99987606e-06f, 9.99987833e-07f}};
            //manifold.separation_axes = {{{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}};
            //manifold.positions = {{{0.0f, 0.0f, -0.448334426f}, {0.0f, 0.0f, -0.448334426f}, {0.0f, 0.0f, -0.448334426f}, {0.0f, 0.0f, -0.448334426f}}};
            //manifold.ages = {{0, 1, 2, 3}};
            //manifold.contact_point_count = 4;
            CollisionEventOffsets event_offsets;
            event_offsets.kinematic_static = 0;
            event_offsets.kinematic_kinematic = 0;
            event_offsets.rigid_kinematic = 0;
            event_offsets.rigid_static = 0;
            //event_offsets.rigid_rigid = 1;
            event_offsets.rigid_rigid = 0;
            PerstistentConstraints persistent_constraints;
            //persistent_constraints.position_constraints.bodies = {{{1,0},{2,0}},{{1,0},{2,0}},{{1,0},{2,0}}};
            //persistent_constraints.position_constraints.directions = {{1.0f, 0.0f, 0.0},{0.0f, 1.0f, 0.0f},{0.0f, 0.0f, 1.0f}};
            //persistent_constraints.position_constraints.attachment_points.resize(3, {{{0, 0, 0}, {0, 0, 0}}});
            //persistent_constraints.position_constraints.distances = {7.10742185e-08f, 9.03112429e-09f, 0.861539185f};
            //persistent_constraints.position_constraints.minmax_forces = {3, {-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
            persistent_constraints.rotation_constraints.bodies = {{{1, 0}, {2, 0}}};
            persistent_constraints.rotation_constraints.rotation_normals = {{0.0f, 0.0f, 1.0f}};
            persistent_constraints.rotation_constraints.target_angles = {0};
            persistent_constraints.rotation_constraints.minmax_torques = {{-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
            std::vector<uint32_t> rigid_body_to_element = {4294967295, 0, 1};
            std::vector<Movement> old_movements = {{{0.f, 0.f, 0.f}, {0.0f, 0.0f, 40.9167938f}}, {{0.f, 0.f, 0.f}, {10.2373457f, 8.71367645f, -67.3452606f}}};
            std::vector<Movement> rigid_body_movements = old_movements; //{{{0.f,0.f,0.f},{0.0f, 0.0f, 0.f}},{{0.f,0.f,0.f},{0.0f, 0.0f, 0.f}}};
            std::vector<Orientation> rigid_body_orientations = {{{0.f, 0.f, 0.f}, {0.000000000f, 0.000000000f, 0.999522865f, 0.0308789909f}}, {{0.f, 0.f, 0.f}, {0.762248755f, -0.646745801f, -0.000866752118f, -0.0263904277f}}};
            //std::vector<Orientation> rigid_body_orientations = {{{-15.6076097f, -10.6465940f, 1.30887365f}, {0.000000000f, 0.000000000f, 0.927774191f, 0.373142302f}}, {{-15.6076097f, -10.6465940f, 0.447334439f}, Normalize(Math::Quaternion{0, 0,0.0337252915f, 0.0108235506f})}};
            std::vector<Inertia> rigid_body_inverse_inertias = {{{0, 0, 0, 0, 0, 0, 0, 0, 0.930167973f}, 1.f}, {{0.758285895f, 4.04880848e-05f, -0.00944667030f, 4.04919847e-05f, 0.758058205f, -0.00715696719f, -0.00944667496f, -0.00715696719f, 0.924740657f}, 1.f}};
            //rigid_body_inverse_inertias[1] = rigid_body_inverse_inertias[0];
            std::vector<Math::Float3> rigid_body_forces = {{0.0f, 0.0f, -98.1000061f}, {0.0f, 0.0f, -98.1000061f}};
            std::vector<uint32_t> body_to_element = {0, 1, 2};
            std::vector<float> bounce_factors = {0.500000000f, 0.00999999978f, 0.000000000};
            std::vector<float> friction_factors = {1, 1, 1};
            auto constraint_solver = std::make_unique<ImplicitConstraintSolver>();
            constraint_solver->SetConfigurationFromWorldConfiguration(world_config);
            constraint_solver->SetCollisionEvents(&collision_events, &event_offsets);
            constraint_solver->SetConstraints(
                &persistent_constraints.distance_constraints,
                &persistent_constraints.position_constraints,
                &persistent_constraints.rotation_constraints,
                &velocity_constraints,
                &angular_velocity_constraints,
                &rotation_constraints);
            constraint_solver->SetRigidBodyData(rigid_body_to_element, old_movements, rigid_body_movements, rigid_body_orientations, rigid_body_inverse_inertias, rigid_body_forces);
            constraint_solver->SetCommonBodyData(
                body_to_element,
                bounce_factors,
                friction_factors
            );
            float time_step = 0.0166666675f;
            constraint_solver->DoYourThing(time_step);

            auto normal = Rotate(persistent_constraints.rotation_constraints.rotation_normals[0], rigid_body_orientations[0].rotation);
            auto diff = rigid_body_orientations[1].rotation * Conjugate(rigid_body_orientations[0].rotation);
            auto angle = Math::AngleAroundNormal(diff, normal);
            Assert::AreEqual(0.f, angle, 0.1f);
        }
        

		TEST_METHOD(TestSomethingWithRotation)
		{
            WorldConfiguration world_config;
            world_config.position_correction_fraction = 0.899999976f;
            world_config.penetration_depth_tolerance = 0.00100000005f;
            world_config.position_correction_iterations = 1000;
            world_config.velocity_correction_iterations = 1500;
            world_config.angular_velocity_correction_fraction = 0.699999988f;
            world_config.fixed_fraction_velocity_loss_per_second = 0.100000001f;
            world_config.persitent_contact_expiry_age = 5;
            world_config.constraint_solver_type = ConstraintSolverType(0);
            world_config.solve_parallel = true;
            world_config.minimal_island_size = 32;
            world_config.warm_start_factor = 0.750000000f;
            world_config.solver_relaxation_factor = {1.00000000f, 1.50000000f};
            Physics::WorldRotationConstraints rotation_constraints;
            //rotation_constraints.body_ids = {{1, 0}};
            //rotation_constraints.rotation_normals = {{0,0,1}};
            //rotation_constraints.target_angles = {2.35456347f};
            //rotation_constraints.minmax_torques = {{-2000, 2000}};
            Physics::WorldVelocityConstraints velocity_constraints;
            //velocity_constraints.body_ids = {{1,0}, {1,0}};
            //velocity_constraints.directions = {{-0.721529841f, 0.692383587f, 0.000000000f}, {-0.692383587f, -0.721529841f, 0.000000000f}};
            //velocity_constraints.target_speeds = {0,0};
            //velocity_constraints.minmax_force = {{-297.262665f, 297.262665f}, {-371.578339f, 371.578339f}};
            Physics::WorldAngularVelocityConstraints angular_velocity_constraints; // empty
            CollisionEvents collision_events;
            //collision_events.bodies = {{{2, 0}, {0, 0}}};
            //collision_events.relative_positions = {{-15.6076097f, -10.6465940f, 0.447334439f}};
            //collision_events.manifolds.emplace_back();
            //auto & manifold = collision_events.manifolds.back();
            //manifold.penetration_depths = {{0.000999987125f, 9.99987387e-05f, 9.99987606e-06f, 9.99987833e-07f}};
            //manifold.separation_axes = {{{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}};
            //manifold.positions = {{{0.0f, 0.0f, -0.448334426f}, {0.0f, 0.0f, -0.448334426f}, {0.0f, 0.0f, -0.448334426f}, {0.0f, 0.0f, -0.448334426f}}};
            //manifold.ages = {{0, 1, 2, 3}};
            //manifold.contact_point_count = 4;
            CollisionEventOffsets event_offsets;
            event_offsets.kinematic_static = 0;
            event_offsets.kinematic_kinematic = 0;
            event_offsets.rigid_kinematic = 0;
            event_offsets.rigid_static = 0;
            //event_offsets.rigid_rigid = 1;
            event_offsets.rigid_rigid = 0;
            PerstistentConstraints persistent_constraints;
            //persistent_constraints.position_constraints.bodies = {{{1,0},{2,0}},{{1,0},{2,0}},{{1,0},{2,0}}};
            //persistent_constraints.position_constraints.directions = {{1.0f, 0.0f, 0.0},{0.0f, 1.0f, 0.0f},{0.0f, 0.0f, 1.0f}};
            //persistent_constraints.position_constraints.attachment_points.resize(3, {{{0, 0, 0}, {0, 0, 0}}});
            //persistent_constraints.position_constraints.distances = {7.10742185e-08f, 9.03112429e-09f, 0.861539185f};
            //persistent_constraints.position_constraints.minmax_forces = {3, {-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
            persistent_constraints.rotation_constraints.bodies = {{{1,0},{2,0}}};
            persistent_constraints.rotation_constraints.rotation_normals = {{0.0f, 0.0f, 1.0f}};
            persistent_constraints.rotation_constraints.target_angles = {0};
            persistent_constraints.rotation_constraints.minmax_torques = {{-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()}};
            std::vector<uint32_t> rigid_body_to_element = {4294967295,0,1};
            std::vector<Movement> old_movements = {{{-12.2258148f,-12.4220171f,-3.55243683e-05f},{0.0f, 0.0f, 7.82894230f}}, {{-12.2257786f, -12.4220428f, 0.0f}, {6.63351059f, -5.60163307f, -28.2315369f}}};
            std::vector<Movement> rigid_body_movements = {{{-12.2011471f, -12.3969536f, -1.63173687f}, {0.0f, 0.0f, 7.81520653f}},{{-12.2008839f,-12.3967485f,-1.63167095f},{6.62187243f,-5.59180498f,-28.1820049f}}};
            //std::vector<Movement> old_movements = {{{-12.2258148f,-12.4220171f,-3.55243683e-05f},{0.0f, 0.0f, 7.82894230f}}, {{-12.2257786f, -12.4220428f, 0.0f}, {0.f, -0.f, -28.2315369f}}};
            //std::vector<Movement> rigid_body_movements = {{{-12.2011471f, -12.3969536f, -1.63173687f}, {0.0f, 0.0f, 7.81520653f}},{{-12.2008839f,-12.3967485f,-1.63167095f},{0.f, -0.f, -28.1820049f}}};
            std::vector<Orientation> rigid_body_orientations = {{{-15.6076097f, -10.6465940f, 1.30887365f}, {0.000000000f, 0.000000000f, 0.927774191f, 0.373142302f}}, {{-15.6076097f, -10.6465940f, 0.447334439f}, {0.574052632f, 0.818051994f, 0.0337252915f, 0.0108235506f}}};
            //std::vector<Orientation> rigid_body_orientations = {{{-15.6076097f, -10.6465940f, 1.30887365f}, {0.000000000f, 0.000000000f, 0.927774191f, 0.373142302f}}, {{-15.6076097f, -10.6465940f, 0.447334439f}, Normalize(Math::Quaternion{0, 0,0.0337252915f, 0.0108235506f})}};
            std::vector<Inertia> rigid_body_inverse_inertias = {{{0, 0, 0, 0, 0, 0, 0, 0, 0.0930167958f}, 0.100000001f}, {{0.0758285895f, 4.04880848e-05f, -0.000944667030f, 4.04919847e-05f, 0.0758058205f, -0.000715696719f, -0.000944667496f, -0.000715696719f, 0.0924740657f}, 0.100000001f}};
            //rigid_body_inverse_inertias[1] = rigid_body_inverse_inertias[0];
            std::vector<Math::Float3> rigid_body_forces = {{0.0f, 0.0f, -98.1000061f}, {0.0f, 0.0f, -98.1000061f}};
            std::vector<uint32_t> body_to_element = {0, 1, 2};
            std::vector<float> bounce_factors = {0.500000000f, 0.00999999978f, 0.000000000};
            std::vector<float> friction_factors = {1, 1, 1};
            auto constraint_solver = std::make_unique<ImplicitConstraintSolver>();
            constraint_solver->SetConfigurationFromWorldConfiguration(world_config);
            constraint_solver->SetCollisionEvents(&collision_events, &event_offsets);
            constraint_solver->SetConstraints(
                &persistent_constraints.distance_constraints,
                &persistent_constraints.position_constraints,
                &persistent_constraints.rotation_constraints,
                &velocity_constraints,
                &angular_velocity_constraints,
                &rotation_constraints);
            constraint_solver->SetRigidBodyData(rigid_body_to_element, old_movements, rigid_body_movements, rigid_body_orientations, rigid_body_inverse_inertias, rigid_body_forces);
            constraint_solver->SetCommonBodyData(
                body_to_element,
                bounce_factors,
                friction_factors
                );
            float time_step = 0.0166666675f;
            constraint_solver->DoYourThing(time_step);

            auto normal = Rotate(persistent_constraints.rotation_constraints.rotation_normals[0], rigid_body_orientations[0].rotation);
            auto diff = rigid_body_orientations[1].rotation * Conjugate(rigid_body_orientations[0].rotation);
            auto angle = Math::AngleAroundNormal(diff, normal);
            Assert::AreEqual(0.f, angle, 0.1f);
		}
	};
}
