#include "CppUnitTest.h"

#include "Animating\BlendingSystem.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Animating;

namespace
{
    StateData CreateDefaultStateData()
    {
        StateData state_data;
        state_data.blender.index = std::numeric_limits<unsigned>::max();
        state_data.duration = 60 / 24;
        state_data.cyclic = true;
        state_data.priority = 1;
        state_data.speed_factor = 1.0f;
        state_data.time_progress = 0.5f;
        state_data.total_weight = 1.0f;

        return state_data;
    }
}
namespace DogDealerAnimating
{
    TEST_CLASS(BlenderWeightTest)
    {
    public:

        // Test weight assignment of throwing while walking diagonally
        // the upper body should throw with full priority
        // the lower body should walk with the following weights:
        //  -forwards: 0.25
        //  -forwards-right: 0.5
        //  -right: 0.25
        TEST_METHOD(TestDiagonalWalkAndThrowPriorities)
        {
            // ANIMATION INFO
            AnimationInfo animation_info;
            animation_info.offset = 0;
            animation_info.count = 2;

            std::vector<AnimationInfo> animation_infos;
            animation_infos.push_back(animation_info);

            // STATE INFOS
            StateInfo state_info_throw;
            state_info_throw.offset = 0;
            state_info_throw.count = 1;
            state_info_throw.bone_count = 18;
            state_info_throw.bone_weight_offset = 0;

            StateInfo state_info_walk;
            state_info_walk.offset = 1;
            state_info_walk.count = 9;
            state_info_walk.bone_count = 18;
            state_info_walk.bone_weight_offset = 18;

            std::vector<StateInfo> state_infos;
            state_infos.push_back(state_info_throw);
            state_infos.push_back(state_info_walk);

            // STATE DATAS
            StateData state_data_throw;
            state_data_throw.blender.index = std::numeric_limits<unsigned>::max();
            state_data_throw.duration = 60 / 24;
            state_data_throw.cyclic = false;
            state_data_throw.priority = 10;
            state_data_throw.speed_factor = 1.0f;
            state_data_throw.time_progress = 0.5f;
            state_data_throw.total_weight = 1.0f;

            StateData state_data_walk;
            state_data_walk.blender.index = 0;
            state_data_walk.duration = 60 / 24;
            state_data_walk.cyclic = true;
            state_data_walk.priority = 1;
            state_data_walk.speed_factor = 1.0f;
            state_data_walk.time_progress = 0.5f;
            state_data_walk.total_weight = 1.0f;

            std::vector<StateData> state_datas;
            state_datas.push_back(state_data_throw);
            state_datas.push_back(state_data_walk);

            // SEQUENCE WEIGHTS
            std::vector<float> sequence_weights;
            sequence_weights.push_back(1.0f); //throw

            for (auto i = 0u; i < 9; i++) sequence_weights.push_back(0.0f);
            sequence_weights[5] = 0.5f; // walk_forwards_right
            sequence_weights[1] = 0.25f; // walk_forwards
            sequence_weights[3] = 0.25f; // walk_right

            // SEQUENCE IDS
            std::vector<SequenceID> sequence_ids;
            for (auto i = 0u; i < 11; i++)
            {
                SequenceID sequence_id;
                sequence_id.index = i;
                sequence_ids.push_back(sequence_id);
            }

            // SEQUENCE BONE WEIGHTS
            auto sequence_bone_weights = std::vector<float>(126, 1.0f);

            // upper body blend mask for throw
            for (auto i = 12; i < 18; i++) sequence_bone_weights[i] = 0.0f;
            sequence_bone_weights[0] = 0.1f;

            // THINGS
            auto sequence_data_indices = std::vector<unsigned>(2);
            std::vector<SequenceID> nonzero_sequences;
            std::vector<float> sequence_time_progress;
            auto sequence_bone_data_indices = std::vector<unsigned>(2);
            std::vector<float> bone_weights;

            CalculateWeights(
                animation_infos,
                // state data
                state_infos, state_datas, sequence_weights, sequence_ids, sequence_bone_weights,
                // sequence output data
                sequence_data_indices, nonzero_sequences, sequence_time_progress,
                // sequence bone output data
                sequence_bone_data_indices, bone_weights);

            // Assert that throw state got full upper-body weight.
            // Exclude hip as it blends 0.1/0.9 with upper-body / lower-body
            for (auto i = 1; i < 12; i++) Assert::AreEqual(bone_weights[i], 1.0f);

            // Assert that the lower body bones of the two equal-weighted walking animations
            // received the same weight.
            // The bone sets of the forwards and right walking animations follow one another
            auto bone_set_forwards_start = 18;
            auto bone_set_right_start = 2 * 18;

            // Iterate over lower body bones (excluding hip)
            for (auto i = 12; i < 18; i++)
            {
                // Get absolute indices of bone weights for both sequences
                auto forwards_bone_index = bone_set_forwards_start + i;
                auto right_bone_index = bone_set_right_start + i;

                // Get weights for the same bone of the forwards and right sequences
                auto current_bone_weight_forwards = bone_weights[forwards_bone_index];
                auto current_bone_weight_right = bone_weights[right_bone_index];

                // Assert that both received the same weight
                Assert::AreEqual(current_bone_weight_forwards, current_bone_weight_right);
            }
        }
    };
}