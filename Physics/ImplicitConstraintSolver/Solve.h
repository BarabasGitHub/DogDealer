#pragma once

#include <Utilities\MinMax.h>
#include <Utilities\Range.h>

#include <cstdint>

namespace Physics
{
    struct Constraint;
    struct SingleBodyConstraint;
    struct Movement;

    void Solve(
        uint32_t max_iterations,
        Range<Constraint *> constraints,
        Range<SingleBodyConstraint *> single_constraints,
        Range<Movement *> movements);

    void Solve(
        uint32_t max_iterations,
        MinMax<float> relaxation_bounds,
        Range<Constraint *> constraints,
        Range<SingleBodyConstraint *> single_constraints,
        Range<Movement *> movements);

    void SolveSSE(
        uint32_t max_iterations,
        MinMax<float> relaxation_bounds,
        Range<Constraint *> constraints,
        Range<SingleBodyConstraint *> single_constraints,
        Range<Movement *> movements);

    void Solve(
        uint32_t max_iterations,
        MinMax<float> relaxation_bounds,
        float stopping_error_tolerance,
        Range<Constraint *> constraints,
        Range<SingleBodyConstraint *> single_constraints,
        Range<Movement *> movements);

    void SolveIslands(
        uint32_t max_iterations,
        MinMax<float> relaxation_bounds,
        Range<uint32_t const *> island_offsets,
        Range<uint32_t const *> single_body_island_offsets,
        Range<Constraint *> constraints,
        Range<SingleBodyConstraint *> single_body_constraints,
        Range<Movement *> movements);
}