#include "PhysicsWorld.h"

#include "BodyAndOrientationPair.h"
#include "Algorithms.h"
#include "BroadPhase.h"
#include "CollisionResolving.h"
#include "ConstraintSolverType.h"
#include "ImplicitConstraintSolver\ImplicitConstraintSolver.h"
#include "FrictionAlgorithms.h"
#include "InertiaFunctions.h"
#include "ManifoldFunctions.h"
#include "NarrowPhase.h"
#include "RayCasting.h"
#include "Inertia.h"
#include "BodyEntityMappingFunctions.h"
#include "Constraints.h"

#include <BoundingShapes\AxisAlignedBoxFunctions.h>
#include <BoundingShapes\AxisAlignedBoxHierarchyFunctions.h>
#include <BoundingShapes\OrientedBoxFunctions.h>
#include <BoundingShapes\SphereFunctions.h>
#include <BoundingShapes\RayFunctions.h>

#include <Conventions\OrientationFunctions.h>
#include <Conventions\RotationConstraints.h>
#include <Conventions\VelocityConstraints.h>

#include <Math\Edge.h>
#include <Math\FloatMatrixOperators.h>
#include <Math\FloatMatrixTypes.h>
#include <Math\MathFunctions.h>
#include <Math\SparseAdjacencyMatrixFunctions.h>
#include <Math\TransformFunctions.h>
#include <Math\VectorAlgorithms.h>

#include <Utilities\HRTimer.h>
#include <Utilities\IndexedHelp.h>
#include <Utilities\IntegerIterator.h>
#include <Utilities\Logger.h>
#include <Utilities\Memory.h>
#include <Utilities\DogDealerException.h>
#include <Utilities\VectorHelper.h>

#include <cassert>
#include <cmath>

#include <Math\MathToString.h>

using namespace Physics;

PhysicsWorld::PhysicsWorld()
{
    m_world_configuration.position_correction_iterations = 10;
    m_world_configuration.velocity_correction_iterations = 15;
    m_world_configuration.position_correction_fraction = 0.7f;
    m_world_configuration.penetration_depth_tolerance = 1e-2f;
    m_world_configuration.angular_velocity_correction_fraction = 0.5f;
    m_world_configuration.fixed_fraction_velocity_loss_per_second = 0.1f;
    m_world_configuration.persitent_contact_expiry_age = 5;
    m_world_configuration.constraint_solver_type = ConstraintSolverType::Implicit;
    m_world_configuration.solver_relaxation_factor = {1, 1};
    m_world_configuration.minimal_island_size = 128;
    m_gravity = { 0, 0, -9.81f };

    m_constraint_solver = std::make_unique<ImplicitConstraintSolver>();
    m_constraint_solver->SetConfigurationFromWorldConfiguration(m_world_configuration);
}


PhysicsWorld::~PhysicsWorld()
{
}


void PhysicsWorld::SetWorldConfiguration(WorldConfiguration const & world_config)
{
    if(m_world_configuration.constraint_solver_type != world_config.constraint_solver_type)
    {
        switch(world_config.constraint_solver_type)
        {
            case ConstraintSolverType::Implicit:
            m_constraint_solver = std::make_unique<ImplicitConstraintSolver>();
            break;

            default:
            throw DogDealerException("Invalid constraint solver type.", false);
        }
    }
    m_world_configuration = world_config;
    m_constraint_solver->SetConfigurationFromWorldConfiguration(m_world_configuration);
}


namespace
{
    bool IsValidMass( float mass )
    {
        return !isinf( mass ) && mass > 0;
    }


    void ProvideCollisionData(
        std::string const & collision_file,
        ResourceManager & resource_manager,
        MeshContainer & mesh_container,
        StoredCollisionData & collision_data
        )
    {
        if(!resource_manager.GetLoadedCollisionData(collision_file, collision_data))
        {
            NewCollisionData new_collision_data;
            resource_manager.LoadCollisionData(collision_file, new_collision_data);
            for(auto & mesh : new_collision_data.meshes)
            {
                auto id = AddMesh(std::move(mesh), mesh_container);
                collision_data.mesh_ids.push_back(id);
            }
            collision_data.axis_aligned_box = new_collision_data.axis_aligned_box;
            collision_data.oriented_boxes = move(new_collision_data.oriented_boxes);
            collision_data.spheres = move(new_collision_data.spheres);
            resource_manager.StoreCollisionData(collision_file, collision_data);
        }
    }


    BodyID CreateNewBodyID(
        EntityID entity_id,
        BodyIDGenerator & generator,
        BodyEntityMapping & mapping
        )
    {
        auto body_id = generator.NewID();
        Add(entity_id, body_id, mapping);
        return body_id;
    }


    void FreeBodyIDsForEntity(
        EntityID entity_id,
        BodyIDGenerator & generator,
        BodyEntityMapping & mapping
        )
    {
        auto bodies = Bodies(entity_id, mapping);
        generator.Remove(bodies);
    }
}


void PhysicsWorld::CreateRigidBodyComponent(
    EntityID entity_id,
    std::string const & collision_file,
    float mass,
    Orientation orientation,
    Movement movement,
    float bounciness,
    float friction_factor,
    bool lock_rotation
    )
{
    assert( IsValidMass( mass ) );
    StoredCollisionData collision_data;
    ProvideCollisionData(collision_file, m_resource_manager, m_mesh_container, collision_data);
    // assert(collision_data.oriented_boxes.empty() != collision_data.spheres.empty()); // one XOR the other should not be empty
    assert(!collision_data.oriented_boxes.empty() || !collision_data.spheres.empty());
    assert(collision_data.mesh_ids.empty());

    auto body_id = CreateNewBodyID(entity_id, m_body_id_generator, m_body_entity_mapping);
    Inertia total_inertia;
    Math::Float3 center_of_mass;
    CalculateTotalInertia(collision_data.spheres, collision_data.oriented_boxes, mass, total_inertia, center_of_mass);
    for( auto & box : collision_data.oriented_boxes )
    {
        box.center -= center_of_mass;
    }
    for( auto & sphere : collision_data.spheres )
    {
        sphere.center -= center_of_mass;
    }
    if(!IsEmpty(collision_data.oriented_boxes))
    {
        AddBoxes( body_id, collision_data.oriented_boxes, m_oriented_box_container );
    }
    if(!collision_data.spheres.empty())
    {
        AddSpheres( body_id, collision_data.spheres, m_sphere_container );
    }
    // for stabilization we add the mass to the diagonal
    total_inertia.moment( 0, 0 ) += mass;
    total_inertia.moment( 1, 1 ) += mass;
    total_inertia.moment( 2, 2 ) += mass;
    auto inverse_inertia = Invert(total_inertia);
    if(lock_rotation)
    {
        auto moment_z = inverse_inertia.moment(2,2);
        inverse_inertia.moment = Math::Float3x3(0);
        inverse_inertia.moment(2,2) = moment_z;
    }

    CorrectForCenterOfMassForward( CreateRange( &center_of_mass, 1 ), CreateRange( &orientation, 1 ) );
    collision_data.axis_aligned_box.center -= center_of_mass;

    AddRigidBodyComponent(
        body_id,
        orientation,
        center_of_mass,
        movement,
        inverse_inertia,
        collision_data.axis_aligned_box,
        bounciness,
        friction_factor,
        m_element_container);

}


void PhysicsWorld::CreateMultiRigidBodyComponent(
    EntityID entity_id,
    Orientation orientation,
    Movement movement,
    Range<ComponentDescription::Body const *> body_descriptions,
    Range<Connection const*> connections
    )
{
    assert(BodyCount(entity_id, m_body_entity_mapping) == 0);
    for(auto const & body : body_descriptions)
    {
        CreateRigidBodyComponent(
            entity_id,
            body.collision_file,
            body.mass,
            orientation,
            movement,
            body.bounciness,
            body.friction_factor,
            body.lock_rotation
            );
    }
    CreatePersistentConstraints(entity_id, connections);
}


void PhysicsWorld::CreatePersistentConstraints(EntityID entity_id, Range<Connection const *> connections)
{
    auto bodies = Bodies(entity_id, m_body_entity_mapping);
    for(auto & connection : connections)
    {
        assert(connection.child < Size(bodies));
        assert(connection.parent < Size(bodies));

        auto parent = bodies[connection.parent];
        auto child = bodies[connection.child];
        MinMax<float> infinities = {-std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()};
        switch(connection.type)
        {
            case ConnectionType::KeepPositions:
            {
                auto parent_center_of_mass = m_element_container.pointers.centers_of_mass[m_element_container.pointers.body_to_element[parent.index]];
                auto child_center_of_mass = m_element_container.pointers.centers_of_mass[m_element_container.pointers.body_to_element[child.index]];
                Append(m_persistent_constraints.position_constraints.bodies, BodyPair{parent, child}, 3);
                std::array<Math::Float3, 3> directions = {{{1,0,0},{0,1,0},{0,0,1}}};
                Append(m_persistent_constraints.position_constraints.directions, CreateRange(directions));
                Append(m_persistent_constraints.position_constraints.attachment_points, std::array<Math::Float3, 2>{{0, 0}}, 3);
                auto distance = parent_center_of_mass - child_center_of_mass;
                Append(m_persistent_constraints.position_constraints.distances, CreateRange(begin(distance), end(distance)));
                Append(m_persistent_constraints.position_constraints.minmax_forces, infinities, 3);

                Append(m_persistent_constraints.velocity_constraints.bodies, BodyPair{parent, child}, 3);
                Append(m_persistent_constraints.velocity_constraints.directions, CreateRange(directions));
                Append(m_persistent_constraints.velocity_constraints.attachment_points, std::array<Math::Float3, 2>{{0, 0}}, 3);
                Append(m_persistent_constraints.velocity_constraints.target_speeds, 0.f, 3);
                Append(m_persistent_constraints.velocity_constraints.minmax_forces, infinities, 3);

                Math::Edge edge(parent.index, child.index);
                AddSymmetricEdges(CreateRange(&edge, 1), m_non_colliding_bodies_pairs);
                break;
            }
            case ConnectionType::KeepZRotation:
            {
                Append(m_persistent_constraints.rotation_constraints.bodies, BodyPair{parent, child});
                Append(m_persistent_constraints.rotation_constraints.minmax_torques, infinities);
                Append(m_persistent_constraints.rotation_constraints.rotation_normals, Math::Float3{0, 0, 1});
                Append(m_persistent_constraints.rotation_constraints.target_angles, 0.f);
                break;
            }
            default:
            {
                assert(!"Invalid body connection type.");
            }
        }
    }
}


void PhysicsWorld::CreateKinematicBodyComponent(
    EntityID entity_id,
    std::string const & collision_file,
    Orientation orientation,
    float bounciness,
    float friction_factor
    )
{
    StoredCollisionData collision_data;
    ProvideCollisionData( collision_file, m_resource_manager, m_mesh_container, collision_data);
    auto body_id = CreateNewBodyID(entity_id, m_body_id_generator, m_body_entity_mapping);
    AddKinematicComponent( body_id, orientation, collision_data.axis_aligned_box, bounciness, friction_factor, m_element_container);
    assert(collision_data.mesh_ids.empty());
    assert(!collision_data.oriented_boxes.empty() || !collision_data.spheres.empty()); // one should not be empty
    if(!collision_data.oriented_boxes.empty())
    {
        AddBoxes(body_id, collision_data.oriented_boxes, m_oriented_box_container);
    }
    if(!collision_data.spheres.empty())
    {
        AddSpheres(body_id, collision_data.spheres, m_sphere_container);
    }
}


void PhysicsWorld::CreateStaticDensityBodyComponent(
    EntityID entity_id,
    BoundingShapes::AxisAlignedBox broad_bounds,
    DensityFunctionType sample_function,
    Orientation orientation,
    float bounciness,
    float friction_factor
    )
{
    auto body_id = CreateNewBodyID(entity_id, m_body_id_generator, m_body_entity_mapping);
    AddStaticComponent(body_id, orientation, broad_bounds, bounciness, friction_factor, m_element_container);
    AddFunction(move(sample_function), body_id, m_density_function_container);
}


void PhysicsWorld::CreateStaticBodyComponent(
    EntityID entity_id,
    std::string const & collision_file,
    Orientation orientation,
    float bounciness,
    float friction_factor
    )
{
    StoredCollisionData collision_data;
    ProvideCollisionData(collision_file, m_resource_manager, m_mesh_container, collision_data);
    assert( collision_data.mesh_ids.empty() + collision_data.oriented_boxes.empty() + collision_data.spheres.empty() < 3); // one should not be empty
    auto body_id = CreateNewBodyID(entity_id, m_body_id_generator, m_body_entity_mapping);
    if(!collision_data.mesh_ids.empty())
    {
        AddMesh(body_id, collision_data.mesh_ids.front(), m_mesh_container);
    }
    if(!collision_data.oriented_boxes.empty())
    {
        AddBoxes(body_id, collision_data.oriented_boxes, m_oriented_box_container);
    }
    if(!collision_data.spheres.empty())
    {
        AddSpheres(body_id, collision_data.spheres, m_sphere_container);
    }
    AddStaticComponent(
        body_id,
        orientation,
        collision_data.axis_aligned_box,
        bounciness,
        friction_factor,
        m_element_container);
}


void PhysicsWorld::CreateStaticNonCollidingBodyComponent(
    EntityID entity_id,
    Orientation orientation
    )
{
    m_non_colliding_bodies.AddStaticComponent( entity_id, orientation );
}


void PhysicsWorld::CreateKinematicNonCollidingBodyComponent(
    EntityID entity_id,
    Orientation orientation
    )
{
    m_non_colliding_bodies.AddKinematicComponent( entity_id, orientation );
}


void PhysicsWorld::ReplaceWithKinematicBodyComponent(
    EntityID entity_id,
    std::string const & collision_file,
    float bouncincess,
    float friction_factor
    )
{
    StoredCollisionData collision_data;
    ProvideCollisionData( collision_file, m_resource_manager, m_mesh_container, collision_data );
    assert(collision_data.mesh_ids.empty());
    // assert((collision_data.oriented_boxes.empty() + collision_data.spheres.empty()) == 1); // one should be empty
    assert(!collision_data.oriented_boxes.empty() || !collision_data.spheres.empty()); // one should not be empty

    auto bodies = Bodies(entity_id, m_body_entity_mapping);
    RemoveShapes(bodies);
    Remove(bodies, m_persistent_constraints);

    auto body_id = First(bodies);
    if(!collision_data.oriented_boxes.empty())
    {
        AddBoxes(body_id, collision_data.oriented_boxes, m_oriented_box_container);
    }
    if(!collision_data.spheres.empty())
    {
        AddSpheres(body_id, collision_data.spheres, m_sphere_container);
    }
    // first remove all but the first body
    PopFirst(bodies);
    RemoveBodies(bodies, m_element_container);
    RemoveBodiesFromNonCollidingBodyPairs(bodies);
    // this one last, as it will remove the bodies from the 'bodies' range itself
    RemoveInReverse(bodies, m_body_entity_mapping);
    // then replace the first
    Physics::ReplaceWithKinematicComponent(body_id, collision_data.axis_aligned_box, bouncincess, friction_factor, m_element_container);
}


void PhysicsWorld::ReplaceWithRigidBodyComponent(
    EntityID entity_id,
    std::string const & collision_file,
    float mass,
    float bounciness,
    float friction_factor,
    bool lock_rotation
    )
{
    assert( IsValidMass( mass ) );
    StoredCollisionData collision_data;
    ProvideCollisionData( collision_file, m_resource_manager, m_mesh_container, collision_data );
    // assert(collision_data.oriented_boxes.empty() != collision_data.spheres.empty()); // one XOR the other should not be empty
    assert(!collision_data.oriented_boxes.empty() || !collision_data.spheres.empty());
    assert(collision_data.mesh_ids.empty());
    auto bodies = Bodies(entity_id, m_body_entity_mapping);
    Remove(bodies, m_persistent_constraints);
    RemoveShapes(bodies);
    auto body_id = First(bodies);
    Inertia total_inertia;
    Math::Float3 center_of_mass;
    CalculateTotalInertia(collision_data.spheres, collision_data.oriented_boxes, mass, total_inertia, center_of_mass);
    for( auto & box : collision_data.oriented_boxes )
    {
        box.center -= center_of_mass;
    }
    for( auto & sphere : collision_data.spheres )
    {
        sphere.center -= center_of_mass;
    }
    if(!IsEmpty(collision_data.oriented_boxes))
    {
        AddBoxes( body_id, collision_data.oriented_boxes, m_oriented_box_container );
    }
    if(!IsEmpty(collision_data.spheres))
    {
        AddSpheres( body_id, collision_data.spheres, m_sphere_container );
    }
    // for stabilization we add the mass to the diagonal
    total_inertia.moment( 0, 0 ) += mass;
    total_inertia.moment( 1, 1 ) += mass;
    total_inertia.moment( 2, 2 ) += mass;
    auto inverse_inertia = Invert(total_inertia);
    if(lock_rotation)
    {
        auto moment_z = inverse_inertia.moment(2,2);
        inverse_inertia.moment = Math::Float3x3(0);
        inverse_inertia.moment(2,2) = moment_z;
    }

    collision_data.axis_aligned_box.center -= center_of_mass;
    // first remove all but the first body
    PopFirst(bodies);
    RemoveBodies(bodies, m_element_container);
    RemoveBodiesFromNonCollidingBodyPairs(bodies);
    // this one last, as it will remove the bodies from the 'bodies' range itself
    RemoveInReverse(bodies, m_body_entity_mapping);
    // then replace the first
    Physics::ReplaceWithRigidBodyComponent(
        body_id,
        center_of_mass,
        inverse_inertia,
        collision_data.axis_aligned_box,
        bounciness,
        friction_factor,
        m_element_container);
}


void PhysicsWorld::ReplaceWithMultiRigidBodyComponent(
    EntityID entity_id,
    Range<ComponentDescription::Body const *> body_descriptions,
    Range<Connection const*> connections
    )
{
    auto bodies = Bodies(entity_id, m_body_entity_mapping);
    // stuff we have to remove anyway
    Remove(bodies, m_persistent_constraints);
    RemoveShapes(bodies);
    RemoveBodiesFromNonCollidingBodyPairs(bodies);
    // if(Size(bodies) == Size(body_descriptions))
    // {
    //     // we can replace the bodies, hooray
    //     CreatePersistentConstraints(entity_id, connections);
    // }
    // else
    // {
        // just remove everything and add new stuff
        // but first get the orientation of the first body
        auto first_body = First(bodies);
        auto data_index = m_element_container.pointers.body_to_element[first_body.index];
        auto orientation = m_element_container.pointers.orientations[data_index];
        Remove(bodies, m_body_entity_mapping);
        RemoveBodies(bodies, m_element_container);
        CreateMultiRigidBodyComponent(entity_id, orientation, {}, body_descriptions, connections);
    // }
}



void PhysicsWorld::ReplaceWithStaticDensityBodyComponent(
    EntityID entity_id,
    BoundingShapes::AxisAlignedBox broad_bounds,
    DensityFunctionType sample_function,
    float bounciness,
    float friction_factor
    )
{
    auto bodies = Bodies(entity_id, m_body_entity_mapping);
    Remove(bodies, m_persistent_constraints);
    RemoveShapes(bodies);
    auto body_id = First(bodies);
    // first remove all but the first body
    PopFirst(bodies);
    RemoveBodies(bodies, m_element_container);
    RemoveBodiesFromNonCollidingBodyPairs(bodies);
    // this one last, as it will remove the bodies from the 'bodies' range itself
    RemoveInReverse(bodies, m_body_entity_mapping);
    // then replace the first
    Physics::ReplaceWithStaticComponent(body_id, broad_bounds, bounciness, friction_factor, m_element_container);
    AddFunction(move(sample_function), body_id, m_density_function_container);
}


void PhysicsWorld::ReplaceWithStaticBodyComponent(
    EntityID entity_id,
    std::string const & collision_file,
    float bounciness,
    float friction_factor
    )
{
    StoredCollisionData collision_data;
    ProvideCollisionData(collision_file, m_resource_manager, m_mesh_container, collision_data);
    assert( (collision_data.mesh_ids.empty() + collision_data.oriented_boxes.empty() + collision_data.spheres.empty()) <= 2); // one should not be empty
    auto bodies = Bodies(entity_id, m_body_entity_mapping);
    Remove(bodies, m_persistent_constraints);
    RemoveShapes(bodies);
    auto body_id = First(bodies);
    if(!collision_data.mesh_ids.empty())
    {
        AddMesh(body_id, collision_data.mesh_ids.front(), m_mesh_container);
    }
    if(!collision_data.oriented_boxes.empty())
    {
        AddBoxes(body_id, collision_data.oriented_boxes, m_oriented_box_container);
    }
    if(!collision_data.spheres.empty())
    {
        AddSpheres(body_id, collision_data.spheres, m_sphere_container);
    }
    // first remove all but the first body
    PopFirst(bodies);
    RemoveBodies(bodies, m_element_container);
    RemoveBodiesFromNonCollidingBodyPairs(bodies);
    // this one last, as it will remove the bodies from the 'bodies' range itself
    RemoveInReverse(bodies, m_body_entity_mapping);
    // then replace the first
    Physics::ReplaceWithStaticComponent(
        body_id,
        collision_data.axis_aligned_box,
        bounciness,
        friction_factor,
        m_element_container);
}


void PhysicsWorld::RemoveEntities( Range<EntityID const *> const entity_ids )
{
    std::vector<BodyID> bodies;
    AppendBodies(entity_ids, m_body_entity_mapping, bodies);
    Remove(bodies, m_body_entity_mapping);
    Remove(bodies, m_persistent_constraints);
    RemoveShapes(bodies);
    Physics::RemoveBodies( bodies, m_element_container );
    RemoveBodiesFromNonCollidingBodyPairs(bodies);
    m_moving_entities.RemoveEntities( entity_ids );
    m_non_colliding_bodies.RemoveEntities(entity_ids);
}


void PhysicsWorld::RemoveShapes( Range<EntityID const *> const entity_ids )
{
    std::vector<BodyID> bodies;
    AppendBodies(entity_ids, m_body_entity_mapping, bodies);
    RemoveShapes(bodies);
}


void PhysicsWorld::RemoveShapes(Range<BodyID const *> const bodies)
{
    Remove(bodies, m_oriented_box_container);
    Remove(bodies, m_density_function_container);
    Remove(bodies, m_mesh_container);
    Remove(bodies, m_sphere_container);
}


IndexedOrientations PhysicsWorld::GetOrientations() const
{
    auto result = IndexedOrientations();
    result.orientations = m_element_container.storage.common.orientations;
    result.previous_orientations = m_element_container.storage.common.previous_orientations;
    auto orientation_range = CreateRigidDataRange( m_element_container.offsets, result.orientations.data() );
    auto centers_of_mass = CreateRigidDataRange( m_element_container.offsets, m_element_container.pointers.centers_of_mass );
    CorrectForCenterOfMassBackward( centers_of_mass, orientation_range );
    orientation_range = CreateRigidDataRange( m_element_container.offsets, result.previous_orientations.data() );
    CorrectForCenterOfMassBackward( centers_of_mass, orientation_range );

    auto offset = uint32_t(result.orientations.size());
    Append(result.orientations, m_non_colliding_bodies.orientations);
    Append(result.previous_orientations, m_non_colliding_bodies.previous_orientations);

    auto max_size = std::max(Size(m_body_entity_mapping.entity_to_bodies), Size(m_non_colliding_bodies.entity_to_element));
    result.indices.resize(max_size, c_invalid_index);

    BodyToElementToEntityToElement(m_element_container.pointers.body_to_element, m_body_entity_mapping, result.indices);

    std::transform(
        begin( m_non_colliding_bodies.entity_to_element ),
        end( m_non_colliding_bodies.entity_to_element ),
        begin(result.indices),
        begin(result.indices ),
                    [offset]( auto a, auto b )
    {
        if( a != c_invalid_index )
        {
            assert( b == c_invalid_index );
            return a + offset;
        }
        else
        {
            return b;
        }
    } );

    return result;
}


Math::Float3 PhysicsWorld::FindRestingPositionOnEntity(
    Math::Float3 starting_position,
    EntityID target_entity
    ) const
{
    return CastRayOnEntity(BoundingShapes::RayFromStartAndDirection(starting_position, m_gravity), target_entity);
}


Math::Float3 PhysicsWorld::CastRayOnStaticEntities(
    BoundingShapes::Ray const & ray
    ) const
{
    // auto bodies = CreateStaticDataRange(m_element_container.offsets, m_element_container.pointers.body_ids);
    auto bodies = CreateStaticDataRange(m_element_container.offsets, m_element_container.pointers.body_ids);
    auto broad_bounds = CreateStaticDataRange(m_element_container.offsets, m_element_container.pointers.transformed_broad_bounds);
    auto broad_bounds_hierarchy = CreateAxisAlignedBoxHierarchy(broad_bounds);
    std::vector<BodyID> candidate_bodies;
    BroadPhaseRayCasting(
        broad_bounds,
        broad_bounds_hierarchy,
        bodies,
        ray,
        candidate_bodies
        );
    return CastRayOnBodies(ray, candidate_bodies);
}



Math::Float3 PhysicsWorld::CastRayOnBody(
    BoundingShapes::Ray const & ray,
    BodyID const & body
    ) const
{
    return CastRayOnBodies(ray, CreateRange(&body, 1));
}

Math::Float3 PhysicsWorld::CastRayOnBodies(
    BoundingShapes::Ray const & ray,
    Range<BodyID const *> bodies
    ) const
{
    auto time = std::numeric_limits<float>::infinity();
    for(auto body : bodies)
    {
        auto element_index = m_element_container.pointers.body_to_element[body.index];
        auto orientation = m_element_container.pointers.orientations[element_index];
        auto transformed_ray = TransformByOrientation(ray, Invert(orientation));
        // density function bodies
        if(Contains(body, m_density_function_container))
        {
            auto index = m_density_function_container.body_to_data[body.index];
            auto & function = m_density_function_container.functions[index];
            // auto search_direction = InverseRotate(ray.direction, orientation.rotation);
            // auto starting_position = InverseRotate( ray.start - orientation.position, orientation.rotation );
            time = Math::Min(IntersectionTime(transformed_ray, function, 1e-3f ), time);
        }
        if(Contains(body, m_oriented_box_container))
        {
            auto offset_index = m_oriented_box_container.body_to_offset[body.index];
            auto boxes = CreateRange(m_oriented_box_container.boxes, m_oriented_box_container.offsets[offset_index], m_oriented_box_container.offsets[offset_index + 1]);
            time = Math::Min(IntersectionTime(transformed_ray, boxes), time);
        }
        if(Contains(body, m_sphere_container))
        {
            auto offset_index = m_sphere_container.body_to_offset[body.index];
            auto spheres = CreateRange(m_sphere_container.spheres, m_sphere_container.offsets[offset_index], m_sphere_container.offsets[offset_index + 1]);
            time = Math::Min(IntersectionTime(transformed_ray, spheres), time);
        }
    }

    return PointAlongRay(ray, time);
}


Math::Float3 PhysicsWorld::CastRayOnEntity(
    BoundingShapes::Ray const & ray,
    EntityID target_entity
    ) const
{
    return CastRayOnBodies(ray, Bodies(target_entity, m_body_entity_mapping));
}




void PhysicsWorld::CalculateVelocities( const float time_step )
{
    m_moving_entities.entity_to_element.clear();
    m_moving_entities.velocities.clear();
    m_moving_entities.angular_velocities.clear();
    m_moving_entities.entity_ids.clear();

    Physics::UpdateVelocities(
        m_moving_entities.velocities,
        m_moving_entities.angular_velocities,
        m_moving_entities.entity_ids,
        m_moving_entities.entity_to_element,
        CreateKinematicDataRange( m_non_colliding_bodies, m_non_colliding_bodies.orientations ),
        CreateKinematicDataRange( m_non_colliding_bodies, m_non_colliding_bodies.previous_orientations ),
        CreateKinematicDataRange( m_non_colliding_bodies, m_non_colliding_bodies.entity_ids ),
        time_step
        );

    Physics::UpdateVelocities(
        m_moving_entities.velocities,
        m_moving_entities.angular_velocities,
        m_moving_entities.entity_ids,
        m_moving_entities.entity_to_element,
        CreateKinematicDataRange( m_element_container.offsets, m_element_container.pointers.orientations ),
        CreateKinematicDataRange( m_element_container.offsets, m_element_container.pointers.previous_orientations ),
        CreateKinematicDataRange( m_element_container.offsets, m_element_container.pointers.body_ids ),
        m_body_entity_mapping,
        time_step );

    Physics::UpdateVelocities(
        m_moving_entities.velocities,
        m_moving_entities.angular_velocities,
        m_moving_entities.entity_ids,
        m_moving_entities.entity_to_element,
        CreateRigidDataRange(m_element_container.offsets, m_element_container.pointers.movements),
        CreateRigidDataRange(m_element_container.offsets, m_element_container.pointers.inverse_inertias),
        CreateRigidDataRange(m_element_container.offsets, m_element_container.pointers.body_ids),
        m_body_entity_mapping );
}


void PhysicsWorld::CopyCurrentToPrevious()
{
    m_element_container.storage.common.previous_orientations = m_element_container.storage.common.orientations;
    m_non_colliding_bodies.previous_orientations = m_non_colliding_bodies.orientations;
}


void PhysicsWorld::UpdateOrientations(
    EntityPositions const & entity_positions,
    EntityRotations const & entity_rotations
    )
{
    Physics::UpdateOrientations(
        CreateAllBodyDataRange(m_element_container.offsets, m_element_container.pointers.orientations),
        m_element_container.storage.body_to_element,
        m_body_entity_mapping,
        entity_positions.positions,
        entity_positions.entity_ids );

    Physics::UpdateOrientations(
        CreateAllBodyDataRange(m_element_container.offsets, m_element_container.pointers.orientations),
        m_element_container.storage.body_to_element,
        m_body_entity_mapping,
        entity_rotations.rotations,
        entity_rotations.entity_ids );

    Physics::UpdateOrientations(
        m_non_colliding_bodies.orientations,
        m_non_colliding_bodies.entity_to_element,
        entity_positions.positions,
        entity_positions.entity_ids );

    Physics::UpdateOrientations(
        m_non_colliding_bodies.orientations,
        m_non_colliding_bodies.entity_to_element,
        entity_rotations.rotations,
        entity_rotations.entity_ids );
}


namespace
{
    void RotateInertias(
        Range<Inertia const *> inertias,
        Range<Orientation const *> orientations,
        Range<Inertia * > result_inertias
        )
    {
        assert( Size( inertias ) == Size( orientations ) );
        assert( Size( result_inertias ) == Size( orientations ) );
        if( IsEmpty( inertias ) ) return;
        std::transform( begin( inertias ), end( inertias ), begin( orientations ), begin( result_inertias ), []( Inertia inertia, Orientation const & orientation )
        {
            inertia.moment = RotateMomentOfInertia( inertia.moment, orientation.rotation );
            return inertia;
        } );
    }


    Math::Float3 SpringForce(
        Math::Float3 rest_position,
        Math::Float3 position,
        Math::Float3 velocity,
        float spring_constant,
        float damping_coefficient
        )
    {
        auto relative_position = position - rest_position;
        auto spring_force = relative_position * spring_constant;
        auto damping_force = velocity * damping_coefficient;
        auto total_force = spring_force + damping_force;
        return -total_force;
    }


    // maybe we should move this someplace else, but it depends on being able to merge the manifolds
    void RemoveDuplicates(Physics::CollisionEvents & events)
    {
        // first make sure the first entity is always smaller
        FlipIf(events, [](auto const & entities)
        {
            return entities.id1.index < entities.id2.index;
        });

        // create indices so we can reorder the events later
        auto size = uint32_t(Size(events.bodies));
        std::vector<uint32_t> indices(IntegerIterator<uint32_t>(0), IntegerIterator<uint32_t>(size));

        // sort the indices
        std::sort(begin(indices), end(indices), [&events](uint32_t a, uint32_t b)
        {
            return events.bodies[a] < events.bodies[b];
        });

        // make a compare function
        auto compare_entities = [&events](uint32_t a, uint32_t b){return events.bodies[a] == events.bodies[b];};

        // find all duplicate entity pairs
        for( auto adjacent_pair = std::adjacent_find(begin(indices), end(indices), compare_entities);
            adjacent_pair < end(indices);
            adjacent_pair = std::adjacent_find(adjacent_pair, end(indices), compare_entities))
        {
            auto first = adjacent_pair[0];
            // and merge the manifolds
            auto & manifold = events.manifolds[first];
            adjacent_pair += 1;
            for( auto first_bodies = events.bodies[first]; adjacent_pair < end( indices ) && first_bodies == events.bodies[*adjacent_pair]; ++adjacent_pair )
            {
                manifold = Physics::MergeManifolds(manifold, events.manifolds[*adjacent_pair]);
            }
        }

        // remove duplicate indices
        indices.erase(std::unique(begin(indices), end(indices), compare_entities), end(indices));

        // reorder the events
        Reorder(indices, events);

        // and remove the excess elements
        Resize(Size(indices), events);
    }


    size_t Filter(
        Math::SparseAdjacencyMatrix const & filter,
        Range<BodyAndOrientationPair *> collision_pairs
        )
    {
        return std::remove_if(begin(collision_pairs), end(collision_pairs),
                              [&filter](auto const & entity_pair)
        {
            return Exists({entity_pair.body1.index, entity_pair.body2.index}, filter);
        }) - begin(collision_pairs);
    }
}


Physics::CollisionEvents PhysicsWorld::FindCollisions( Physics::CollisionEvents collision_events ) const
{
    HRTimer timer;
    timer.Start();

    std::vector<BodyAndOrientationPair> candidate_collision_entities;
    BroadPhaseCollisionDetection(
        CreateAllBodyDataRange(m_element_container.offsets, m_element_container.pointers.transformed_broad_bounds),
        m_element_container.storage.common.transformed_broad_bounds_hierarchy,
        CreateAllBodyDataRange(m_element_container.offsets, m_element_container.pointers.orientations),
        CreateAllBodyDataRange(m_element_container.offsets, m_element_container.pointers.body_ids),
        StaticBodyEnd(m_element_container.offsets),
        candidate_collision_entities);

    Clear(collision_events);

    auto new_size = Filter(m_non_colliding_bodies_pairs, candidate_collision_entities);
    candidate_collision_entities.resize(new_size);

    NarrowPhaseCollisionDetection(
        m_sphere_container.body_to_offset,
        m_sphere_container.offsets,
        m_sphere_container.spheres,
        m_oriented_box_container.body_to_offset,
        m_oriented_box_container.offsets,
        m_oriented_box_container.boxes,
        m_density_function_container.body_to_data,
        m_density_function_container.functions,
        m_mesh_container.body_to_data,
        m_mesh_container.meshes,
        // order gets changed
        candidate_collision_entities,
        // output
        collision_events.bodies,
        collision_events.relative_positions,
        collision_events.manifolds);

    RemoveDuplicates(collision_events);

    timer.Stop();
    Log( [&timer]()
    {
        return "FindCollisions took " + std::to_string( timer.GetMilliSeconds() ) + " ms";
    } );
    return collision_events;
}


namespace
{
    void ConvertCollisionEvents(
        Physics::CollisionEvents const & physics_events,
        BodyEntityMapping const & mapping,
        ::CollisionEvents & output_events
        )
    {
        for(auto body : physics_events.bodies)
        {
            auto entity1 = Physics::Entity(body.id1, mapping);
            auto entity2 = Physics::Entity(body.id2, mapping);
            output_events.entities.emplace_back(entity1, entity2);
        }

        output_events.manifolds = physics_events.manifolds;
        output_events.relative_positions = physics_events.relative_positions;
    }


    void ConvertConstraints(
        ::RotationConstraints const & external,
        BodyEntityMapping const & mapping,
        Physics::WorldRotationConstraints & physics
        )
    {
        for(auto entity : external.entity_ids)
        {
            auto body = First(Physics::Bodies(entity, mapping));
            Append(physics.body_ids, body);
        }

        physics.minmax_torques = external.minmax_torques;
        physics.rotation_normals = external.rotation_normals;
        physics.target_angles = external.target_angles;
    }


    void ConvertConstraints(
        ::VelocityConstraints const & external,
        BodyEntityMapping const & mapping,
        Physics::WorldVelocityConstraints & physics
        )
    {
        for(auto entity : external.entity_ids)
        {
            auto body = First(Physics::Bodies(entity, mapping));
            Append(physics.body_ids, body);
        }

        physics.directions = external.directions;
        physics.minmax_force = external.minmax_force;
        physics.target_speeds = external.target_speeds;
    }


    void ConvertConstraints(
        ::AngularVelocityConstraints const & external,
        BodyEntityMapping const & mapping,
        Physics::WorldAngularVelocityConstraints & physics
        )
    {
        for(auto entity : external.entity_ids)
        {
            auto body = First(Physics::Bodies(entity, mapping));
            Append(physics.body_ids, body);
        }

        physics.angular_directions = external.angular_directions;
        physics.minmax_torque = external.minmax_torque;
        physics.angular_target_speeds = external.angular_target_speeds;
    }
}


::CollisionEvents PhysicsWorld::UpdateBodies(
    EntityForces const & entity_forces,
    EntityTorques const & entity_torque,
    ::RotationConstraints const & external_rotation_constraints,
    ::VelocityConstraints const & external_velocity_constraints,
    ::AngularVelocityConstraints const & external_angular_velocity_constraints,
    float const time_step
    )
{
    // Update broad bounds
    {
        // first transform all broad bounds
        TransformByOrientation(
            CreateDynamicDataRange(m_element_container.offsets, m_element_container.pointers.broad_bounds),
            CreateDynamicDataRange(m_element_container.offsets, m_element_container.pointers.orientations),
            CreateDynamicDataRange(m_element_container.offsets, m_element_container.pointers.transformed_broad_bounds)
            );
        // update the broad bounds hierarchy
        BoundingShapes::CreateAxisAlignedBoxHierarchy(
            CreateAllBodyDataRange(m_element_container.offsets, m_element_container.pointers.transformed_broad_bounds),
            m_element_container.storage.common.transformed_broad_bounds_hierarchy );
    }

    auto collision_events = FindCollisions(std::move(m_current_collision_events));
    CollisionEventOffsets event_offsets;
    auto body_to_element = m_element_container.pointers.body_to_element;
    SortAndCatagorize( collision_events, event_offsets, body_to_element, KinematicBodyStart( m_element_container.offsets ), RigidBodyStart( m_element_container.offsets ) );

    if(m_world_configuration.persitent_contact_expiry_age > 0)
    {
        AgeManifolds(m_previous_collision_events.manifolds, m_world_configuration.persitent_contact_expiry_age);
        UpdatePenetrationDepth(m_previous_collision_events.manifolds, m_world_configuration.position_correction_fraction);
        MergeEventManifolds(
            collision_events,
            event_offsets,
            m_previous_collision_events,
            m_previous_collision_event_offsets);
    }

    // Log( [&collision_events]()
    // {
    //     std::string out;
    //     for( size_t i = 0; i < collision_events.entities.size(); i++ )
    //     {
    //         if( i != 0 ) out += "\n";
    //         auto const & manifold = collision_events.manifolds[i];
    //         for( auto j = 0u; j < manifold.contact_point_count; ++j )
    //         {
    //             out += "Position: ";
    //             out += ToString( manifold.positions[j] );
    //             out += "\n";
    //             out += "Axis: ";
    //             out += ToString( manifold.separation_axes[j] );
    //             out += "\n";
    //             out += "Depth: ";
    //             out += Math::c_start + std::to_string( manifold.penetration_depths[j] ) + Math::c_end;
    //             out += "\n";
    //         }
    //         out += "Relative position: ";
    //         out += ToString( collision_events.relative_positions[i] );
    //     }
    //     return out;
    // } );

    // TODO: determine what to do with the kinematic resolving, maybe they also need to go through the solver?
    {
        ResolveKinematicStatic(
            CreateKinematicStaticRange(event_offsets, collision_events.bodies),
            CreateKinematicStaticRange(event_offsets, collision_events.manifolds),
            body_to_element,
            CreateAllBodyDataRange(m_element_container.offsets, m_element_container.pointers.orientations) );

        ResolveKinematicKinematic(
            CreateKinematicKinematicRange(event_offsets, collision_events.bodies),
            CreateKinematicKinematicRange(event_offsets, collision_events.manifolds),
            body_to_element,
            CreateAllBodyDataRange(m_element_container.offsets, m_element_container.pointers.orientations) );

        // we assume kinematic bodies cannot influence rigid bodies, so we pretend the rigid bodies are static for this purpose
        ResolveStaticKinematic(
            CreateRigidKinematicRange(event_offsets, collision_events.bodies),
            CreateRigidKinematicRange(event_offsets, collision_events.manifolds),
            body_to_element,
            CreateAllBodyDataRange(m_element_container.offsets, m_element_container.pointers.orientations) );
    }

    auto rigid_body_orientations = CreateRigidDataRange( m_element_container.offsets, m_element_container.pointers.orientations );
    std::vector<Inertia> rigid_body_inverse_inertias( Size( rigid_body_orientations ) );
    RotateInertias( CreateRigidDataRange( m_element_container.offsets, m_element_container.pointers.inverse_inertias ), rigid_body_orientations, rigid_body_inverse_inertias );
    std::vector<uint32_t> rigid_body_to_element(begin(body_to_element), end(body_to_element));
    for( auto & i : rigid_body_to_element)
    {
        if( i != c_invalid_index )
        {
            auto start = RigidBodyStart(m_element_container.offsets);
            if( i < start )
            {
                i = c_invalid_index;
            }
            else
            {
                i -= start;
            }
        }
    }

    auto rigid_body_forces = CreateRigidDataRange(m_element_container.offsets, m_element_container.pointers.forces);
    Zero( begin( rigid_body_forces ), Size( rigid_body_forces ) );
    AddGravity( m_gravity, rigid_body_inverse_inertias, rigid_body_forces );
    // AddForces( entity_forces, m_element_container.pointers.entity_to_element, CreateAllBodyDataRange(m_element_container.offsets, m_element_container.pointers.forces) );
    AddForces( entity_forces, m_body_entity_mapping, rigid_body_to_element, rigid_body_forces );

    auto rigid_body_movements = CreateRigidDataRange(m_element_container.offsets, m_element_container.pointers.movements);
    std::vector<Movement> old_movements(begin(rigid_body_movements), end(rigid_body_movements));
    ApplyForces( old_movements, rigid_body_forces, time_step, rigid_body_movements );
    ApplyTorques( rigid_body_movements, body_to_element, entity_torque.torques, entity_torque.entity_ids, m_body_entity_mapping, time_step, rigid_body_movements );
    auto rigid_body_bounds = CreateRigidDataRange( m_element_container.offsets, m_element_container.pointers.broad_bounds );
    ApplyDrag( rigid_body_movements, rigid_body_inverse_inertias, rigid_body_bounds, time_step );
    ApplyInternalFriction( rigid_body_movements, 1 - m_world_configuration.fixed_fraction_velocity_loss_per_second, time_step, rigid_body_movements );


    Physics::WorldRotationConstraints rotation_constraints;
    Physics::WorldVelocityConstraints velocity_constraints;
    Physics::WorldAngularVelocityConstraints angular_velocity_constraints;
    ConvertConstraints(external_rotation_constraints, m_body_entity_mapping, rotation_constraints);
    ConvertConstraints(external_velocity_constraints, m_body_entity_mapping, velocity_constraints);
    ConvertConstraints(external_angular_velocity_constraints, m_body_entity_mapping, angular_velocity_constraints);
    m_constraint_solver->SetCollisionEvents(&collision_events, &event_offsets);
    m_constraint_solver->SetConstraints(
        &m_persistent_constraints.distance_constraints,
        &m_persistent_constraints.position_constraints,
        &m_persistent_constraints.rotation_constraints,
        &m_persistent_constraints.velocity_constraints,
        &velocity_constraints,
        &angular_velocity_constraints,
        &rotation_constraints);
    m_constraint_solver->SetRigidBodyData(rigid_body_to_element, old_movements, rigid_body_movements, rigid_body_orientations, rigid_body_inverse_inertias, rigid_body_forces);
    m_constraint_solver->SetCommonBodyData(
        body_to_element,
        CreateAllBodyDataRange(m_element_container.offsets, m_element_container.pointers.bounce_factors),
        CreateAllBodyDataRange(m_element_container.offsets, m_element_container.pointers.friction_factors)
        );

    static HRTimer solver_timer;
    solver_timer.Start();
    m_constraint_solver->DoYourThing(time_step);
    solver_timer.Stop();
    auto solver_time = solver_timer.GetMilliSeconds();
    Log([solver_time](){return "Solving rigid body constraints took: " + std::to_string(solver_time) + " milliseconds.";});

    // Log([rigid_body_orientations]()
    // {
    //     std::string out;
    //     out += "Positions: ";
    //     out += Math::c_start;
    //     for( auto & orientation : rigid_body_orientations )
    //     {
    //         out += ToString( orientation.position );
    //         if(&orientation != end(rigid_body_orientations) - 1) out += Math::c_delim;
    //     }
    //     out += Math::c_end;
    //     return out;
    // });

    ::CollisionEvents output_events;
    ConvertCollisionEvents(collision_events, m_body_entity_mapping, output_events);
    m_current_collision_events = std::move(m_previous_collision_events);
    m_previous_collision_events = std::move(collision_events);
    std::swap(m_previous_collision_event_offsets, event_offsets);
    return output_events;
}


bool PhysicsWorld::HasRigidBodyComponent( EntityID entity_id ) const
{
    auto bodies = Bodies(entity_id, m_body_entity_mapping);
    return std::any_of(begin(bodies), end(bodies), [&](auto id) { return IsRigidBody(id, m_element_container); });
}


void PhysicsWorld::AdjustAllPositions(Math::Float3 adjustment)
{
    auto orientations = CreateAllBodyDataRange(m_element_container.offsets, m_element_container.pointers.orientations);
    for( auto & orientation : orientations )
    {
        orientation.position += adjustment;
    }

    orientations = CreateAllBodyDataRange(m_element_container.offsets, m_element_container.pointers.previous_orientations);
    for( auto & orientation : orientations )
    {
        orientation.position += adjustment;
    }

    auto boxes = CreateStaticDataRange(m_element_container.offsets, m_element_container.pointers.broad_bounds);
    for( auto & box : boxes )
    {
        box.center += adjustment;
    }

    orientations = m_non_colliding_bodies.orientations;
    for( auto & orientation : orientations )
    {
        orientation.position += adjustment;
    }

    orientations = m_non_colliding_bodies.previous_orientations;
    for( auto & orientation : orientations )
    {
        orientation.position += adjustment;
    }
}


void PhysicsWorld::GetAllOrientedBoxes(std::vector<BoundingShapes::OrientedBox>& boxes, std::vector<EntityID>& entities)
{
    for(auto id : m_oriented_box_container.bodies)
    {
        Append(entities, Entity(id, m_body_entity_mapping));
    }
    auto output_boxes = Append(boxes, m_oriented_box_container.boxes);

    auto centers_of_mass = CreateAllBodyDataRange(m_element_container.offsets, m_element_container.pointers.centers_of_mass);
    auto orientations = CreateAllBodyDataRange(m_element_container.offsets, m_element_container.pointers.orientations);
    for (auto i = 0u; i < Size(m_oriented_box_container.bodies); ++i)
    {
        auto body = m_oriented_box_container.bodies[i];
        if(IsRigidBody(body , m_element_container))
        {
            auto index = m_element_container.pointers.body_to_element[body.index];
            auto first_body = First(Bodies(entities[i], m_body_entity_mapping));
            if(first_body != body)
            {
                auto first_index = m_element_container.pointers.body_to_element[first_body.index];
                std::array<Orientation, 2> shape_orientations;
                shape_orientations[0] = orientations[first_index];
                shape_orientations[1] = orientations[index];
                std::array<Math::Float3, 2> shape_center_of_masses;
                shape_center_of_masses[0] = centers_of_mass[first_index];
                shape_center_of_masses[1] = centers_of_mass[index];
                CorrectForCenterOfMassBackward(shape_center_of_masses, shape_orientations);
                //auto relative_orientation = GetOffset(orientation, first_orientation);
                output_boxes[i].center += shape_center_of_masses[1];
                output_boxes[i] = TransformByOrientation(output_boxes[i], shape_orientations[1]);
                output_boxes[i] = TransformByOrientation(output_boxes[i], Invert(shape_orientations[0]));
            }
            else
            {
                output_boxes[i].center += centers_of_mass[index];
            }
        }
    }
}


void PhysicsWorld::GetAllSpheres(
    std::vector<BoundingShapes::Sphere>& boxes,
    std::vector<EntityID>& entities
    )
{
    for(auto id : m_sphere_container.bodies)
    {
        Append(entities, Entity(id, m_body_entity_mapping));
    }
    auto output_spheres = Append(boxes, m_sphere_container.spheres);

    auto centers_of_mass = CreateAllBodyDataRange(m_element_container.offsets, m_element_container.pointers.centers_of_mass);
    auto orientations = CreateAllBodyDataRange(m_element_container.offsets, m_element_container.pointers.orientations);
    for (auto i = 0u; i < Size(m_sphere_container.bodies); ++i)
    {
        auto body = m_sphere_container.bodies[i];
        if(IsRigidBody(body, m_element_container))
        {
            auto index = m_element_container.pointers.body_to_element[body.index];
            auto first_body = First(Bodies(entities[i], m_body_entity_mapping));
            if(first_body != body)
            {
                auto first_index = m_element_container.pointers.body_to_element[first_body.index];
                std::array<Orientation, 2> shape_orientations;
                shape_orientations[0] = orientations[first_index];
                shape_orientations[1] = orientations[index];
                std::array<Math::Float3, 2> shape_center_of_masses;
                shape_center_of_masses[0] = centers_of_mass[first_index];
                shape_center_of_masses[1] = centers_of_mass[index];
                CorrectForCenterOfMassBackward(shape_center_of_masses, shape_orientations);
                //auto relative_orientation = GetOffset(orientation, first_orientation);
                output_spheres[i].center += shape_center_of_masses[1];
                output_spheres[i] = TransformByOrientation(output_spheres[i], shape_orientations[1]);
                output_spheres[i] = TransformByOrientation(output_spheres[i], Invert(shape_orientations[0]));
            }
            else
            {
                output_spheres[i].center += centers_of_mass[index];
            }
        }
    }
}


void PhysicsWorld::AddNonCollidingEntityPairs(Range<EntityPair const *> entity_pairs)
{
    std::vector<Math::Edge> edges;
    edges.reserve(Size(entity_pairs));
    for(auto p : entity_pairs)
    {
        auto bodies1 = Bodies(p.id1, m_body_entity_mapping);
        auto bodies2 = Bodies(p.id2, m_body_entity_mapping);
        for(auto b1 : bodies1)
        {
            for(auto b2 : bodies2)
            {
                edges.emplace_back(b1.index, b2.index);
            }
        }
    }
    AddSymmetricEdges(edges, m_non_colliding_bodies_pairs);
}



void PhysicsWorld::RemoveNonCollidingEntityPairs(Range<EntityPair const *> entity_pairs)
{
    std::vector<Math::Edge> edges;
    edges.reserve(Size(entity_pairs));
    for(auto p : entity_pairs)
    {
        auto bodies1 = Bodies(p.id1, m_body_entity_mapping);
        auto bodies2 = Bodies(p.id2, m_body_entity_mapping);
        for(auto b1 : bodies1)
        {
            for(auto b2 : bodies2)
            {
                edges.emplace_back(b1.index, b2.index);
            }
        }
    }
    RemoveSymmetricEdges(edges, m_non_colliding_bodies_pairs);
}


void PhysicsWorld::RemoveBodiesFromNonCollidingBodyPairs(Range<BodyID const *> bodies)
{
    std::vector<uint32_t> vertices;
    vertices.reserve(Size(bodies));
    for(auto b : bodies)
    {
        vertices.emplace_back(b.index);
    }
    RemoveVertices(vertices, m_non_colliding_bodies_pairs);
}
