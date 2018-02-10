#pragma once

#include <vector>

namespace Math
{
    struct SparseMatrix;
    void swap( SparseMatrix & a, SparseMatrix & b );
    SparseMatrix MakeSize( SparseMatrix m, unsigned rows, unsigned columns, unsigned elements );
    SparseMatrix CreateSparseMatrix( unsigned rows, unsigned columns, unsigned elements );
    unsigned GetNumberOfRows( SparseMatrix const & m );
    unsigned GetNumberOfElements( SparseMatrix const & m );
    unsigned GetNumberOfColumns( SparseMatrix const & m );

    struct SparseMatrix
    {
        std::vector<float> values;
        std::vector<unsigned> column_indices;
        // indices to the column indices and values, start and end
        std::vector<unsigned> row_offsets = { { { 0 } } };
        // number of columns
        unsigned column_count = 0;
    };


    inline void swap( SparseMatrix & a, SparseMatrix & b )
    {
        using std::swap;
        swap( a.values, b.values );
        swap( a.column_indices, b.column_indices );
        swap( a.row_offsets, b.row_offsets );
        swap( a.column_count, b.column_count );
    }

    inline SparseMatrix MakeSize( SparseMatrix m, unsigned rows, unsigned columns, unsigned elements )
    {
        m.values.clear();
        m.values.resize( elements );
        m.column_indices.clear();
        m.column_indices.resize( elements );
        m.row_offsets.clear();
        m.row_offsets.resize( rows + 1 );
        m.column_count = columns;
        return m;
    }


    inline SparseMatrix CreateSparseMatrix( unsigned rows, unsigned columns, unsigned elements )
    {
        return MakeSize( {}, rows, columns, elements );
    }


    inline unsigned GetNumberOfRows( SparseMatrix const & m )
    {
        return unsigned(m.row_offsets.size() - 1);
    }


    inline unsigned GetNumberOfElements( SparseMatrix const & m )
    {
        return m.row_offsets.back();
    }


    inline unsigned GetNumberOfColumns( SparseMatrix const & m )
    {
        return m.column_count;
    }
}
