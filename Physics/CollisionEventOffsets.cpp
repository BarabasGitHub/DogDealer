#include "CollisionEventOffsets.h"
#include "CollisionEvent.h"
#include "ManifoldFunctions.h"

#include <Utilities\IntegerIterator.h>
#include <Utilities\IntegerRange.h>

using namespace Physics;

void Physics::SortAndCatagorize(
    CollisionEvents & collision_events,
    CollisionEventOffsets& collision_event_ranges,
    Range<uint32_t const *> body_to_index,
    uint32_t kinematic_body_start_index,
    uint32_t rigid_body_start_index )
{
    assert(kinematic_body_start_index <= rigid_body_start_index);
    PutHigherOrderBodyFirst( body_to_index, collision_events );

    auto size = uint32_t(Size(collision_events.bodies));
    std::vector<uint32_t> indices(IntegerIterator<uint32_t>(0), IntegerIterator<uint32_t>(size));

    auto kinematic_static_start = begin( indices );

    // before this is kinematic vs something (NOT rigid vs kinematic)
    auto rigid_kinematic_start = std::partition( kinematic_static_start, end( indices ),
        [&collision_events, body_to_index, rigid_body_start_index]( uint32_t i )
    {
        return body_to_index[collision_events.bodies[i].id1.index] < rigid_body_start_index;
    } );

    // before this is kinematic static
    auto kinematic_kinematic_start = std::partition( kinematic_static_start, rigid_kinematic_start,
        [&collision_events, body_to_index, kinematic_body_start_index]( uint32_t i )
    {
        return body_to_index[collision_events.bodies[i].id2.index] < kinematic_body_start_index;
    } );

    // before this we have rigid vs something, but not rigid vs rigid
    auto rigid_rigid_start = std::partition( rigid_kinematic_start, end(indices),
        [&collision_events, body_to_index, rigid_body_start_index]( uint32_t i )
    {
        return body_to_index[collision_events.bodies[i].id2.index] < rigid_body_start_index;
    } );

    // before this is rigid kinematic, after rigid static
    auto rigid_static_start = std::partition( rigid_kinematic_start, rigid_rigid_start,
        [&collision_events, body_to_index, kinematic_body_start_index]( uint32_t i )
    {
        return body_to_index[collision_events.bodies[i].id2.index] >= kinematic_body_start_index;
    } );

    // sort so we can easily merge later on
    auto body_comparer = [&collision_events](uint32_t a, uint32_t b)
    {
        auto index1_a = collision_events.bodies[a].id1.index;
        auto index1_b = collision_events.bodies[b].id1.index;
        if( index1_a == index1_b )
        {
            auto index2_a = collision_events.bodies[a].id2.index;
            auto index2_b = collision_events.bodies[b].id2.index;
            return index2_a < index2_b;
        }
        else
        {
            return index1_a < index1_b;
        }
    };

    std::sort(kinematic_static_start, kinematic_kinematic_start, body_comparer);
    std::sort(kinematic_kinematic_start, rigid_kinematic_start, body_comparer);
    std::sort(rigid_kinematic_start, rigid_static_start, body_comparer);
    std::sort(rigid_static_start, rigid_rigid_start, body_comparer);
    std::sort(rigid_rigid_start, end(indices), body_comparer);

    Reorder(indices, collision_events);
    collision_event_ranges.kinematic_static = uint32_t(kinematic_static_start - begin(indices));
    collision_event_ranges.kinematic_kinematic = uint32_t(kinematic_kinematic_start - begin(indices));
    collision_event_ranges.rigid_kinematic = uint32_t(rigid_kinematic_start - begin(indices));
    collision_event_ranges.rigid_static = uint32_t(rigid_static_start - begin(indices));
    collision_event_ranges.rigid_rigid = uint32_t(rigid_rigid_start - begin(indices));
}


namespace
{
    void MergeManifoldsRange(
        CollisionEvents const & previous_collision_events, IntegerRange<uint32_t> previous_index_range,
        CollisionEvents & current_collision_events, IntegerRange<uint32_t> current_index_range)
    {
        if(IsEmpty(current_index_range)) return;

        auto current_bodies_range = CreateRange(current_collision_events.bodies, First(current_index_range), *end(current_index_range));
        auto current_manifold_range = CreateRange(current_collision_events.manifolds, First(current_index_range), *end(current_index_range));
        for( auto i = begin(previous_index_range); i < end(previous_index_range); ++i )
        {
            auto body_pair = previous_collision_events.bodies[*i];
            auto found = std::lower_bound(begin(current_bodies_range), end(current_bodies_range), body_pair);
            if(found != end(current_bodies_range) && (*found == body_pair))
            {
                auto found_index = found - begin(current_bodies_range);
                auto & current_manifold = current_manifold_range[found_index];
                current_manifold = Physics::MergeManifolds(current_manifold, previous_collision_events.manifolds[*i]);
            }
        }
    }
}


void Physics::MergeEventManifolds(
    CollisionEvents & current_collision_events,
    CollisionEventOffsets const current_offsets,
    CollisionEvents const & previous_collision_events,
    CollisionEventOffsets const previous_offsets)
{
    MergeManifoldsRange(
        previous_collision_events,
        {previous_offsets.kinematic_static, previous_offsets.kinematic_kinematic},
        current_collision_events,
        {current_offsets.kinematic_static, current_offsets.kinematic_kinematic});
    MergeManifoldsRange(
        previous_collision_events,
        {previous_offsets.kinematic_kinematic, previous_offsets.rigid_kinematic},
        current_collision_events,
        {current_offsets.kinematic_kinematic, current_offsets.rigid_kinematic});
    MergeManifoldsRange(
        previous_collision_events,
        {previous_offsets.rigid_kinematic, previous_offsets.rigid_static},
        current_collision_events,
        {current_offsets.rigid_kinematic, current_offsets.rigid_static});
    MergeManifoldsRange(
        previous_collision_events,
        {previous_offsets.rigid_static, previous_offsets.rigid_rigid},
        current_collision_events,
        {current_offsets.rigid_static, current_offsets.rigid_rigid});
    MergeManifoldsRange(
        previous_collision_events,
        {previous_offsets.rigid_rigid, uint32_t(Size(previous_collision_events.bodies))},
        current_collision_events,
        {current_offsets.rigid_rigid, uint32_t(Size(current_collision_events.bodies))});
}
