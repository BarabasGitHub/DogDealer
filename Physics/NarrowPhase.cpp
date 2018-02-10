#include "NarrowPhase.h"

#include "ManifoldFunctions.h"

#include <Conventions\OrientationFunctions.h>
#include <BoundingShapes\OrientedBoxFunctions.h>
#include <BoundingShapes\SphereFunctions.h>
#include <Utilities\VectorHelper.h>
#include <Math\MathFunctions.h>

namespace
{
    using namespace Physics;

    BodyAndOrientationPair Flip(BodyAndOrientationPair pair)
    {
        using std::swap;
        swap(pair.body1, pair.body2);
        swap(pair.orientation1, pair.orientation2);
        return pair;
    }


    void FlipAll(Range<BodyAndOrientationPair*> pairs)
    {
        for( auto & pair : pairs )
        {
            pair = Flip(pair);
        }
    }


    struct CollisionCatagories
    {
        std::vector<BodyAndOrientationPair> all;
        Range<BodyAndOrientationPair *>
            sphere_vs_sphere,
            sphere_vs_box,
            sphere_vs_density_function,
            sphere_vs_mesh,
            box_vs_box,
            box_vs_density_function,
            box_vs_mesh;
    };


    uint32_t CopyBothWays(
        Range<BodyAndOrientationPair const *> entities_and_orientations,
        Range<uint32_t const *> bodyy_to_shape1,
        Range<uint32_t const *> body_to_shape2,
        std::vector<BodyAndOrientationPair> & storage)
    {
        auto offset = Size(storage);
        for( auto i = 0u; i < Size(entities_and_orientations); ++i )
        {
            auto pair = entities_and_orientations[i];
            if(
                (GetOptional(bodyy_to_shape1, pair.body1.index) != c_invalid_index) &&
                (GetOptional(body_to_shape2, pair.body2.index) != c_invalid_index)
                )
            {
                storage.push_back(pair);
            }
            if(
                (GetOptional(bodyy_to_shape1, pair.body2.index) != c_invalid_index) &&
                (GetOptional(body_to_shape2, pair.body1.index) != c_invalid_index)
                )
            {
                storage.push_back(Flip(pair));
            }
        }
        return uint32_t( Size( storage ) - offset );
    }


    uint32_t CopyIfBoth(
        Range<BodyAndOrientationPair const *> entities_and_orientations,
        Range<uint32_t const *> body_to_shape,
        std::vector<BodyAndOrientationPair> & storage)
    {
        auto offset = Size(storage);
        for( auto i = 0u; i < Size(entities_and_orientations); ++i )
        {
            auto pair = entities_and_orientations[i];
            if(
                (GetOptional(body_to_shape, pair.body1.index) != c_invalid_index) &&
                (GetOptional(body_to_shape, pair.body2.index) != c_invalid_index)
                )
            {
                storage.push_back(pair);
            }
        }
        return uint32_t(Size( storage ) - offset);
    }


    Range<BodyAndOrientationPair *> FindAll(
        Range<BodyAndOrientationPair *> entities_and_orientations,
        Range<uint32_t const *> body_to_shape)
    {
        auto the_end = std::partition(begin(entities_and_orientations), end(entities_and_orientations),
            [body_to_shape](auto eaop)
            {
                return
                    (GetOptional(body_to_shape, eaop.body1.index) != c_invalid_index) ||
                    (GetOptional(body_to_shape, eaop.body2.index) != c_invalid_index);
            });
        return CreateRange(begin(entities_and_orientations), the_end);
    }


    void CategorizeBodiesAndOrientations(
        Range<uint32_t const *> body_to_sphere,
        Range<uint32_t const *> body_to_box,
        Range<uint32_t const *> body_to_density_function,
        Range<uint32_t const *> body_to_mesh,
        Range<BodyAndOrientationPair *> entities_and_orientations,
        CollisionCatagories & catagories
        )
    {
        catagories.all.clear();
        catagories.all.reserve(Size(entities_and_orientations));

        std::array<uint32_t, 7> ends;

        auto spheres = FindAll(entities_and_orientations, body_to_sphere);
        ends[0] = CopyIfBoth(spheres, body_to_sphere, catagories.all);
        ends[1] = ends[0] + CopyBothWays(spheres, body_to_sphere, body_to_box, catagories.all);
        ends[2] = ends[1] + CopyBothWays(spheres, body_to_sphere, body_to_density_function, catagories.all);
        ends[3] = ends[2] + CopyBothWays(spheres, body_to_sphere, body_to_mesh, catagories.all);

        auto boxes = FindAll(entities_and_orientations, body_to_box);
        ends[4] = ends[3] + CopyIfBoth(boxes, body_to_box, catagories.all);
        ends[5] = ends[4] + CopyBothWays(boxes, body_to_box, body_to_density_function, catagories.all);
        ends[6] = ends[5] + CopyBothWays(boxes, body_to_box, body_to_mesh, catagories.all);

        catagories.sphere_vs_sphere = CreateRange(catagories.all, 0, ends[0]);
        catagories.sphere_vs_box = CreateRange(catagories.all, ends[0], ends[1]);
        catagories.sphere_vs_density_function = CreateRange(catagories.all, ends[1], ends[2]);
        catagories.sphere_vs_mesh = CreateRange(catagories.all, ends[2], ends[3]);
        catagories.box_vs_box = CreateRange(catagories.all, ends[3], ends[4]);
        catagories.box_vs_density_function = CreateRange(catagories.all, ends[4], ends[5]);
        catagories.box_vs_mesh = CreateRange(catagories.all, ends[5], ends[6]);
    }

}


void Physics::NarrowPhaseCollisionDetection(
        Range<BodyAndOrientationPair const *> body_and_orientation_pairs,
        Range<uint32_t const *> body_to_sphere_offset,
        Range<uint32_t const *> sphere_offests,
        Range<BoundingShapes::Sphere const *> spheres,
        std::vector<BodyPair> & collided_bodies,
        std::vector<Math::Float3> & relative_positions,
        std::vector<Manifold> & collision_manifolds)
{
    for( auto i = 0u; i < Size(body_and_orientation_pairs); ++i )
    {
        auto const thingy = body_and_orientation_pairs[i];
        auto const sphere_offset_index1 = body_to_sphere_offset[thingy.body1.index];
        auto const end_offset1 = sphere_offests[sphere_offset_index1 + 1];
        auto const sphere_offset_index2 = body_to_sphere_offset[thingy.body2.index];
        auto const begin_offset2 = sphere_offests[sphere_offset_index2];
        auto const end_offset2 = sphere_offests[sphere_offset_index2 + 1];
        auto const orientation1 = thingy.orientation1;
        auto orientation2 = thingy.orientation2;
        orientation2.position -= orientation1.position;
        Manifold manifold;
        for( auto j = sphere_offests[sphere_offset_index1]; j < end_offset1; ++j )
        {
            auto const sphere1 = spheres[j];
            auto const transformed_sphere1 = Rotate(sphere1, orientation1.rotation);
            for( auto k = begin_offset2; k < end_offset2; ++k )
            {
                auto const sphere2 = spheres[k];
                auto const transformed_sphere2 = TransformByOrientation( sphere2, orientation2 );

                auto new_manifold = CreateManifold( transformed_sphere1, transformed_sphere2 );
                manifold = MergeManifolds(manifold, new_manifold);
            }
        }
        if( manifold.contact_point_count > 0 )
        {
            auto const relative_position = orientation2.position;
            collision_manifolds.push_back(manifold);
            relative_positions.push_back(relative_position);
            collided_bodies.emplace_back(thingy.body1, thingy.body2);
        }
    }
}

// sphere vs box
void Physics::NarrowPhaseCollisionDetection(
    Range<BodyAndOrientationPair const *> body_and_orientation_pairs,
    Range<uint32_t const *> body_to_sphere_offset,
    Range<uint32_t const *> sphere_offests,
    Range<BoundingShapes::Sphere const *> spheres,
    Range<uint32_t const *> body_to_box_offset,
    Range<uint32_t const *> box_offests,
    Range<BoundingShapes::OrientedBox const *> boxes,
    std::vector<BodyPair> & collided_bodies,
    std::vector<Math::Float3> & relative_positions,
    std::vector<Manifold> & collision_manifolds)
{
    for( auto i = 0u; i < Size(body_and_orientation_pairs); ++i )
    {
        auto const thingy = body_and_orientation_pairs[i];
        auto const sphere_offset_index = body_to_sphere_offset[thingy.body1.index];
        auto const sphere_end_offset = sphere_offests[sphere_offset_index + 1];
        auto const box_offset_index = body_to_box_offset[thingy.body2.index];
        auto const box_begin_offset = box_offests[box_offset_index];
        auto const box_end_offset = box_offests[box_offset_index + 1];
        auto const orientation1 = thingy.orientation1;
        auto orientation2 = thingy.orientation2;
        orientation2.position -= orientation1.position;
        Manifold manifold;
        for( auto j = sphere_offests[sphere_offset_index]; j < sphere_end_offset; ++j )
        {
            auto const sphere = spheres[j];
            auto const transformed_sphere = Rotate(sphere, orientation1.rotation);
            for( auto k = box_begin_offset; k < box_end_offset; ++k )
            {
                auto const box = boxes[k];
                auto const transformed_box = TransformByOrientation( box, orientation2 );

                auto new_manifold = CreateManifold( transformed_sphere, transformed_box );
                manifold = MergeManifolds(manifold, new_manifold);
            }
        }
        if( manifold.contact_point_count > 0 )
        {
            auto const relative_position = orientation2.position;
            collision_manifolds.push_back(manifold);
            relative_positions.push_back(relative_position);
            collided_bodies.emplace_back(thingy.body1, thingy.body2);
        }
    }
}


// sphere vs density
void Physics::NarrowPhaseCollisionDetection(
        Range<BodyAndOrientationPair const *> body_and_orientation_pairs,
        Range<uint32_t const *> body_to_sphere_offset,
        Range<uint32_t const *> sphere_offests,
        Range<BoundingShapes::Sphere const *> spheres,
        Range<uint32_t const *> body_to_density_function,
        Range<DensityFunctionType const *> density_functions,
        std::vector<BodyPair> & collided_bodies,
        std::vector<Math::Float3> & relative_positions,
        std::vector<Manifold> & collision_manifolds)
{
    for( auto i = 0u; i < Size(body_and_orientation_pairs); ++i )
    {
        auto thingy = body_and_orientation_pairs[i];
        auto const orientation1 = thingy.orientation1;
        auto const orientation2 = thingy.orientation2;
        auto const & density_function = density_functions[body_to_density_function[thingy.body2.index]];
        auto const relative_orientation = ToParentFromLocal(Invert(orientation2), orientation1);

        auto sphere_offset_index = body_to_sphere_offset[thingy.body1.index];
        auto end_offset = sphere_offests[sphere_offset_index + 1];
        Manifold manifold;
        for( auto j = sphere_offests[sphere_offset_index]; j < end_offset; ++j )
        {
            auto const & sphere = spheres[j];
            auto const transformed_sphere = TransformByOrientation( sphere, relative_orientation );

            // create the manifold in the local space of the mesh / second body
            Manifold new_manifold = CreateManifold( transformed_sphere, density_function );
            manifold = MergeManifolds(manifold, new_manifold);
        }
        if( manifold.contact_point_count > 0 )
        {
            // transform to world coordinates, but relative to the position of the first body
            for( auto p = 0; p < manifold.contact_point_count; p++ )
            {
                // rotate to world coordinates
                auto world_rot = Rotate( manifold.positions[p], orientation2.rotation );
                // make position relative to body 1
                auto rel_1 = world_rot - relative_orientation.position;
                manifold.positions[p] = rel_1;

                // rotate the axis to the world space
                manifold.separation_axes[p] = Rotate( manifold.separation_axes[p], orientation2.rotation );
            }

            auto const relative_position = relative_orientation.position;
            collision_manifolds.push_back(manifold);
            relative_positions.push_back(relative_position);
            collided_bodies.emplace_back(thingy.body1, thingy.body2);
        }
    }
}

// box vs box
void Physics::NarrowPhaseCollisionDetection(
        Range<BodyAndOrientationPair const *> body_and_orientation_pairs,
        Range<uint32_t const *> body_to_box_offset,
        Range<uint32_t const *> box_offests,
        Range<BoundingShapes::OrientedBox const *> boxes,
        std::vector<BodyPair> & collided_bodies,
        std::vector<Math::Float3> & relative_positions,
        std::vector<Manifold> & collision_manifolds)
{
    for( auto i = 0u; i < Size(body_and_orientation_pairs); ++i )
    {
        auto const thingy = body_and_orientation_pairs[i];
        auto const box_offset_index1 = body_to_box_offset[thingy.body1.index];
        auto const end_offset1 = box_offests[box_offset_index1 + 1];
        auto const box_offset_index2 = body_to_box_offset[thingy.body2.index];
        auto const begin_offset2 = box_offests[box_offset_index2];
        auto const end_offset2 = box_offests[box_offset_index2 + 1];
        auto orientation2 = thingy.orientation2;
        orientation2.position -= thingy.orientation1.position;
        Manifold manifold;
        for( auto j = box_offests[box_offset_index1]; j < end_offset1; ++j )
        {
            auto const box1 = boxes[j];
            auto const transformed_box1 = Rotate( box1, thingy.orientation1.rotation );
            for( auto k = begin_offset2; k < end_offset2; ++k )
            {
                auto const box2 = boxes[k];
                auto const transformed_box2 = TransformByOrientation(box2, orientation2 );

                auto new_manifold = CreateManifold( transformed_box1, transformed_box2 );
                manifold = MergeManifolds(manifold, new_manifold);
            }
        }
        if( manifold.contact_point_count > 0 )
        {
            auto const relative_position = orientation2.position;
            collision_manifolds.push_back(manifold);
            relative_positions.push_back(relative_position);
            collided_bodies.emplace_back(thingy.body1, thingy.body2);
        }
    }
}


void Physics::NarrowPhaseCollisionDetection(
        Range<BodyAndOrientationPair const *> body_and_orientation_pairs,
        Range<uint32_t const *> body_to_box_offset,
        Range<uint32_t const *> box_offests,
        Range<BoundingShapes::OrientedBox const *> boxes,
        Range<uint32_t const *> body_to_density_function,
        Range<DensityFunctionType const *> density_functions,
        std::vector<BodyPair> & collided_bodies,
        std::vector<Math::Float3> & relative_positions,
        std::vector<Manifold> & collision_manifolds)
{
    for( auto i = 0u; i < Size(body_and_orientation_pairs); ++i )
    {
        auto thingy = body_and_orientation_pairs[i];
        auto const orientation1 = thingy.orientation1;
        auto const orientation2 = thingy.orientation2;
        auto const & density_function = density_functions[body_to_density_function[thingy.body2.index]];
        auto const relative_orientation = ToParentFromLocal(Invert(orientation2), orientation1);

        auto box_offset_index = body_to_box_offset[thingy.body1.index];
        auto end_offset = box_offests[box_offset_index + 1];
        Manifold manifold;
        for( auto j = box_offests[box_offset_index]; j < end_offset; ++j )
        {
            auto const & box = boxes[j];
            auto const transformed_box = TransformByOrientation( box, relative_orientation );

            // create the manifold in the local space of the mesh / second body
            Manifold new_manifold = CreateManifold( transformed_box, density_function );
            manifold = MergeManifolds(manifold, new_manifold);
        }
        if( manifold.contact_point_count > 0 )
        {
            // transform to world coordinates, but relative to the position of the first body
            for( auto p = 0; p < manifold.contact_point_count; p++ )
            {
                // rotate to world coordinates
                manifold.positions[p] = Rotate( manifold.positions[p] - relative_orientation.position, orientation2.rotation );

                // rotate the axis to the world space
                manifold.separation_axes[p] = Rotate( manifold.separation_axes[p], orientation2.rotation );
            }

            auto const relative_position = relative_orientation.position;
            collision_manifolds.push_back(manifold);
            relative_positions.push_back(relative_position);
            collided_bodies.emplace_back(thingy.body1, thingy.body2);
        }
    }
}


void Physics::NarrowPhaseCollisionDetection(
        Range<BodyAndOrientationPair const *> body_and_orientation_pairs,
        Range<uint32_t const *> body_to_box_offset,
        Range<uint32_t const *> box_offests,
        Range<BoundingShapes::OrientedBox const *> boxes,
        Range<uint32_t const *> body_to_mesh,
        Range<BoundingShapes::AxisAlignedBoxHierarchyMesh const *> meshes,
        std::vector<BodyPair> & collided_bodies,
        std::vector<Math::Float3> & relative_positions,
        std::vector<Manifold> & collision_manifolds)
{
    for( auto i = 0u; i < Size(body_and_orientation_pairs); ++i )
    {
        auto thingy = body_and_orientation_pairs[i];
        auto const orientation1 = thingy.orientation1;
        auto const orientation2 = thingy.orientation2;
        auto const & mesh = meshes[body_to_mesh[thingy.body2.index]];
        auto const relative_orientation = ToParentFromLocal(Invert(orientation2), orientation1);

        auto box_offset_index = body_to_box_offset[thingy.body1.index];
        auto end_offset = box_offests[box_offset_index + 1];
        Manifold manifold;
        for( auto j = box_offests[box_offset_index]; j < end_offset; ++j )
        {
            auto const & box = boxes[j];
            auto const transformed_box = TransformByOrientation( box, relative_orientation );

            // create the manifold in the local space of the mesh / second body
            Manifold new_manifold = CreateManifold( transformed_box, mesh );
            manifold = MergeManifolds(manifold, new_manifold);
        }
        if( manifold.contact_point_count > 0 )
        {
            // transform to world coordinates, but relative to the position of the first body
            for( auto p = 0; p < manifold.contact_point_count; p++ )
            {
                // rotate to world coordinates
                auto world_rot = Rotate( manifold.positions[p], orientation2.rotation );
                // make position relative to body 1
                auto rel_1 = world_rot - relative_orientation.position;
                manifold.positions[p] = rel_1;
                // rotate the axis to the world space
                manifold.separation_axes[p] = Rotate( manifold.separation_axes[p], orientation2.rotation );
            }

            auto const relative_position = relative_orientation.position;
            collision_manifolds.push_back(manifold);
            relative_positions.push_back(relative_position);
            collided_bodies.emplace_back(thingy.body1, thingy.body2);
        }
    }
}


void Physics::NarrowPhaseCollisionDetection(
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
    Range<BodyAndOrientationPair *> entities_and_orientations,
    // output
    std::vector<BodyPair> & collided_bodies,
    std::vector<Math::Float3> & relative_positions,
    std::vector<Manifold> & collision_manifolds
    )
{

    CollisionCatagories catagories;
    CategorizeBodiesAndOrientations(
        body_to_sphere_offset,
        body_to_box_offset,
        body_to_density_function,
        body_to_mesh,
        entities_and_orientations,
        catagories);

    // sphere vs sphere
    NarrowPhaseCollisionDetection(
         catagories.sphere_vs_sphere,
         body_to_sphere_offset,
         sphere_offests,
         spheres,
         collided_bodies,
         relative_positions,
         collision_manifolds);

    // sphere vs box
    NarrowPhaseCollisionDetection(
        catagories.sphere_vs_box,
        body_to_sphere_offset,
        sphere_offests,
        spheres,
        body_to_box_offset,
        box_offests,
        boxes,
        collided_bodies,
        relative_positions,
        collision_manifolds);

    // sphere vs density
    NarrowPhaseCollisionDetection(
        catagories.sphere_vs_density_function,
        body_to_sphere_offset,
        sphere_offests,
        spheres,
        body_to_density_function,
        density_functions,
        collided_bodies,
        relative_positions,
        collision_manifolds);

    assert(IsEmpty(catagories.sphere_vs_mesh) && "Not implemented yet.");

    // box vs box
    NarrowPhaseCollisionDetection(
         catagories.box_vs_box,
         body_to_box_offset,
         box_offests,
         boxes,
         collided_bodies,
         relative_positions,
         collision_manifolds);

    // box vs density
    NarrowPhaseCollisionDetection(
        catagories.box_vs_density_function,
        body_to_box_offset,
        box_offests,
        boxes,
        body_to_density_function,
        density_functions,
        collided_bodies,
        relative_positions,
        collision_manifolds);


    NarrowPhaseCollisionDetection(
        catagories.box_vs_mesh,
        body_to_box_offset,
        box_offests,
        boxes,
        body_to_mesh,
        meshes,
        collided_bodies,
        relative_positions,
        collision_manifolds);
}
