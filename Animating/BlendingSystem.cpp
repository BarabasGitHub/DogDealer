#include "BlendingSystem.h"

#include "StateContainer.h"

#include "SequenceContainer.h"

#include <Math\MathFunctions.h>
#include <Math\VectorAlgorithms.h>

#include <Utilities\IntegerRange.h>
#include <Utilities\Range.h>
#include <Utilities\StdVectorFunctions.h>

#include <numeric>

namespace Animating
{
    // Filter out all weights<->sequences and add them to the back of the active weights and nonzero sequences
    // the output contains all weights and sequences for which the weight is nonzero
    void FilterZeroWeights( Range<float const *> sequence_weights,
                            Range<SequenceID const *> sequences,
                            Range<float const*> bone_weights,
                            std::vector<float> & output_weights,
                            std::vector<SequenceID> & output_sequences,
                            float const total_weight)
    {
        if( total_weight == 0 ) return;
        auto bone_count = uint32_t(Size( bone_weights ));
        // grow the vector by the maximum possibly needed elements
        auto output_range = Grow(output_weights, bone_count * Size(sequence_weights) );
        // keep track of where to put the next data
        auto output_index = 0u;
        for( auto j = 0u; j < Size(sequence_weights); ++j )
        {
            auto sequence_weight = sequence_weights[j];
            if( sequence_weight > 0 )
            {
                sequence_weight *= total_weight;
                output_sequences.emplace_back( sequences[j] );
                Math::Multiply( bone_weights, sequence_weight, CreateRange( output_range, output_index, output_index + bone_count ) );
                output_index += bone_count;
            }
        }
        // shrink the vector by the amount of unused elements
        Shrink( output_weights, size_t(Size( output_range ) - output_index) );
    }

    // normalizes the weights for all bones, the workspace needs to have the size of the number of bones
    void NormalizeWeights( Range<float*> weights, Range<float*> workspace )
    {
        assert( !IsEmpty( workspace ) );
        assert( Size( weights ) % Size( workspace ) == 0 );
        // initialize the total weights to zero
        std::fill( begin( workspace ), end( workspace ), 0.f );
        // sum the total for each bone
        auto weight = begin( weights );
        while( weight != end( weights ) )
        {
            for( auto& total_weight : workspace )
            {
                total_weight += *weight;
                ++weight;
            }
        }
        assert( std::count( begin( workspace ), end( workspace ), 0.f ) == 0 );
        // adjust the weights
        weight = begin( weights );
        while( weight != end( weights ) )
        {
            for( auto total_weight : workspace )
            {
                *weight /= total_weight;
                ++weight;
            }
        }
    }

    void ExtractActiveSequences(Range<StateInfo const *> state_infos,
                                Range<StateData const *> state_datas,
                                Range<float const *> sequence_weights,
                                Range<SequenceID const *> sequence_ids,
                                Range<float const*> sequence_bone_weight_modifiers,
                                Range<uint32_t *> active_state_sequence_counts,
                                std::vector<SequenceID>& nonzero_sequences,
                                std::vector<float>& sequence_time_progress,
                                std::vector<float>& active_bone_weights)
    {
        assert( Size( state_infos ) == Size( state_datas ) );

        auto state_count = Size( state_infos );
        for( auto j = 0u; j < state_count; j++ )
        {
            auto const state_info = state_infos[j];
            auto const & state_data = state_datas[j];
            auto total_weight = state_data.total_weight;
            auto time_progress = state_data.time_progress;

            // Weights for this state
            auto weights = CreateRange( begin(sequence_weights) + state_info.offset, state_info.count );

            // Sequence ids and bone weights for this state
            auto sequences = CreateRange( begin(sequence_ids) + state_info.offset, state_info.count );
            auto bone_weights = CreateRange(sequence_bone_weight_modifiers, state_info.bone_weight_offset, state_info.bone_weight_offset + state_info.bone_count);

            // Store the previous size to ...
            auto previous_size = Size( nonzero_sequences );
            FilterZeroWeights( weights, sequences, bone_weights, active_bone_weights, nonzero_sequences, total_weight );
            auto new_size = Size( nonzero_sequences );

            // ... keep track of how many sequences we added
            auto added_states_count = new_size - previous_size;

            // Store the time_progress for these sequences
            sequence_time_progress.resize( new_size, time_progress );

            active_state_sequence_counts[j] = uint32_t(added_states_count);
        }

    }


    void AddBoneWeightsForSequences(Range<float const*> sequence_bone_weights, Range<float *> total_bone_weights)
    {
        assert( Size( sequence_bone_weights ) % Size( total_bone_weights ) == 0 );
        auto bone_count = Size( total_bone_weights );
        auto total_bone_count = Size( sequence_bone_weights );
        // iterate over all active sequences of the state
        for( size_t i = 0u; i < total_bone_count; i += bone_count )
        {
            Math::Add( CreateRange( begin(sequence_bone_weights) + i, bone_count ), total_bone_weights );
        }
    }


    void CalculateNormalizationFactorsAndAdjustLeftOverWeights( Range<float const*> total_weights, Range<float *> left_over_weights, Range<float *> normalization_factors )
    {
        assert( Size( left_over_weights ) == Size( total_weights ) );
        assert( Size( total_weights ) == Size( normalization_factors ) );

        auto bone_count = Size( left_over_weights );
        for( auto i = 0u; i < bone_count; ++i )
        {
            // fix normalization factor
            auto total_weight = total_weights[i];
            auto left_over_weight = left_over_weights[i];
            auto normalization_factor = Math::Min( total_weight, left_over_weight );
            // subtract from left over weights, max with 0 so it never goes negative due to rounding errors
            left_over_weights[i] = Math::Max(0.f, left_over_weight - normalization_factor);
            if( total_weight != 0 )
            {
                normalization_factor /= total_weight;
            }
            normalization_factors[i] = normalization_factor;
        }
    }


    // multiply bone weights for multiple sequences
    void MultiplyBoneWeights( Range<float*> const bone_weights, Range<float const*> const multiplication_factors )
    {
        // check if the size is a multiple
        assert( Size( bone_weights ) % Size( multiplication_factors ) == 0 );

        auto size = Size( multiplication_factors );
        auto bone_weight = begin( bone_weights );
        while( bone_weight != end(bone_weights) )
        {
            auto range = CreateRange( bone_weight, size );
            Math::Multiply( multiplication_factors, range );
            bone_weight = end( range );
        }
    }


    void AssignPrioritizedWeights(unsigned const bone_count,
                                Range<StateData const*> state_datas,
                                Range<unsigned const*> active_state_sequence_counts,
                                Range<float *> bone_weights,
                                Range<float *> weight_workspace)
    {
        assert( Size( weight_workspace ) >= bone_count * 2 );
        assert( Size( bone_weights ) % bone_count == 0 );

        auto left_over_bone_weights = CreateRange(weight_workspace, 0, bone_count);
        std::fill(begin(left_over_bone_weights), end(left_over_bone_weights), 1.f);

        auto total_bone_weights_per_priority = CreateRange(weight_workspace, bone_count, 2 * bone_count);

        auto bone_weight_begin = begin(bone_weights);
        auto bone_weight_end = begin(bone_weights);

        auto state_sequence_count = begin(active_state_sequence_counts);

        // Iterate over all states for the current current AnimationInfo
        // Group states by priority and store total bone weights for all their sequences
        for( auto state_data = begin( state_datas ); state_data != end( state_datas );  )
        {
            std::fill(begin(total_bone_weights_per_priority), end(total_bone_weights_per_priority), 0.f);

            // Iterate over all states with the same priority and mixing blend mode.
            // By convention, the states for each component are sorted hierarchically by:
            //  -blend mode
            //  -priority
            for( auto priority = state_data->priority;
                 state_data != end( state_datas ) &&
                 priority == state_data->priority;
                 ++state_data,
                 ++state_sequence_count )
            {
                // add up the bone weights
                if (state_data->total_weight > 0)
                {
                    auto sequence_count = *state_sequence_count;
                    auto bone_weight_count = sequence_count * bone_count;
                    // add the bone weights for all sequences belonging to a state to the bone weights for the current priority
                    AddBoneWeightsForSequences( CreateRange( bone_weight_end, bone_weight_count ), total_bone_weights_per_priority );
                    // update the end of the current bone weights
                    bone_weight_end += bone_weight_count;
                    assert( bone_weight_end <= end( bone_weights ) );
                }
            }

            // make a copy of the range (pointing to the same data) with a more appropriate name
            auto normalization_factors = total_bone_weights_per_priority;
            CalculateNormalizationFactorsAndAdjustLeftOverWeights( total_bone_weights_per_priority, left_over_bone_weights, normalization_factors );

            // normalize the bone weights
            MultiplyBoneWeights( CreateRange( bone_weight_begin, bone_weight_end ), normalization_factors );

            // update the begin for the next iteration
            bone_weight_begin = bone_weight_end;
        }
    }


    namespace
    {
        uint32_t FindLastBlendStateIndex(Range<StateData const *> state_datas)
        {
            auto state_count = uint32_t(Size( state_datas ));
            for( uint32_t i = 0; i < state_count; i++ )
            {
                if( state_datas[i].blend_mode != BlendMode::Mixing )
                {
                    return i;
                }
            }
            return state_count;
        }
    }


    // animation_infos point to animation states
    // animation states point to sequence weights, sequence ids and sequence bone weights
    // sequence data indices store the offsets to the data in nonzero sequences and sequence time progress, data for animation i starts at indices[i] and ends before indices[i + 1]
    // sequence bone data indices store the offsets to the active bone weights, bone data for animation i starts at indices[i] and ends before indices[i + 1]
    void CalculateWeights(
        // animations
        Range<AnimationInfo const *> animation_infos,
        // state data
        Range<StateInfo const*> state_infos, Range<StateData const*> state_datas, Range<float const *> sequence_weights, Range<SequenceID const *> sequence_ids, Range<float const*> sequence_bone_weight_modifiers,
        // sequence output data
        Range<unsigned *> sequence_data_indices, std::vector<SequenceID>& nonzero_sequences, std::vector<float>& sequence_time_progress,
        // sequence bone output data
        Range<unsigned *> sequence_bone_data_indices, std::vector<float>& active_bone_weights)
    {
        assert( Size( sequence_data_indices ) == Size( sequence_bone_data_indices ) );
        assert( Size( sequence_data_indices ) - 1 == Size( animation_infos ) );

        nonzero_sequences.clear();
        sequence_time_progress.clear();
        active_bone_weights.clear();

        std::vector<unsigned> active_state_sequence_counts;
        std::vector<float> weight_workspace;

        for( auto i = 0u; i < Size( animation_infos ); ++i )
        {
            // keep track of the data indices, both for sequence data ...
            auto data_offset = uint32_t( Size( nonzero_sequences ) );
            sequence_data_indices[i] = data_offset;

            // and for bone data
            auto active_bone_weight_offset = uint32_t( Size( active_bone_weights ) );
            sequence_bone_data_indices[i] = active_bone_weight_offset;

            // get stuff and check whether we actually have states
            auto animation_info = animation_infos[i];
            auto state_count = animation_info.count;
            if( state_count == 0 ) continue;

            // ------ get sequences with nonzero weights and their bone weights etc ------

            // reset the counts for active sequences per state
            ResetSize(active_state_sequence_counts, state_count);

            // Loop over all states of the current AnimationInfo
            auto states_end_index = animation_info.offset + state_count;
            auto current_state_datas = CreateRange( state_datas, animation_info.offset, states_end_index );
            auto current_state_infos = CreateRange( state_infos, animation_info.offset, states_end_index );

            ExtractActiveSequences(
                    current_state_infos,
                    current_state_datas,
                    sequence_weights,
                    sequence_ids,
                    sequence_bone_weight_modifiers,
                    active_state_sequence_counts,
                    nonzero_sequences,
                    sequence_time_progress,
                    active_bone_weights );

            // -------- Do weight calculations ---------

            // Get bone count for component from first used state
            auto bone_count = state_infos[animation_info.offset].bone_count;

            // Moeck does other things [Bas made it shorter]
            auto first_additive_state_index = FindLastBlendStateIndex( current_state_datas );
            if(first_additive_state_index > 0)
            {
                auto blended_state_sequence_counts = CreateRange(active_state_sequence_counts, 0, first_additive_state_index);
                auto first_additive_state_bone_weight_index = std::accumulate(begin(blended_state_sequence_counts), end(blended_state_sequence_counts), 0u);
                first_additive_state_bone_weight_index *= bone_count;

                // Oh my god [i.e.: use only the blended stuff for prioritized weights and normalizing]
                auto blended_bone_weights = CreateRange(active_bone_weights, active_bone_weight_offset, active_bone_weight_offset + first_additive_state_bone_weight_index);
                auto blended_state_datas = CreateRange(current_state_datas, 0, first_additive_state_index);

                // reset weight workspace
                ResetSize(weight_workspace, bone_count * 2);

                if(animation_info.count > 1)
                {
                    AssignPrioritizedWeights(bone_count,
                                             blended_state_datas,
                                             blended_state_sequence_counts,
                                             blended_bone_weights,
                                             weight_workspace);
                }


                // Normalize the weights so they all add up to 1 for each bone
                NormalizeWeights(blended_bone_weights, CreateRange(weight_workspace, 0, bone_count));
            }
        }

        Last( sequence_data_indices ) = uint32_t(nonzero_sequences.size());
        Last( sequence_bone_data_indices ) = uint32_t(active_bone_weights.size());
    }
}