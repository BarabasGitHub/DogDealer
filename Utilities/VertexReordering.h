#pragma once

#include "Range.h"

#include <cstdint>
#include <vector>


std::vector<uint32_t> CalculateNewIndices( std::vector<uint32_t> const & indices, unsigned const vertex_count );

template<typename DataType>
std::vector<DataType> ReorderData( std::vector<DataType> const & data, std::vector<uint32_t> const & old_indices, std::vector<uint32_t> const & new_indices );

template<typename DataType>
uint32_t ReorderData( std::vector<DataType> const & data, std::vector<uint32_t> const & old_indices, std::vector<uint32_t> const & new_indices, Range<DataType *> new_data );




// implementation
template<typename DataType>
std::vector<DataType> ReorderData( std::vector<DataType> const & data, std::vector<uint32_t> const & old_indices, std::vector<uint32_t> const & new_indices )
{
    std::vector<DataType> new_data(data.size());
    auto size = ReorderData<DataType>(data, old_indices, new_indices, new_data);
    new_data.resize(size);
    return new_data;
}


template<typename DataType>
uint32_t ReorderData( std::vector<DataType> const & data, std::vector<uint32_t> const & old_indices, std::vector<uint32_t> const & new_indices, Range<DataType*> new_data )
{
    assert( old_indices.size( ) == new_indices.size( ) );
    assert( data.size( ) == Size(new_data) );

    auto destination_index = 0u;
    for( auto i = 0u;  i < old_indices.size(); ++i )
    {
        // if the new index is smaller it means we have already moved the data.
        if( new_indices[i] < destination_index) continue;
        assert( new_indices[i] == destination_index );
        new_data[destination_index] = data[old_indices[i]];
        destination_index += 1;
    }
    return destination_index;
}