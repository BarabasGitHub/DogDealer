#pragma once

#include "BodyAndOrientationPair.h"
#include "DensityFunction.h"

#include <Conventions\CollisionEvent.h>
#include <BoundingShapes\AxisAlignedBoxHierarchyMesh.h>
#include <Utilities\Range.h>

#include <cstdint>


namespace BoundingShapes
{
    struct OrientedBox;
    struct Sphere;
}


namespace Physics
{
    // sphere vs sphere
    void NarrowPhaseCollisionDetection(
        Range<BodyAndOrientationPair const *> body_and_orientation_pairs,
        Range<uint32_t const *> body_to_sphere_offset,
        Range<uint32_t const *> sphere_offests,
        Range<BoundingShapes::Sphere const *> spheres,
        std::vector<BodyPair> & collided_bodies,
        std::vector<Math::Float3> & relative_positions,
        std::vector<Manifold> & collision_manifolds);

    // sphere vs box
    void NarrowPhaseCollisionDetection(
        Range<BodyAndOrientationPair const *> body_and_orientation_pairs,
        Range<uint32_t const *> body_to_sphere_offset,
        Range<uint32_t const *> sphere_offests,
        Range<BoundingShapes::Sphere const *> spheres,
        Range<uint32_t const *> body_to_box_offset,
        Range<uint32_t const *> box_offests,
        Range<BoundingShapes::OrientedBox const *> boxes,
        std::vector<BodyPair> & collided_bodies,
        std::vector<Math::Float3> & relative_positions,
        std::vector<Manifold> & collision_manifolds);

    // sphere vs density
    void NarrowPhaseCollisionDetection(
        Range<BodyAndOrientationPair const *> body_and_orientation_pairs,
        Range<uint32_t const *> body_to_sphere_offset,
        Range<uint32_t const *> sphere_offests,
        Range<BoundingShapes::Sphere const *> spheres,
        Range<uint32_t const *> body_to_density_function,
        Range<DensityFunctionType const *> density_functions,
        std::vector<BodyPair> & collided_bodies,
        std::vector<Math::Float3> & relative_positions,
        std::vector<Manifold> & collision_manifolds);


    // box vs box
    void NarrowPhaseCollisionDetection(
        Range<BodyAndOrientationPair const *> body_and_orientation_pairs,
        Range<uint32_t const *> body_to_box_offset,
        Range<uint32_t const *> box_offests,
        Range<BoundingShapes::OrientedBox const *> boxes,
        std::vector<BodyPair> & collided_bodies,
        std::vector<Math::Float3> & relative_positions,
        std::vector<Manifold> & collision_manifolds);

    // box vs density
    void NarrowPhaseCollisionDetection(
        Range<BodyAndOrientationPair const *> body_and_orientation_pairs,
        Range<uint32_t const *> body_to_box_offset,
        Range<uint32_t const *> box_offests,
        Range<BoundingShapes::OrientedBox const *> boxes,
        Range<uint32_t const *> body_to_density_function,
        Range<DensityFunctionType const *> density_functions,
        std::vector<BodyPair> & collided_bodies,
        std::vector<Math::Float3> & relative_positions,
        std::vector<Manifold> & collision_manifolds);

    // box vs mesh
    void NarrowPhaseCollisionDetection(
        Range<BodyAndOrientationPair const *> body_and_orientation_pairs,
        Range<uint32_t const *> body_to_box_offset,
        Range<uint32_t const *> box_offests,
        Range<BoundingShapes::OrientedBox const *> boxes,
        Range<uint32_t const *> body_to_mesh,
        Range<BoundingShapes::AxisAlignedBoxHierarchyMesh const *> meshes,
        std::vector<BodyPair> & collided_bodies,
        std::vector<Math::Float3> & relative_positions,
        std::vector<Manifold> & collision_manifolds);

    // everything vs everything
    void NarrowPhaseCollisionDetection(
        Range<uint32_t const *> body_to_sphere_offset,
        Range<uint32_t const *> sphere_offests,
        Range<BoundingShapes::Sphere const *> spheres,
        Range<uint32_t const *> body_to_box_offset,
        Range<uint32_t const *> box_offests,
        Range<BoundingShapes::OrientedBox const *> boxes,
        Range<uint32_t const *> body_to_density_function,
        Range<DensityFunctionType const *> density_functions,
        Range<uint32_t const *> body_to_mesh,
        Range<BoundingShapes::AxisAlignedBoxHierarchyMesh const *> meshes,
        // order gets changed
        Range<BodyAndOrientationPair *> bodies_and_orientations,
        // output
        std::vector<BodyPair> & collided_bodies,
        std::vector<Math::Float3> & relative_positions,
        std::vector<Manifold> & collision_manifolds
        );
}
