#pragma once

#include "Structures.h"

#include <Utilities\Range.h>

namespace Animating{
    	
    // Output of the blending system
    // containing bone weights for all active sequences of all components
    struct BlendedBoneWeightData
    {
        // Bone weights for all sequences of all components
        std::vector<float> bone_weights;
        
        // SequenceIDs and time progress of all sequences of all components
        std::vector<SequenceID> sequences;
        std::vector<float> sequence_time_progress;

        // Refers to 'sequences', start indices of all seqeuences belonging to one component
        std::vector<unsigned> component_sequence_offsets; // was sequence_data_indices
        
        // Refers to 'bone_weights', start indices of all weights belonging to one component
        std::vector<unsigned> component_bone_weight_offsets; // sequence_bone_data_indices
    };



    // Filter out all weights<->sequences and add them to the back of the active weights and nonzero sequences
    // the output contains all weights and sequences for which the weight is nonzero
    void FilterZeroWeights( Range<float const *> sequence_weights,
                            Range<SequenceID const *> sequences,
                            Range<float const*> bone_weights,
                            std::vector<float> & output_weights,
                            std::vector<SequenceID> & output_sequences,
                            float const total_weight );

    // normalizes the weights for all bones, the workspace needs to have the size of the number of bones
    void NormalizeWeights( Range<float*> weights, Range<float*> workspace );

    // extracts the active sequences for all states of state_infos
    // 
    void ExtractActiveSequences( 
        // input
        Range<StateInfo const *> state_infos,
        Range<StateData const *> state_datas,
        Range<float const *> sequence_weights,
        Range<SequenceID const *> sequence_ids,
        Range<float const*> sequence_bone_masks,
        // output
        Range<uint32_t *> active_state_sequence_counts,
        std::vector<SequenceID>& nonzero_sequences,
        std::vector<float>& sequence_time_progress,
        std::vector<float>& active_bone_weights );

    // Adds all bone weights for several sequences, which should be contigeous in the sequence_bone_weights
    // Adds to the existing content in total_bone_weights
    void AddBoneWeightsForSequences( Range<float const*> sequence_bone_weights, Range<float *> total_bone_weights );

    // ...
    void CalculateNormalizationFactorsAndAdjustLeftOverWeights( Range<float const*> total_weights, Range<float *> left_over_weights, Range<float *> normalization_factors );

    // multiply bone weights for multiple sequences
    void MultiplyBoneWeights( Range<float*> bone_weights, Range<float const*> multiplication_factors );

    void AssignPrioritizedWeights( unsigned const bone_count,
                                   Range<StateData const*> state_datas,
                                   Range<unsigned const*> active_state_sequence_counts,
                                   Range<float *> bone_weights,
                                   Range<float *> weight_workspace );

    // animations point to animation states
    // animation states point to sequence weights, sequence ids and sequence bone weights
    // sequence data indices store the offsets to the data in nonzero sequences and sequence time progress, data for animation i starts at indices[i] and ends before indices[i + 1]
    // sequence bone data indices store the offsets to the active bone weights, bone data for animation i starts at indices[i] and ends before indices[i + 1]
    void CalculateWeights(
        // animations
        Range<AnimationInfo const *> animation_infos,
        // state data
        Range<StateInfo const*> state_infos, Range<StateData const *> state_datas, Range<float const *> sequence_weights, Range<SequenceID const *> sequence_ids, Range<float const*> sequence_bone_weights,
        // sequence output data
        Range<unsigned *> sequence_data_indices, std::vector<SequenceID>& nonzero_sequences, std::vector<float>& sequence_time_progress,
        // sequence bone output data
        Range<unsigned *> sequence_bone_data_indices, std::vector<float>& active_bone_weights);
}