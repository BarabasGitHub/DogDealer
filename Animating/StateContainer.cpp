#include "StateContainer.h"
#include "Structures.h"

#include "SequenceContainer.h"

#include <Utilities\Range.h>
#include <Utilities\ContainerHelpers.h>
#include <Utilities\VectorHelper.h>

#include <functional>
#include <numeric>

using namespace Animating;

AnimationInfo StateContainer::CreateAnimationInfo()
{
    AnimationInfo info;
    info.count = 0;
    info.offset = 0;
    return info;
}

void StateContainer::RemoveAnimationInfo( AnimationInfo info )
{
    std::vector<uint32_t> indices( info.count );
    std::iota( begin( indices ), end( indices ), info.offset );

    auto result = RemoveStates( info, indices );
    (void) result;
    assert( result.count == 0 );
}


uint32_t StateContainer::InsertNewSequence( SequenceID sequence, float weight )
{
    auto offset = InsertNewSequencesAndWeights(CreateRange(&sequence, 1));
    m_sequence_weights[offset] = weight;
    return offset;
}


// Finds a new position in memory for the input weights and types and returns the offset
uint32_t StateContainer::InsertNewSequencesAndWeights( Range<SequenceID const *> sequences )
{
    // Find existing gap of sufficient size
    auto block_size = (uint32_t) Size(sequences);


    assert( m_sequence_ids.size() == m_sequence_weights.size() );
    auto offset = FindNewOffset( m_sequence_gaps, block_size, m_sequence_weights, m_sequence_ids );

    // Overwrite data in gap
    std::copy_n( begin(sequences), block_size, begin(m_sequence_ids) + offset );
    std::fill_n( begin( m_sequence_weights ) + offset, block_size, 1.f );

    return offset;
}


// returns the offset
uint32_t StateContainer::InsertBoneWeights( Range<float const *> bone_weights )
{
    auto bone_count = uint32_t( Size( bone_weights ) );
    auto offset = FindNewOffset( m_bone_mask_gaps, bone_count, m_bone_masks );

    std::copy_n( begin( bone_weights ), Size(bone_weights), begin( m_bone_masks ) + offset );

    return offset;
}


AnimationInfo StateContainer::AddState( AnimationInfo animation_info, StateInfo state_info, StateLogic state_logic, StateData state_data )
{
    auto block_end = animation_info.offset + animation_info.count;
    assert( block_end <= m_state_infos.size() );
    // check for bone count consistency
    assert( animation_info.count == 0 || m_state_infos[animation_info.offset].bone_count == state_info.bone_count );

    if( block_end == m_state_infos.size() )
    {
        m_state_infos.push_back(state_info);
        m_state_logics.push_back(state_logic);
        m_state_datas.push_back(state_data);
    }
    else
    {
        auto new_offset = FindNewOffset( m_state_gaps, animation_info.count + 1, m_state_infos, m_state_logics, m_state_datas);

        // Copy state infos
        std::copy_n( begin( m_state_infos ) + animation_info.offset, animation_info.count, begin( m_state_infos ) + new_offset );
        // Add new state info
        m_state_infos[new_offset + animation_info.count] = state_info;

        // Copy state logic
        std::copy_n( begin( m_state_logics ) + animation_info.offset, animation_info.count, begin( m_state_logics ) + new_offset );
        // Add new state logic
        m_state_logics[new_offset + animation_info.count] = state_logic;

        // Copy state datas
        std::copy_n( begin( m_state_datas ) + animation_info.offset, animation_info.count, begin( m_state_datas ) + new_offset );
        // Add new state data
        m_state_datas[new_offset + animation_info.count] = state_data;


        // register old stuff as gaps
        if( animation_info.count > 0 )
        {
            m_state_gaps.emplace( animation_info.count, animation_info.offset );
        }
        // update offset
        animation_info.offset = new_offset;
    }

    // Update AnimationInfo
    ++animation_info.count;

    // TODO: should be done when inserting the state
    SortStates( animation_info.offset, animation_info.count );

    return animation_info;
}


void StateContainer::RemoveStates( uint32_t offset, uint32_t count )
{
    ClearStates( offset, count );

    // remove the states themself
    {
        auto states_end = offset + count;
        // Create no gaps at end of vector
        if( states_end < m_state_infos.size() )
        {
            m_state_gaps.emplace( count, offset );
        }
        else
        {
            m_state_infos.resize( offset );
        }
    }

}


void StateContainer::ClearStates( uint32_t const offset, uint32_t const count)
{
    auto states_end = offset + count;

    // register the sequences and weights as gap
    for (auto i = offset; i < states_end; i++)
    {
        auto& state_info = m_state_infos[i];
        ClearState(state_info);
    }
}


void StateContainer::ClearState(StateInfo const & state_info)
{
    auto gap_end = state_info.offset + state_info.count;
    if (gap_end < m_sequence_ids.size())
    {
        m_sequence_gaps.emplace(state_info.count, state_info.offset);
    }
    else
    {
        // Shorten end index
        auto new_size = state_info.offset;
        m_sequence_weights.resize(new_size);
        m_sequence_ids.resize(new_size);
    }

    auto end = state_info.bone_weight_offset + state_info.bone_count;

    // DEBUG
    std::fill(m_bone_masks.begin() + state_info.bone_weight_offset, m_bone_masks.begin() + end, -10.f);

    if (end < m_bone_masks.size())
    {
        m_bone_mask_gaps.emplace(state_info.bone_count, state_info.bone_weight_offset);
    }
    else
    {
        // Shrink vector to exclude created gap
        m_bone_masks.resize(state_info.bone_weight_offset);
    }
}


// Swap expired states to end of block associated with the AnimationInfo
// Input: original AnimationInfo and absolute indices of states to be removed
// Return: reduced AnimationInfo
AnimationInfo StateContainer::RemoveStates( AnimationInfo animation_info, Range<uint32_t const *> absolute_indices )
{
    assert( std::is_sorted( begin( absolute_indices ), end( absolute_indices ) ) );

	// Get end of block of AnimationStateInfos associated with the AnimationInfo
    auto block_end_index = animation_info.offset + animation_info.count;
    auto removed_states = 0u;

    std::vector<StateInfo> to_be_cleared_states;

	// Loop over indices of expired states
    auto index = begin( absolute_indices );
    while( index != end(absolute_indices) )
    {
		// Get index of expired state and mark for deletion
        auto state_index = *index;

        // check that we're not removing the same state twice
        assert( std::find( begin( to_be_cleared_states ), end( to_be_cleared_states ), m_state_infos[state_index]) == end( to_be_cleared_states ) );
        to_be_cleared_states.emplace_back( std::move( m_state_infos[state_index] ) );

		// Get index of next expired AnimationStateInfo
        ++index;

		// Get the source index of either the last state of the block or the last state before the next one that has to be removed
		auto source_end_index = index == end(absolute_indices) ? block_end_index : *index;

		// Move states to the front
        // destination is the state info that has to be removed, plus 'removed_states' places earlier
        auto destination_index = ( state_index - removed_states );
        // source is the next state, one after the one that is being removed. Unless the one that is removed is the last one
        auto source_index = std::min(state_index + 1, source_end_index);

        // Move state info to front of block
        {
            auto source = begin(m_state_infos) + source_index;
            auto source_end = begin(m_state_infos) + source_end_index;
            auto destination = begin( m_state_infos ) + destination_index;
            std::move(source, source_end, destination);
        }

        // Move state logic to front of block
        {
            auto source = begin(m_state_logics) + source_index;
            auto source_end = begin(m_state_logics) + source_end_index;
            auto destination = begin(m_state_logics) + (state_index - removed_states);

            std::move(source, source_end, destination);
        }

        // Move state data to front of block
        {
            auto source = begin(m_state_datas) + source_index;
            auto source_end = begin(m_state_datas) + source_end_index;
            auto destination = begin(m_state_datas) + (state_index - removed_states);

            std::move(source, source_end, destination);
        }

        ++removed_states;
    }

    // Shorten block to exclude pruned states
    animation_info.count -= removed_states;

    // 14-11-2014: Do we really need to sort them?
    if( removed_states != 0 )
    {
        SortStates( animation_info.offset, animation_info.count );
    }

    // ### Moeck: No mercy for duplicates   ###
    for (auto i = animation_info.offset; i < animation_info.offset + animation_info.count; i++)
    {
        assert( std::find( begin( m_state_infos ) + i + 1, begin( m_state_infos ) + animation_info.offset + animation_info.count, m_state_infos[i] ) == begin(m_state_infos) + animation_info.offset + animation_info.count);
    }
    // ########################################

    for( auto& state_info : to_be_cleared_states )
    {
        ClearState( state_info );
    }

    // Register the gap
    {
        auto gap_offset = animation_info.offset + animation_info.count;
        auto states_end = block_end_index;
        // Create no gaps at end of vector
        if( states_end < m_state_infos.size() )
        {
            m_state_gaps.emplace( removed_states, gap_offset );
        }
        else
        {
            m_state_infos.resize( gap_offset );
            m_state_logics.resize( gap_offset );
            m_state_datas.resize( gap_offset );
        }
    }

    assert(m_state_infos.size() == m_state_logics.size());
    assert(m_state_infos.size() == m_state_datas.size());

    return animation_info;
}


void StateContainer::SortStates( uint32_t offset, uint32_t count )
{
    assert( m_state_infos.size() >= offset + count );

    std::vector<uint32_t> mapping(count);
    std::iota( begin( mapping ), end( mapping ), 0u );

    // Create mapping according to sorted priorities
    auto state_datas_begin = begin( m_state_datas ) + offset;

    // In decreasing hierarchy, sort states by:
    //  -state data blend mode
    //  -state data priority
    // This order is used in the BlendingSystems AssignPrioritizedWeights()
    std::sort(begin(mapping), end(mapping), [state_datas_begin](uint32_t a, uint32_t b)
    {
        if (state_datas_begin[a].blend_mode == state_datas_begin[b].blend_mode)
        {
            // If equal blend mode, sort higher priorities to the front
            return state_datas_begin[a].priority > state_datas_begin[b].priority;
        }
        else
        {
            // Otherwise sort according to order of enum
            return state_datas_begin[a].blend_mode < state_datas_begin[b].blend_mode;
        }
    });

    auto unsorted_info_range = CreateRange( m_state_infos, offset, offset + count );
    auto sorted_infos = Reorder<StateInfo>( unsorted_info_range, mapping );
    std::copy_n( begin( sorted_infos ), count, begin( unsorted_info_range ) );

    auto unsorted_logics_range = CreateRange( m_state_logics, offset, offset + count );
    auto sorted_logics = Reorder<StateLogic>( unsorted_logics_range, mapping );
    std::copy_n( begin( sorted_logics ), count, begin( unsorted_logics_range ) );

    auto unsorted_data_range = CreateRange( m_state_datas, offset, offset + count );
    auto sorted_datas = Reorder<StateData>( unsorted_data_range, mapping );
    std::copy_n( begin( sorted_datas ), count, begin( unsorted_data_range ) );
}


// Unite adjacent gaps
void StateContainer::MergeGaps()
{
    // STATE INFOS
    // Store gaps by index, with default size 0
    auto gap_sizes = std::vector<uint32_t>( m_state_infos.size(), 0 );

    // Fill in existing gap sizes
    for( auto& gap : m_state_gaps )
    {
        auto gap_size = gap.first;
        auto gap_position = gap.second;

        gap_sizes[gap_position] = gap_size;
    }

    // Keep start index of previous gap
    auto last_gap_start = -1;

    auto i = 0u;

    // Iterate over gaps, trying to connect them
    while( i < gap_sizes.size() )
    {
        // Skip index if no gap exists
        if( gap_sizes[i] == 0 )
        {
            // Set last adjacent gap start to invalid
            last_gap_start = -1;
            ++i;
            continue;
        }

        // Otherwise mark new start of gap if no previous one
        if( last_gap_start == -1 )
        {
            // Start new gap
            last_gap_start = i;
        }
        else
        {
            // Merge with previous gap
            gap_sizes[last_gap_start] += gap_sizes[i];
            gap_sizes[i] = 0;
        }

        // Skip to end of inspected gap
        i += gap_sizes[i];
    }

    // Update m_state_info_gaps with merged gaps
    m_state_gaps.clear();

    for( auto j = 0u; j < gap_sizes.size(); j++ )
    {
        // Get size of gap
        auto gap_size = gap_sizes[j];

        // Skip irrelevant indices
        if( gap_size == 0 ) continue;

        auto gap_position = j;

        m_state_gaps.emplace( gap_size, gap_position );
    }
}