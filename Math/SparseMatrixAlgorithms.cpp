#include "SparseMatrix.h"
#include "SparseMatrixAlgorithms.h"

#include "VectorAlgorithms.h"
#include "MathFunctions.h"

#include <Utilities/Memory.h>
#include <Utilities/Range.h>
#include <Utilities/IntegerRange.h>
#include <Utilities\StdVectorFunctions.h>

#include <algorithm>
#include <cassert>

#include <string>

using namespace Math;

void Math::RemoveZeroEntries(SparseMatrix & m)
{
    auto count = Size(m.row_offsets) - 1;
    auto offset = 0u;
    for( auto i = 0u; i < count; ++i )
    {
        auto start = m.row_offsets[i];
        m.row_offsets[i] = start - offset;
        for( auto r : CreateIntegerRange(start, m.row_offsets[i + 1]) )
        {
            if( m.values[r] == 0 )
            {
                offset += 1;
            }
            else
            {
                m.values[r - offset] = m.values[r];
                m.column_indices[r - offset] = m.column_indices[r];
            }
        }
    }
    auto value_count = Last(m.row_offsets) - offset;
    Last(m.row_offsets) = value_count;
    m.values.resize(value_count);
    m.column_indices.resize(value_count);
}


void Math::Transpose( SparseMatrix const& in, SparseMatrix& out )
{
    assert( &in.values != &out.values && &in.row_offsets != &out.row_offsets && &in.column_indices != &out.column_indices);
    //assert( GetNumberOfElements( in ) <= out.values.capacity() );
    auto element_count = GetNumberOfElements( in );
    ResetSize( out.values, element_count );
    ResetSize( out.column_indices, element_count );
    auto row_count = GetNumberOfRows( in );
    out.column_count = row_count;
    auto column_count = GetNumberOfColumns( in );
    ResetSize( out.row_offsets, column_count + 1 );
    Zero( out.row_offsets.data(), out.row_offsets.size() );

    // count the number of elements in columns and store them in the output row indices
    auto column_counts = begin(out.row_offsets) + 1;
    for( auto column : CreateRange(begin(in.column_indices), begin(in.column_indices) + element_count) )
    {
        ++column_counts[column];
    }

    // set the row offsets to point at the first element of the all elements for each row
    // note: we also displace all offsets, such that they are all at their final position
    {
        unsigned total_offset = 0;
        for( auto& offset : CreateRange(column_counts, end(out.row_offsets) ) )
        {
            auto t = offset;
            offset = total_offset;
            total_offset += t;
        }
    }

    // copy the values and assign the right column indices, meanwhile keeping track of where to add the new elements
    // in the row offsets
    for( auto old_row = 0u; old_row < row_count; ++old_row )
    {
        for( auto i : CreateIntegerRange( in.row_offsets[old_row], in.row_offsets[old_row + 1] ) )
        {
            auto column = in.column_indices[i];
            auto & current_column_count = column_counts[column];
            auto destination_index = current_column_count;
            current_column_count += 1;
            out.column_indices[destination_index] = old_row;
            out.values[destination_index] = in.values[i];
        }
    }
}


// new row count and column count are stored in b
void Math::CopySubMatrix( SparseMatrix const & a, SparseMatrix& b )
{
    CopySubMatrix(a, 0, 0, b);
}


void Math::CopySubMatrix( SparseMatrix const & a, uint32_t row_offset, uint32_t column_offset, SparseMatrix& b )
{
    assert( GetNumberOfRows( a ) >= GetNumberOfRows( b ) + row_offset );
    assert( GetNumberOfColumns( a ) >= GetNumberOfColumns( b ) + column_offset );

    b.column_indices.clear();
    b.values.clear();

    auto column_count = GetNumberOfColumns( b );
    //auto max_column_index = GetNumberOfColumns( b ) - 1;
    uint32_t offset = 0;
    auto row_count_b = GetNumberOfRows(b);
    for( auto row_b = 0u; row_b < row_count_b; ++row_b )
    {
        b.row_offsets[row_b] = offset;
        auto row_a = row_b + row_offset;
        for( auto index_a : CreateIntegerRange( a.row_offsets[row_a], a.row_offsets[row_a + 1] ) )
        {
            auto column_a = a.column_indices[index_a];
            uint32_t column_b = column_a - column_offset;
            if( column_b < column_count )
            {
                b.column_indices.push_back( column_b );
                b.values.push_back( a.values[index_a] );
                ++offset;
            }
        }
        // for sorted matrices
        //b.row_offsets[row] = offset;
        //auto index_a = a.row_offsets[row];
        //auto column_indices_begin = begin( a.column_indices ) + index_a;
        //auto column_indices_end = begin( a.column_indices ) + a.row_offsets[row + 1];
        //column_indices_end = std::upper_bound( column_indices_begin, column_indices_end, max_column_index );

        //std::copy( column_indices_begin, column_indices_end, std::back_inserter( b.column_indices ) );
        //auto element_count = uint32_t(column_indices_end - column_indices_begin);
        //offset += element_count;
        //std::copy_n( begin( a.values ) + index_a, element_count, std::back_inserter( b.values ) );
    }
    Last(b.row_offsets) = offset;
}


void Math::CopySortedSubMatrix( SparseMatrix const & a, uint32_t row_offset, uint32_t column_offset, SparseMatrix& b )
{
    assert( GetNumberOfRows( a ) >= GetNumberOfRows( b ) + row_offset );
    assert( GetNumberOfColumns( a ) >= GetNumberOfColumns( b ) + column_offset );

    b.column_indices.clear();
    b.values.clear();

    auto column_count = GetNumberOfColumns(b);
    uint32_t offset = 0;
    auto row_count_b = GetNumberOfRows(b);
    for( auto row_b = 0u; row_b < row_count_b; ++row_b )
    {
        b.row_offsets[row_b] = offset;
        auto row_a = row_b + row_offset;
        auto start_offset_a = a.row_offsets[row_a];
        auto end_offset_a = a.row_offsets[row_a + 1];
        auto columns_start = begin(a.column_indices) + start_offset_a;
        auto columns_end = begin(a.column_indices) + end_offset_a;
        auto columns_start_range = std::lower_bound( columns_start, columns_end, column_offset );
        auto columns_end_range = std::lower_bound( columns_start_range, columns_end, column_count + column_offset);
        auto element_count = uint32_t(columns_end_range - columns_start_range);
        for( auto i = 0u; i < element_count; ++i )
        {
            b.column_indices.push_back(columns_start_range[i] - column_offset);
        }
        auto values_start_range = begin(a.values) + (columns_start_range - begin(a.column_indices));
        b.values.insert(end(b.values), values_start_range, values_start_range + element_count);
        offset += element_count;
    }
    Last(b.row_offsets) = offset;
}


void Math::Multiply( SparseMatrix const & a, SparseMatrix const & b, SparseMatrix & c, Range<float * __restrict> const workspace_range, Range<uint32_t *  __restrict> const used_range )
{
    // The multiplication algorithm doesn't produce a sorted matrix.

    assert( GetNumberOfColumns( a ) == GetNumberOfRows( b ) );
    assert( Size( workspace_range ) >= GetNumberOfColumns( b ) );
    assert( Size( used_range ) >= GetNumberOfColumns( b ) );

    //assert( GetNumberOfRows(a) == GetNumberOfRows(c) );
    ResetSize( c.row_offsets, Size( a.row_offsets ) );
    auto column_count_b = GetNumberOfColumns(b);
    c.column_count = column_count_b;
    c.column_indices.clear();
    c.values.clear();

    float maximum_elements = float(GetNumberOfRows(c)) * float(GetNumberOfColumns(c));
    // reserve the amount for our first guess
    c.column_indices.reserve( GetNumberOfElements( a ) + GetNumberOfElements( b ) );
    // set our first estimate to whatever we have room for
    auto sparsity_estimate = float(c.column_indices.capacity()) / maximum_elements;
    Zero( begin( used_range ), column_count_b );
    auto * __restrict workspace = begin( workspace_range );
    auto * __restrict used = begin( used_range );

    auto row_count_a = GetNumberOfRows(a);
    for (auto row_a : CreateIntegerRange( row_count_a ) )
    {
        auto row_c = row_a;
        c.row_offsets[row_c] = uint32_t(Size(c.column_indices));

        auto const used_mark = row_a + 1;

        auto index_a_end = a.row_offsets[row_a + 1];
        for( auto index_a = a.row_offsets[row_a]; index_a < index_a_end; ++index_a )
        {
            auto column_a = a.column_indices[index_a];
            auto value_a = a.values[index_a];

            auto row_b = column_a;
            // scatter
            auto index_b_end = b.row_offsets[row_b + 1];
            for( auto index_b = b.row_offsets[row_b]; index_b < index_b_end; ++index_b )
            {
                auto column_b = b.column_indices[index_b];
                auto value_b = b.values[index_b];
                if(used[column_b] < used_mark)
                {
                    used[column_b] = used_mark;
                    c.column_indices.push_back(column_b);
                    workspace[column_b] = value_b * value_a;
                }
                else
                {
                    workspace[column_b] += value_b * value_a;
                }
            }
        }

        c.values.reserve( c.column_indices.capacity() );

        for (auto column_c : CreateRange(c.column_indices, c.row_offsets[row_c], Size(c.column_indices)))
        {
            auto value_ab = workspace[column_c];
            c.values.push_back(value_ab);
        }

        auto new_sparsity_estimate = float(Size(c.values)) / (float(GetNumberOfColumns(c)) * float(row_a + 1));
        if(new_sparsity_estimate > sparsity_estimate)
        {
            // make room for at least something times as many elements
            sparsity_estimate = Math::Max( new_sparsity_estimate, 1.3f * sparsity_estimate );
            // add something to our new capacity such that we can always fill the next column
            auto reserve_size = size_t(Math::Ceil(sparsity_estimate * maximum_elements)) + GetNumberOfColumns(c);
            c.column_indices.reserve( reserve_size );
        }
    }

    Last(c.row_offsets) = uint32_t(Size(c.column_indices));
}


void Math::Multiply( SparseMatrix const & a, SparseMatrix const & b, SparseMatrix& c )
{
    auto columns_b = GetNumberOfColumns( b );
    std::vector<float> workspace( columns_b );
    std::vector<uint32_t> used( columns_b );
    Multiply( a, b, c, workspace, used );
}



void Math::MultiplyAdd( SparseMatrix const & m, Range<float const * __restrict> const x, float const x_scale, Range<float * __restrict> const y, float const y_scale )
{
    assert( GetNumberOfColumns( m ) == Size( x ) );
    assert( GetNumberOfRows( m ) == Size( y ) );

    for( auto row : CreateIntegerRange( GetNumberOfRows( m ) ) )
    {
        float value = 0;
        for( auto index : CreateIntegerRange( m.row_offsets[row], m.row_offsets[row + 1] ) )
        {
            auto column = m.column_indices[index];
            auto value_m = m.values[index];
            auto value_x = x[column];
            value += value_m * value_x;
        }
        value *= x_scale;
        value += y[row] * y_scale;
        y[row] = value;
    }
}


void Math::SymbolicMultiplyOr( SparseMatrix const & m, Range<bool const* __restrict > x, Range<bool * __restrict > y)
{
    assert( GetNumberOfColumns( m ) == Size( x ) );
    assert( GetNumberOfRows( m ) == Size( y ) );

    for( auto row : CreateIntegerRange( GetNumberOfRows( m ) ) )
    {
        if(!y[row])
        {
            auto index = m.row_offsets[row];
            auto stop_index = m.row_offsets[row + 1];
            while( index < stop_index && !x[m.column_indices[index]] )
            {
                ++index;
            }
            if(index != stop_index)
            {
                y[row] = true;
            }
        }
    }
}


void Math::MultiplyAddTransposed( SparseMatrix const & m, Range<float const* __restrict> const x, float const x_scale, Range<float * __restrict> const y, float const y_scale )
{
    assert( GetNumberOfRows( m ) == Size( x ) );
    assert( GetNumberOfColumns( m ) == Size( y ) );

    Multiply( y_scale, y );

    for( auto row : CreateIntegerRange( GetNumberOfRows( m ) ) )
    {
        auto value_x = x[row] * x_scale;
        for( auto index : CreateIntegerRange( m.row_offsets[row], m.row_offsets[row + 1] ) )
        {
            auto column = m.column_indices[index];
            auto value_m = m.values[index];
            auto value_mx = value_m * value_x;
            y[column] += value_mx;
        }
    }
}


void Math::SymbolicMultiplyOrTransposed( SparseMatrix const & m, Range<bool const * __restrict > x, Range<bool * __restrict > y )
{
    assert( GetNumberOfRows( m ) == Size( x ) );
    assert( GetNumberOfColumns( m ) == Size( y ) );

    for( auto row : CreateIntegerRange( GetNumberOfRows( m ) ) )
    {
        if(x[row])
        {
            for( auto index : CreateIntegerRange( m.row_offsets[row], m.row_offsets[row + 1] ) )
            {
                auto column = m.column_indices[index];
                y[column] = true;
            }
        }
    }
}


void Math::InverseReorderColumns( Range<uint32_t const * __restrict> reordering, SparseMatrix & a )
{
    assert(Size(reordering) == GetNumberOfColumns(a));

    auto size = Size(a.column_indices);
    auto * __restrict column_indices = a.column_indices.data();
    for( auto i = 0u; i < size; ++i )
    {
        column_indices[i] = reordering[column_indices[i]];
    }
}


void Math::InverseReorderSymmetric(Range<uint32_t const * __restrict> reordering, SparseMatrix & A, SparseMatrix & temp)
{
    assert(GetNumberOfRows(A) == GetNumberOfColumns(A));
    InverseReorderColumns(reordering, A);
    swap( A, temp );
    Transpose(temp, A);
    InverseReorderColumns(reordering, A);
}


void Math::InverseReorderSquare( Range<uint32_t const * __restrict> reordering, SparseMatrix & A, SparseMatrix & temp )
{
    InverseReorderSymmetric( reordering, A, temp );
    swap( A, temp );
    Transpose( temp, A );
}


float Math::DensityPercentage( SparseMatrix & m )
{
    return float( GetNumberOfElements( m ) ) / float( GetNumberOfColumns( m ) * GetNumberOfRows( m ) );
}


float Math::SparistyPercentage( SparseMatrix & m )
{
    return 1 - DensityPercentage( m );
}


bool Math::operator==( SparseMatrix const & a, SparseMatrix const & b )
{
    return a.column_count == b.column_count &&
        a.column_indices == b.column_indices &&
        a.row_offsets == b.row_offsets &&
        a.values == b.values;
}