#pragma once

#include "Handle.h"
#include "Range.h"
#include "InvalidIndex.h"

#include <algorithm>
#include <vector>

/// add an 'index' at 'position' in the 'indices' vector and fill the empty elements with 'fill_element'
void AddIndexToIndices( std::vector<uint32_t> & indices, uint32_t position, uint32_t index, uint32_t fill_element = c_invalid_index );

/// insert an 'index' at 'position' in the 'indices' vector and fill the empty elements with 'fill_element', also increase all elements that are equal or greater than the inserted index and not equal to the fill_element
void InsertIndexInIndices( std::vector<uint32_t> & indices, uint32_t position, uint32_t index, uint32_t fill_element = c_invalid_index );


// replace the index 'remove_index', which is assumed to be unique in the indices vector, with 'new_index'
void ReplaceIndex( std::vector<uint32_t> & indices, uint32_t remove_index, uint32_t new_index );

// calculates the id-to-indices vector given a range of ids/handles, fills all unused elements with invalid_value
void CalculateIndices( Range<UniformHandle const *> ids, uint32_t invalid_value, std::vector<uint32_t> & indices );

template< typename Type >
void CalculateIndices( Range<Handle<Type> const *> ids, uint32_t invalid_value, std::vector<uint32_t> & indices )
{
    CalculateIndices( ReinterpretRange<UniformHandle const>( ids ), invalid_value, indices );
}

template< typename Type >
void CalculateIndices( Range<Handle<Type> *> ids, uint32_t invalid_value, std::vector<uint32_t> & indices )
{
    CalculateIndices( ReinterpretRange<UniformHandle const>( ids ), invalid_value, indices );
}

// gets the data index for each id given in the id_to_data range and stores it in data_indices
void GetIndices( Range<UniformHandle const *> ids, Range<uint32_t const *> id_to_data, Range<uint32_t *> data_indices );

template< typename Type >
void GetIndices( Range<Handle<Type> const *> ids, Range<uint32_t const *> id_to_data, Range<uint32_t *> data_indices )
{
    GetIndices( ReinterpretRange<UniformHandle const>( ids ), id_to_data, data_indices );
}

template< typename Type >
void GetIndices( Range<Handle<Type> *> ids, Range<uint32_t const *> id_to_data, Range<uint32_t *> data_indices )
{
    GetIndices( ReinterpretRange<UniformHandle const>( ids ), id_to_data, data_indices );
}


// gets the data index for each id given in the id_to_data range and stores it in data_indices
void GetValidIndices( Range<UniformHandle const *> ids, Range<uint32_t const *> id_to_data, std::vector<uint32_t> & valid_data_indices );

template< typename Type >
void GetValidIndices( Range<Handle<Type> const *> ids, Range<uint32_t const *> id_to_data, std::vector<uint32_t> & valid_data_indices )
{
    GetValidIndices( ReinterpretRange<UniformHandle const>( ids ), id_to_data, valid_data_indices );
}

template< typename Type >
void GetValidIndices( Range<Handle<Type> *> ids, Range<uint32_t const *> id_to_data, std::vector<uint32_t> & valid_data_indices )
{
    GetValidIndices( ReinterpretRange<UniformHandle const>( ids ), id_to_data, valid_data_indices );
}

// returns the removed indices
std::vector<uint32_t> RemoveIndices( Range<uint32_t *> component_to_data, Range<UniformHandle const *> component_ids, uint32_t invalid_value = c_invalid_index );

// returns the removed indices
template< typename Type >
std::vector<uint32_t> RemoveIndices( Range<uint32_t *> component_to_data, Range<Handle<Type> const *> component_ids, uint32_t invalid_value = c_invalid_index )
{
    return RemoveIndices( component_to_data, ReinterpretRange<UniformHandle const>( component_ids ), invalid_value );
}