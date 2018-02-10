#pragma once
#include "DLL.h"
// local includes
#include "CollisionEventOffsets.h"
#include "DensityFunctionContainer.h"
#include "ElementContainer.h"
#include "MeshContainer.h"
#include "MovingEntities.h"
#include "NonCollidingBodies.h"
#include "OrientedBoxContainer.h"
#include "ResourceManager.h"
#include "SphereContainer.h"
#include "Movement.h"
#include "WorldConfiguration.h"
#include "BodyIDGenerator.h"
#include "BodyEntityMapping.h"
#include "CollisionEvent.h"
#include "ResourceDescriptions.h"
#include "PersistentConstraints.h"

// DogDealer includes
#include <Conventions\CollisionEvent.h>
#include <Conventions\EntityID.h>
#include <Conventions\Velocity.h>
#include <Math\SparseAdjacencyMatrix.h>
#include <BoundingShapes\Ray.h>

#include <memory>

struct RotationConstraints;
struct VelocityConstraints;
struct AngularVelocityConstraints;
struct EntityForces;
struct EntityTorques;

namespace Physics
{
    class ConstraintSolver;

    class PHYSICS_DLL PhysicsWorld
    {
		ResourceManager	m_resource_manager;

        DensityFunctionContainer m_density_function_container;
        ElementContainer m_element_container;
        MeshContainer m_mesh_container;
        MovingEntities m_moving_entities;
        NonCollidingBodies m_non_colliding_bodies;
        OrientedBoxContainer m_oriented_box_container;
        SphereContainer m_sphere_container;
        BodyEntityMapping m_body_entity_mapping;
        PerstistentConstraints m_persistent_constraints;
        // records which bodies can't collide, and thus have to be ignored by the collision detection
        Math::SparseAdjacencyMatrix m_non_colliding_bodies_pairs;

        CollisionEvents m_current_collision_events, m_previous_collision_events;
        CollisionEventOffsets m_previous_collision_event_offsets;
        std::unique_ptr<ConstraintSolver> m_constraint_solver;
        WorldConfiguration m_world_configuration;
        BodyIDGenerator m_body_id_generator;
    public:

        Math::Float3 m_gravity;

        PhysicsWorld();
        ~PhysicsWorld();

        void SetWorldConfiguration(
            WorldConfiguration const & world_config
            );

        void CreateKinematicBodyComponent(
            EntityID entity_id,
            std::string const & collision_file,
            Orientation orientation,
            float bouncincess,
            float friction_factor
            );

        void CreateKinematicNonCollidingBodyComponent(
            EntityID entity_id,
            Orientation orientation
            );

        void CreateRigidBodyComponent(
            EntityID entity_id,
            std::string const & collision_file,
            float mass,
            Orientation orientation,
            Movement movement,
            float bounciness,
            float friction_factor,
            bool lock_rotation
            );


        void CreateMultiRigidBodyComponent(
            EntityID entity_id,
            Orientation orientation,
            Movement movement,
            Range<ComponentDescription::Body const *> bodies,
            Range<Connection const*> connections
            );


        void CreateStaticBodyComponent(
            EntityID entity_id,
            std::string const & collision_file,
            Orientation orientation,
            float bounciness,
            float friction_factor
            );

        void CreateStaticDensityBodyComponent(
            EntityID entity_id,
            BoundingShapes::AxisAlignedBox broad_bounds,
            DensityFunctionType sample_function,
            Orientation orientation,
            float bounciness,
            float friction_factor
            );

        void CreateStaticNonCollidingBodyComponent(
            EntityID entity_id,
            Orientation orientation
            );

        void ReplaceWithKinematicBodyComponent(
            EntityID entity_id,
            std::string const & collision_file,
            float bouncincess,
            float friction_factor
            );

        void ReplaceWithRigidBodyComponent(
            EntityID entity_id,
            std::string const & collision_file,
            float mass,
            float bounciness,
            float friction_factor,
            bool lock_rotation
            );

        void ReplaceWithMultiRigidBodyComponent(
            EntityID entity_id,
            Range<ComponentDescription::Body const *> bodies,
            Range<Connection const*> connections
            );

        void ReplaceWithStaticDensityBodyComponent(
            EntityID entity_id,
            BoundingShapes::AxisAlignedBox broad_bounds,
            DensityFunctionType sample_function,
            float bounciness,
            float friction_factor
            );

        void ReplaceWithStaticBodyComponent(
            EntityID entity_id,
            std::string const & collision_file,
            float bounciness,
            float friction_factor
            );

        void RemoveEntities(
            Range<EntityID const *> const entity_ids
            );

        // will assume all bodies of each entity pair cannot collide with each other
        void AddNonCollidingEntityPairs(
            Range<EntityPair const *> entity_pairs
            );

        void RemoveNonCollidingEntityPairs(
            Range<EntityPair const *> entity_pairs
            );

        void RemoveBodiesFromNonCollidingBodyPairs(Range<BodyID const*> bodies);

        bool HasRigidBodyComponent(
            EntityID entity_id
            ) const;

        Math::Float3 FindRestingPositionOnEntity(
            Math::Float3 starting_position,
            EntityID target_entity
            ) const;

        Math::Float3 PhysicsWorld::CastRayOnStaticEntities(
            BoundingShapes::Ray const & ray
            ) const;

        Math::Float3 PhysicsWorld::CastRayOnBody(
            BoundingShapes::Ray const & ray,
            BodyID const & body
            ) const;

        Math::Float3 PhysicsWorld::CastRayOnBodies(
            BoundingShapes::Ray const & ray,
            Range<BodyID const *> bodies
            ) const;

        Math::Float3 PhysicsWorld::CastRayOnEntity(
            BoundingShapes::Ray const & ray,
            EntityID target_entity
            ) const;

        IndexedOrientations GetOrientations() const;
		MovingEntities const & GetMovingEntities() const;

        void CopyCurrentToPrevious();

        void UpdateOrientations(
            EntityPositions const & entity_positions,
            EntityRotations const & entity_rotations
            );

        ::CollisionEvents UpdateBodies(
            EntityForces const & entity_forces,
            EntityTorques const & entity_torque,
            ::RotationConstraints const & rotation_constraints,
            ::VelocityConstraints const & velocity_constraints,
            ::AngularVelocityConstraints const & angular_velocity_constraints,
            float const time_step
            );

		void CalculateVelocities(
            float time_step
            );

        void AdjustAllPositions(
            Math::Float3 adjustment
            );

        // appends all oriented boxes at their entity local position and the entities they belong to
        void GetAllOrientedBoxes(
            std::vector<BoundingShapes::OrientedBox>& boxes,
            std::vector<EntityID>& entities
            );

        // appends all spheres at their entity local position and the entities they belong to
        void GetAllSpheres(
            std::vector<BoundingShapes::Sphere>& boxes,
            std::vector<EntityID>& entities
            );

    private:

        // currently the previous_collision_events are only used to re-use the storage
        Physics::CollisionEvents FindCollisions(
            Physics::CollisionEvents previous_collision_events = {}
            ) const;

        void CreatePersistentConstraints(EntityID entity_id, Range<Connection const *> connections);

        void RemoveShapes(
            Range<EntityID const *> const entity_ids
            );

        void RemoveShapes(
            Range<BodyID const *> const body_ids
            );
    };


    inline MovingEntities const & PhysicsWorld::GetMovingEntities() const
    {
        return m_moving_entities;
    }
}
