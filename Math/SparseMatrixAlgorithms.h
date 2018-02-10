#pragma once

#include <Utilities\Range.h>
#include <cstdint>

namespace Math
{
    struct SparseMatrix;

    // removes the entries with zero value from the matrix
    void RemoveZeroEntries(SparseMatrix & m);

    // transposes in to out
    // Out will be sorted
    void Transpose( SparseMatrix const & in, SparseMatrix & out );

    // new row count and column count are stored in b
    void CopySubMatrix( SparseMatrix const & a, SparseMatrix& b );
    // new row count and column count are stored in b
    void CopySubMatrix( SparseMatrix const & a, uint32_t row_offset, uint32_t column_offset, SparseMatrix& b );
    // new row count and column count are stored in b
    // Assumes 'a' is sorted
    void CopySortedSubMatrix( SparseMatrix const & a, uint32_t row_offset, uint32_t column_offset, SparseMatrix& b );

    // Performs c = a * b
    // Allocates workspace memory by itself
    void Multiply( SparseMatrix const & a, SparseMatrix const & b, SparseMatrix& c );

    // Performs c = a * b, using the provided workspace
    void Multiply( SparseMatrix const & a, SparseMatrix const & b, SparseMatrix & c, Range<float * __restrict> const workspace, Range<uint32_t *  __restrict> const workspace2 );

    // Performs y = A * x * x_scale + y * y_scale
    void MultiplyAdd( SparseMatrix const & A, Range<float const* __restrict > x, float x_scale, Range<float * __restrict > y, float y_scale );

    // Performs y = A' * x * x_scale + y * y_scale
    void MultiplyAddTransposed( SparseMatrix const & A, Range<float const * __restrict > x, float x_scale, Range<float * __restrict > y, float y_scale );

    // Calculates y = A * x OR y
    void SymbolicMultiplyOr( SparseMatrix const & A, Range<bool const* __restrict > x, Range<bool * __restrict > y);

    // Performs y = A' * x OR y
    void SymbolicMultiplyOrTransposed( SparseMatrix const & A, Range<bool const * __restrict > x, Range<bool * __restrict > y );

    // Reorders the columns,
    // Produces an unsorted matrix
    void InverseReorderColumns( Range<uint32_t const * __restrict> reordering, SparseMatrix & a );

    // Reorders both columns and rows according to 'reordering',
    // Assumes 'a' is a square matrix
    // Produces a sorted matrix
    void InverseReorderSquare(Range<uint32_t const * __restrict> reordering, SparseMatrix & a, SparseMatrix & temp);

    // Reorders both columns and rows according to 'reordering',
    // Assumes 'a' is a symmetric matrix,
    // Produce an unsorted matrix
    void InverseReorderSymmetric(Range<uint32_t const * __restrict> reordering, SparseMatrix & a, SparseMatrix & temp );

    float SparistyPercentage( SparseMatrix & m );
    float DensityPercentage( SparseMatrix & m );

    // test for equality
    bool operator==( SparseMatrix const & a, SparseMatrix const & b );
}