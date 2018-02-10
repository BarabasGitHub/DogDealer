#include "IndexedHelp.h"

#include "VectorHelper.h"

/// add an 'index' at 'position' in the 'indices' vector and fill the empty elements with 'fill_element'
void AddIndexToIndices( std::vector<uint32_t> & indices, uint32_t position, uint32_t index, uint32_t fill_element )
{
    auto size = std::max( position + 1, uint32_t( indices.size() ) );
    indices.resize( size, fill_element );
    indices[position] = index;
}


/// insert an 'index' at 'position' in the 'indices' vector and fill the empty elements with 'fill_element', also increase all elements that are equal or greater than the inserted index and not equal to the fill_element
void InsertIndexInIndices( std::vector<uint32_t> & indices, uint32_t position, uint32_t index, uint32_t fill_element )
{
    for( auto & current_index : indices )
    {
        if( current_index >= index && current_index != fill_element )
        {
            ++current_index;
        }
    }
    AddIndexToIndices( indices, position, index, fill_element );
}

void CalculateIndices( Range<UniformHandle const *> ids, uint32_t invalid_value, std::vector<uint32_t> & indices )
{
    indices.clear();
    for( uint32_t i = 0; i < Size( ids ); i++ )
    {
        AddIndexToIndices( indices, ids[i].index, i, invalid_value );
    }
}


void GetIndices( Range<UniformHandle const *> ids, Range<uint32_t const *> id_to_data, Range<uint32_t *> data_indices )
{
    assert(Size(ids) == Size(data_indices));
    auto size = Size(ids);
    for (auto i = 0u; i < size; ++i)
    {
        data_indices[i] = id_to_data[ids[i].index];
    }
}


void GetValidIndices( Range<UniformHandle const *> ids, Range<uint32_t const *> id_to_data, std::vector<uint32_t> & valid_data_indices )
{
    auto size = Size(ids);
    for (auto i = 0u; i < size; ++i)
    {
        auto index = GetOptional(id_to_data, ids[i].index);
        if( index != c_invalid_index )
        {
            valid_data_indices.push_back(index);
        }
    }
}


// replace the index 'remove_index', which is assumed to be unique in the indices vector, with 'new_index'
void ReplaceIndex( std::vector<uint32_t> & indices, uint32_t remove_index, uint32_t new_index )
{
    assert( std::count( begin( indices ), end( indices ), remove_index ) <= 1 );

    // find the entry to be replaced
    auto found = std::find( begin( indices ), end( indices ), remove_index );
    if( found != end( indices ) )
    {
        *found = new_index;
    }
}


std::vector<uint32_t> RemoveIndices( Range<uint32_t *> component_to_data, Range<UniformHandle const *> component_ids, uint32_t invalid_value )
{
    std::vector<uint32_t> indices;
    for( auto entity : component_ids )
    {
        auto index = GetOptional(component_to_data, entity.index);
        if( index != invalid_value )
        {
            indices.push_back( index );
            component_to_data[entity.index] = invalid_value;
        }
    }

    if( indices.empty() )
    {
        return indices;
    }
    // push_back max so we can loop over all
    indices.push_back( std::numeric_limits< uint32_t >::max() );
    for( auto i = 1u; i < indices.size(); ++i )
    {
        auto index_low = indices[i - 1];
        auto index_high = indices[i - 0];
        for( auto & data_index : component_to_data )
        {
            // index_low shouldn't be there any more, just like index high, so we can do < <
            if( index_low < data_index && data_index < index_high )
            {
                data_index -= i;
            }
        }
    }
    // remove max again
    indices.pop_back();

    return indices;
}