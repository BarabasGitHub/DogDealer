#pragma once

#include "InvalidIndex.h"
#include "StdVectorFunctions.h"

#include <vector>
#include <iterator>
#include <cstdint>


template<typename DataType0> void ResizeMultipleVectors(size_t size, std::vector<DataType0> & vector0 )
{
    vector0.resize(size);
}

template<typename DataType0, typename ... DataTypes > void ResizeMultipleVectors(size_t size, std::vector<DataType0> & vector0, std::vector<DataTypes> & ... other_vectors)
{
    ResizeMultipleVectors(size, vector0);
    ResizeMultipleVectors(size, other_vectors...);
}


// ensures the vector has an element with index element_index and returns a reference to this element.
// if the vector does not have this element it will grow filling all new elements with fill_value
template<typename DataType> DataType& EnsureElementExists(size_t element_index, DataType fill_value, std::vector<DataType> & vector)
{
    if(Size(vector) <= element_index)
    {
        vector.resize(element_index + 1, fill_value);
    }
    return vector[element_index];
}


// Remove a vector entry by overwriting it with the last element
// and shortening the vector by one
template<typename T> void SwapAndPrune(size_t const pruned_index, std::vector<T> &data)
{
    data[pruned_index] = std::move(data.back());
    data.pop_back();
}


// Remove an entry from a bunch of vectors by overwriting it with the last element and shortening the vector by one
template<typename T, typename... Ts> void SwapAndPrune(size_t const pruned_index, std::vector<T> &data, std::vector<Ts> &... rest)
{
	SwapAndPrune(pruned_index, data);
    SwapAndPrune(pruned_index, rest...);
}



template<typename DataType, typename IndexType>
DataType const & GetOptional( Range<DataType const *> const & data, IndexType index, DataType const & default_value )
{
    return Size( data ) > index ? data[index] : default_value;
}


template<typename DataType, typename IndexType>
DataType const & GetOptional(std::vector<DataType> const & data, IndexType index, DataType const & default_value)
{
    return GetOptional(CreateRange(data), index, default_value);
}

// special version for uint32_t because it is so often used to get indices which should return c_invalid_index if they don't exist
template<typename IndexType>
uint32_t GetOptional( Range<uint32_t const *> const & data, IndexType index )
{
    return GetOptional( data, index, c_invalid_index );
}

// reorder according to the indices
template<typename DataType>
void Reorder( Range<DataType const * __restrict> data, Range<uint32_t const * __restrict> indices, Range<DataType * __restrict> output )
{
    assert( Size(data) >= Size(indices) );
    assert( Size(output) >= Size(indices) );
    auto size = Size( indices );
    for( auto i = 0u; i < size; ++i )
    {
        output[i] = data[indices[i]];
    }
}


// reorder according to the indices
template<typename DataType>
void InverseReorder( Range<DataType const * __restrict> data, Range<uint32_t const * __restrict> indices, Range<DataType * __restrict> output )
{
    assert( Size(data) >= Size(indices) );
    assert( Size(output) >= Size(indices) );
    auto size = Size( indices );
    for( auto i = 0u; i < size; ++i )
    {
        output[indices[i]] = data[i];
    }
}

/// reorder according to the indices
template<typename DataType>
std::vector<DataType> Reorder( Range<DataType const *> data, Range<uint32_t const *> indices )
{
    assert( Size(data) >= Size(indices) );
    auto size = Size(indices);
    std::vector<DataType> new_container( size );
    Reorder<DataType>(data, indices, new_container);
    return new_container;
}

/// reorder according to the indices
template<typename DataType>
std::vector<DataType> Reorder( std::vector<DataType> const & data, Range<uint32_t const *> indices )
{
    return Reorder( CreateRange(data), indices );
}


void InvertReordering(Range<uint32_t const * __restrict> original, Range<uint32_t * __restrict> inverted);


// copies the elements in data pointed to by indices to selected
// data and selected can't overlap
template<typename DataType>
void Select( Range<DataType const * __restrict> data, Range<uint32_t const *> indices, Range<DataType * __restrict> selected)
{
    assert(Size(indices) == Size(selected));
    for( auto i = 0u; i < Size(indices); ++i )
    {
        auto index = indices[i];
        selected[i] = data[index];
    }
}


namespace Details
{
    template<typename DataType>
    struct Mover
    {
        Mover( typename std::vector<DataType>::iterator destination, typename std::vector<DataType>::iterator source):
            destination(destination), source(source)
        { }

        typename std::vector<DataType>::iterator destination, source;

        void operator()( typename std::vector<DataType>::const_iterator last )
        {
            while( source < last )
            {
                *destination = std::move( *source );
                ++destination;
                ++source;
            }
        }
    };
}

// Removes the data elements given by the indices. Assumes sorted indices!
template<typename DataType> void RemoveEntries( std::vector<DataType> & data, Range<uint32_t const *> indices )
{
    if( Size( indices ) == 0 ) return;
    auto destination = begin( data ) + indices[0];
    auto source = destination + 1;
    ++indices.start;

    auto move_till = Details::Mover<DataType>( destination, source );

    for( auto i : indices )
    {
        auto next = begin( data ) + i;
        move_till( next );
        ++move_till.source;
    }
    move_till( end( data ) );

    data.erase( move_till.destination, end(data) );
}


template<typename DataType> void RemoveEntries( std::vector<DataType> & data, std::vector<uint32_t>& offsets, Range<uint32_t const *> indices )
{
    if( Size( indices ) == 0 ) return;
    auto data_destination = begin( data ) + offsets[indices[0]];
    auto data_source = begin( data ) + offsets[indices[0] + 1];
    ++indices.start;

    auto move_data_till = Details::Mover<DataType>( data_destination, data_source );

    for( auto i : indices )
    {
        auto data_offset = offsets[i];
        auto data_end = offsets[i + 1];
        auto next = begin( data ) + data_offset;
        move_data_till( next );
        // move forward the whole data block
        move_data_till.source += data_end - data_offset;
    }
    move_data_till( end( data ) );

    data.erase( move_data_till.destination, end( data ) );
}

void RemoveOffsets( std::vector<uint32_t>& offsets, Range<uint32_t const *> indices );