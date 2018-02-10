#include "Solve.h"

#include "Constraints.h"

#include "MovementToEffectiveMovement.h"

#include <Math\MathFunctions.h>
#include <Math\SSELoadStore.h>
#include <Math\SSEFloatFunctions.h>
#include <Math\VectorAlgorithms.h>

#include <ppl.h>

using namespace Physics;

void Physics::Solve(uint32_t max_iterations, Range<Constraint *> constraints, Range<SingleBodyConstraint *> single_constraints, Range<Movement *> movements)
{
    auto count = Size(constraints);
    auto single_count = Size(single_constraints);
    for( auto iteration = 0u; iteration < max_iterations; ++iteration )
    {
        for( auto i = 0u; i < count; ++i )
        {
            auto c = constraints[i];
            auto impulse =
                CalculateEffectiveMovement(movements[c.body_indices[0]], c.movement_to_effective_movement[0]) +
                CalculateEffectiveMovement(movements[c.body_indices[1]], c.movement_to_effective_movement[1]);
            auto impulse_error = c.target_impulse - impulse - c.feedback_coefficient * c.total_impulse;
            auto total_impulse = c.total_impulse + impulse_error;
            total_impulse = Math::Clamp(c.impulse_limits.min, c.impulse_limits.max, total_impulse);
            impulse_error = total_impulse - c.total_impulse;
            constraints[i].total_impulse = total_impulse;
            movements[c.body_indices[0]] += impulse_error * c.unit_impulses[0];
            movements[c.body_indices[1]] += impulse_error * c.unit_impulses[1];
        }

        for( auto i = 0u; i < single_count; ++i )
        {
            auto c = single_constraints[i];
            auto impulse = CalculateEffectiveMovement(movements[c.body_index], c.movement_to_effective_movement);
            auto impulse_error = c.target_impulse - impulse - c.feedback_coefficient * c.total_impulse;
            auto total_impulse = c.total_impulse + impulse_error;
            total_impulse = Math::Clamp(c.impulse_limits.min, c.impulse_limits.max, total_impulse);
            impulse_error = total_impulse - c.total_impulse;
            single_constraints[i].total_impulse = total_impulse;
            movements[c.body_index] += impulse_error * c.unit_impulse;
        }
    }
}


void Physics::Solve(uint32_t max_iterations, MinMax<float> relaxation_bounds, Range<Constraint *> constraints, Range<SingleBodyConstraint *> single_constraints, Range<Movement *> movements)
{
    auto count = Size(constraints);
    auto single_count = Size(single_constraints);
    for( auto iteration = 0u; iteration < max_iterations; ++iteration )
    {
        auto relaxation = Math::Lerp( relaxation_bounds.max, relaxation_bounds.min, float( iteration ) / max_iterations );
        for( auto i = 0u; i < count; ++i )
        {
            auto c = constraints[i];
            auto impulse =
                CalculateEffectiveMovement(movements[c.body_indices[0]], c.movement_to_effective_movement[0]) +
                CalculateEffectiveMovement(movements[c.body_indices[1]], c.movement_to_effective_movement[1]);
            auto impulse_error = c.target_impulse - impulse - c.feedback_coefficient * c.total_impulse;
            impulse_error *= relaxation;
            auto total_impulse = c.total_impulse + impulse_error;
            total_impulse = Math::Clamp(c.impulse_limits.min, c.impulse_limits.max, total_impulse);
            impulse_error = total_impulse - c.total_impulse;
            constraints[i].total_impulse = total_impulse;
            movements[c.body_indices[0]] += impulse_error * c.unit_impulses[0];
            movements[c.body_indices[1]] += impulse_error * c.unit_impulses[1];
        }

        for( auto i = 0u; i < single_count; ++i )
        {
            auto c = single_constraints[i];
            auto impulse = CalculateEffectiveMovement(movements[c.body_index], c.movement_to_effective_movement);
            auto impulse_error = c.target_impulse - impulse - c.feedback_coefficient * c.total_impulse;
            impulse_error *= relaxation;
            auto total_impulse = c.total_impulse + impulse_error;
            total_impulse = Math::Clamp(c.impulse_limits.min, c.impulse_limits.max, total_impulse);
            impulse_error = total_impulse - c.total_impulse;
            single_constraints[i].total_impulse = total_impulse;
            movements[c.body_index] += impulse_error * c.unit_impulse;
        }
    }
}


void Physics::SolveSSE(uint32_t max_iterations, MinMax<float> relaxation_bounds, Range<Constraint *> constraints, Range<SingleBodyConstraint *> single_constraints, Range<Movement *> movements)
{
    using namespace Math::SSE;
    auto count = Size(constraints);
    auto single_count = Size(single_constraints);
    for( auto iteration = 0u; iteration < max_iterations; ++iteration )
    {
        auto relaxation = Math::Lerp( relaxation_bounds.max, relaxation_bounds.min, float( iteration ) / max_iterations );
        for( auto i = 0u; i < count; ++i )
        {
            auto & c = constraints[i];
            Float32Vector both_movements[3];
            auto movement0_begin = begin((begin(movements) + c.body_indices[0])->momentum);
            auto movement1_begin = begin((begin(movements) + c.body_indices[1])->momentum);
            both_movements[0] = Load(movement0_begin);
            both_movements[1] = Load2Upper(movement1_begin, Load2(movement0_begin + 4));
            both_movements[2] = Load(movement1_begin + 2);
            auto mte_begin = begin(c.movement_to_effective_movement[0].linear);
            float impulse;
            {
                auto m0 = Multiply(both_movements[0], Load(mte_begin + 0));
                auto m1 = Multiply(both_movements[1], Load(mte_begin + 4));
                auto m2 = Multiply(both_movements[2], Load(mte_begin + 8));
                auto m = Add(Add(m0, m1), m2);
                auto h = HorizontalAdd(m, m);
                impulse = GetSingle(HorizontalAdd(h, h));
            }
            auto impulse_error = c.target_impulse - impulse - c.feedback_coefficient * c.total_impulse;
            impulse_error *= relaxation;
            auto total_impulse = c.total_impulse + impulse_error;
            total_impulse = Math::Clamp(c.impulse_limits.min, c.impulse_limits.max, total_impulse);
            impulse_error = total_impulse - c.total_impulse;
            c.total_impulse = total_impulse;
            {
                auto impulse_error4 = SetAll(impulse_error);
                auto impulse_begin = begin(c.unit_impulses[0].momentum);
                both_movements[0] = Add(both_movements[0], Multiply(impulse_error4, Load(impulse_begin + 0)));
                both_movements[1] = Add(both_movements[1], Multiply(impulse_error4, Load(impulse_begin + 4)));
                both_movements[2] = Add(both_movements[2], Multiply(impulse_error4, Load(impulse_begin + 8)));
            }
            Store(both_movements[0], movement0_begin);
            Store2(both_movements[1], movement0_begin + 4);
            Store2Upper(both_movements[1], movement1_begin);
            Store(both_movements[2], movement1_begin + 2);
        }

        for( auto i = 0u; i < single_count; ++i )
        {
            auto & c = single_constraints[i];
            auto movement_begin = begin(movements[c.body_index].momentum);
            Float32Vector movement[2];
            movement[0] = Load(movement_begin);
            movement[1] = Load2(movement_begin + 4);
            auto mte_begin = begin(c.movement_to_effective_movement.linear);
            float impulse;
            {
                auto m0 = Multiply(movement[0], Load(mte_begin + 0));
                auto m1 = Multiply(movement[1], Load2(mte_begin + 4));
                auto m = Add(m0, m1);
                auto h = HorizontalAdd(m, m);
                impulse = GetSingle(HorizontalAdd(h, h));
            }
            auto impulse_error = c.target_impulse - impulse - c.feedback_coefficient * c.total_impulse;
            impulse_error *= relaxation;
            auto total_impulse = c.total_impulse + impulse_error;
            total_impulse = Math::Clamp(c.impulse_limits.min, c.impulse_limits.max, total_impulse);
            impulse_error = total_impulse - c.total_impulse;
            c.total_impulse = total_impulse;
            auto impulse_begin = begin(c.unit_impulse.momentum);
            auto all_impulse_error = SetAll(impulse_error);
            movement[0] = Add(movement[0], Multiply(all_impulse_error, Load(impulse_begin + 0)));
            movement[1] = Add(movement[1], Multiply(all_impulse_error, Load2(impulse_begin + 4)));
            Store(movement[0], movement_begin);
            Store2(movement[1], movement_begin + 4);
        }
    }
}


// Checks for the total impulse error and stops early if it is small enough
// This seem to happen almost never.
void Physics::Solve(uint32_t max_iterations, MinMax<float> relaxation_bounds, float stopping_error_tolerance, Range<Constraint *> constraints, Range<SingleBodyConstraint *> single_constraints, Range<Movement *> movements)
{
    auto count = Size(constraints);
    auto single_count = Size(single_constraints);
    stopping_error_tolerance *= stopping_error_tolerance * (count + single_count);
    auto total_error = 0.f;
    auto iteration = 0u;
    while( iteration < max_iterations && total_error > stopping_error_tolerance )
    {
        auto relaxation = Math::Lerp( relaxation_bounds.max, relaxation_bounds.min, float( iteration ) / max_iterations );
        total_error = 0;
        for( auto i = 0u; i < count; ++i )
        {
            auto c = constraints[i];
            auto impulse =
                CalculateEffectiveMovement(movements[c.body_indices[0]], c.movement_to_effective_movement[0]) +
                CalculateEffectiveMovement(movements[c.body_indices[1]], c.movement_to_effective_movement[1]);
            auto impulse_error = c.target_impulse - impulse - c.feedback_coefficient * c.total_impulse;
            impulse_error *= relaxation;
            auto total_impulse = c.total_impulse + impulse_error;
            total_impulse = Math::Clamp(c.impulse_limits.min, c.impulse_limits.max, total_impulse);
            impulse_error = total_impulse - c.total_impulse;
            constraints[i].total_impulse = total_impulse;
            movements[c.body_indices[0]] += impulse_error * c.unit_impulses[0];
            movements[c.body_indices[1]] += impulse_error * c.unit_impulses[1];
            total_error += impulse_error * impulse_error;
        }

        for( auto i = 0u; i < single_count; ++i )
        {
            auto c = single_constraints[i];
            auto impulse = CalculateEffectiveMovement(movements[c.body_index], c.movement_to_effective_movement);
            auto impulse_error = c.target_impulse - impulse - c.feedback_coefficient * c.total_impulse;
            impulse_error *= relaxation;
            auto total_impulse = c.total_impulse + impulse_error;
            total_impulse = Math::Clamp(c.impulse_limits.min, c.impulse_limits.max, total_impulse);
            impulse_error = total_impulse - c.total_impulse;
            single_constraints[i].total_impulse = total_impulse;
            movements[c.body_index] += impulse_error * c.unit_impulse;
            total_error += impulse_error * impulse_error;
        }
        ++iteration;
    }
}




void Physics::SolveIslands(
    uint32_t max_iterations,
    MinMax<float> relaxation_bounds,
    Range<uint32_t const *> island_offsets,
    Range<uint32_t const *> single_body_island_offsets,
    Range<Constraint *> constraints,
    Range<SingleBodyConstraint *> single_body_constraints,
    Range<Movement *> movements)
{
    assert(Size(island_offsets) == Size(single_body_island_offsets));
    auto island_count = Size(island_offsets) - 1;
    Concurrency::parallel_for( 0u, uint32_t(island_count),
        [=](uint32_t i)
    {
        auto begin_offset = island_offsets[i];
        auto end_offset = island_offsets[i + 1];
        auto begin_single_body_offset = single_body_island_offsets[i];
        auto end_single_body_offset = single_body_island_offsets[i + 1];
        SolveSSE(
            max_iterations,
            relaxation_bounds,
            CreateRange(constraints, begin_offset, end_offset),
            CreateRange(single_body_constraints, begin_single_body_offset, end_single_body_offset),
            movements
            // island_movements
            );
    }
    );
}
