#pragma once

#include "Handle.h"
#include "HandleIDPair.h"
#include "Range.h"
#include "VectorHelper.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <functional>
#include <map>
#include <vector>



// returns the new offset for a container with gaps
template<typename DataType0, typename ... DataTypes >
uint32_t FindNewOffset( std::multimap<uint32_t, uint32_t> & gaps, uint32_t gap_size, std::vector<DataType0> & container0, std::vector<DataTypes> & ... containers )
{
    auto found = gaps.lower_bound( gap_size );

    uint32_t new_offset;
    // if we're at the end
    if( found == end( gaps ) )
    {
        // No gap found, add to end of storage
        auto old_size = uint32_t(container0.size());
        ResizeMultipleVectors( old_size + gap_size, container0, containers... );
        new_offset = old_size;
    }
    else
    {
        assert( found->first >= gap_size );
        new_offset = found->second;
        auto left_over = found->first - gap_size;
        gaps.erase( found );
        // if the gap is bigger than we needed, give back the unused space
        if( left_over > 0 )
        {
            gaps.emplace( left_over, new_offset + gap_size );
        }
    }
    return new_offset;
}

// containers without an index list

//template<typename ContainerType, typename FreeListType, typename ElementType>
//uint32_t Add(ContainerType& container, FreeListType& free_list, ElementType element)
//{
//    if( free_list.empty() )
//    {
//        auto new_id = static_cast<uint32_t>(container.size());
//        container.resize( new_id + 1 );
//        free_list.push_back(new_id);
//    }
//    auto id = free_list.front();
//    container[id] = element;
//    free_list.pop_front();
//    return id;
//
//}

/// add an element to a container with generations and a free list
template<typename Type, typename ElementType>
void Add( Handle<Type> &id, ElementType new_element, std::vector<ElementType>& container, std::vector<typename Handle<Type>::generation_t>& generations, std::deque<typename Handle<Type>::index_t>& free_list )
{
    using namespace std;
    if( free_list.empty() )
    {
        auto index = static_cast<typename Handle<Type>::index_t>( container.size() );
        container.emplace_back( move( new_element ) );
        generations.emplace_back( uint8_t(0) );
        id.index = index;
        id.generation = 0;
    }
    else
    {
        auto index = free_list.front();
        container[index] = move( new_element );
        free_list.pop_front();
        id.index = index;
        id.generation = generations[index];
    }
}

/// add an element to a container with an index list, generations and a free list
template<typename Type, typename ElementType>
void Add( typename Handle<Type> &id, ElementType new_element, std::vector<ElementType>& container, std::vector<typename Handle<Type>::index_t>& indices, std::vector<typename Handle<Type>::generation_t>& generations, std::deque<typename Handle<Type>::index_t>& free_list )
{
    typedef typename Handle<Type>::index_t IndexType;

    auto index = static_cast<IndexType>( container.size( ) );
    container.emplace_back( new_element );

    if( free_list.empty( ) )
    {
        auto id_index = static_cast<IndexType>( indices.size( ) );
        indices.emplace_back( index );
        generations.emplace_back( 0 );
        id.index = id_index;
        id.generation = 0;
    }
    else
    {
        auto id_index = free_list.front( );
        indices[id_index] = index;
        free_list.pop_front( );
        id.index = id_index;
        id.generation = generations[id_index];
    }
}

/// remove an element from a container with an index list, generations and a free list
template<typename ElementType>
void Remove( UniformHandle::index_t index, std::vector<ElementType>& container, std::vector<UniformHandle::index_t>& index_container, std::vector<UniformHandle::generation_t>& generation_container, std::deque<UniformHandle::index_t >& free_list )
{
    using namespace std;
    // invalidate the id by increasing the generation in the container
    ++generation_container[index];
    auto data_index = index_container[index];
    SwapAndPrune(data_index, container);
    index_container[index] = c_invalid_index;
    // now update the index container
    // replace all indices that pointed to the last element to point to the element it was swapped with
    auto last_index = static_cast<UniformHandle::index_t>( container.size() );
    replace_if(index_container.begin(), index_container.end(), [last_index](uint32_t i){ return i == last_index; }, data_index);
    // register the gap in the index container
    free_list.emplace_back(index);
}


template<typename ObjectType, typename... Ts>
void RemoveIDsWithSwapAndPrune(Range<Handle<ObjectType> const *> entities_to_be_removed, std::vector<uint32_t> & entity_to_data, std::vector<Handle<ObjectType>> & container_entities, std::vector<Ts> & ... data)
{
    for( auto i = 0u; i < Size(entities_to_be_removed); ++i )
    {
        auto entity_to_be_removed = entities_to_be_removed[i];

        auto entity_data_index = GetOptional(entity_to_data, entity_to_be_removed.index);
        if( entity_data_index != c_invalid_index )
        {
            // data entries with last entry
            SwapAndPrune( entity_data_index, container_entities, data... );

            // Invalidate the old index entry and update the former user of last entry to use new position
            entity_to_data[entity_to_be_removed.index] = c_invalid_index;
            if( entity_data_index < container_entities.size() ) // this is not true if we removed the last entity
            {
                auto const & swapped_entity = container_entities[entity_data_index];
                entity_to_data[swapped_entity.index] = entity_data_index;
            }
        }
    }
}


template<typename ObjectType, typename... Ts>
void RemoveIDWithSwapAndPrune(Handle<ObjectType> handle_to_be_removed, std::vector<Handle<ObjectType>> & container_handles, std::vector<Ts> & ... data)
{
    for( auto i = 0u; i < Size(container_handles); ++i )
    {
        if(container_handles[i] == handle_to_be_removed)
        {
            SwapAndPrune(i, container_handles, data... );
        }
    }
}


template<typename ObjectType, typename... Ts>
void RemoveIDWithSwapAndPrune(Handle<ObjectType> handle_to_be_removed, std::vector<HandleIDPair<ObjectType>> & container_handle_pairs, std::vector<Ts> & ... data)
{
    for( auto i = 0u; i < Size(container_handle_pairs); ++i )
    {
        if(container_handle_pairs[i].id1 == handle_to_be_removed || container_handle_pairs[i].id2 == handle_to_be_removed)
        {
            SwapAndPrune(i, container_handle_pairs, data... );
        }
    }
}
