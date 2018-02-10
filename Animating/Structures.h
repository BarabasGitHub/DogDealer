#pragma once

#include <Conventions\EntityID.h> // for EntityID
#include <Conventions\PoseInfo.h>
#include <Conventions\AnimatingExternalParameterType.h>
#include <Conventions\AnimatingBlendModes.h>

#include <map>
#include <cstdint>

namespace Animating{

	// Offset and count for AnimationStateInfos
	struct AnimationInfo{
		unsigned offset;
		unsigned count;
	};

	// Index and orientation of a bone
    typedef Orientation BoneState;

	struct SkeletonInfo{
		unsigned bone_count;
		unsigned bone_offset;
	};

	// Helper for position and length in BoneStateContainer
	struct SequenceInfo{
		unsigned frame_offset;
		unsigned frame_count;
	};
	typedef Handle<SequenceInfo> SequenceID;

	// Used to determine the influence of a states sequences
	struct AnimationBlenderInfo{
        unsigned node_count;
        unsigned node_offset;

        ExternalParameterType parameter_type = ExternalParameterType::HorizontalMovement;
	};
	typedef Handle<AnimationBlenderInfo> AnimationBlenderID;

    struct CircleBlenderNode
    {
        Math::Float2 center;
        float radius;
    };

    struct StateInfo{
        // Offset and count of played sequences
        unsigned offset;
        unsigned count;

        // Offset and count of weight multipliers for individual bones
        unsigned bone_weight_offset;
        unsigned bone_count;
    };

    struct StateLogic{

        float target_weight;
        float blend_time = 0.0f;

        uint32_t layer_index = 0;
    };

    struct StateData{

        AnimationBlenderID blender;
        BlendMode blend_mode = BlendMode::Mixing;

        float total_weight = 0.0f;

        float time_progress = 0.0f;
        float speed_factor = 1.0f;
        // the time it takes to play the state sequences once from the beginning till end (or one cycle)
        float duration;

        uint32_t priority = 0;
        bool cyclic = true;

        // Optional, used for speed modifiers based on logic input
        ExternalParameterType external_parameter_type;
        float external_parameter_factor;
    };

    inline bool operator==(StateInfo const & a, StateInfo const & b)
    {
        return
            a.count == b.count &&
            a.offset == b.offset &&
            a.bone_count == b.bone_count &&
            a.bone_weight_offset == b.bone_weight_offset;
    }

    inline bool operator==(StateLogic const & a, StateLogic const & b)
    {
        return

            a.blend_time == b.blend_time &&
            a.layer_index == b.layer_index &&
            a.target_weight == b.target_weight;
    }

    inline bool operator==(StateData const & a, StateData const & b)
    {
        return
            a.blender == b.blender &&
            a.duration == b.duration &&
            a.cyclic == b.cyclic &&
            a.priority == b.priority &&
            a.speed_factor == b.speed_factor &&
            a.time_progress == b.time_progress &&
            a.total_weight == b.total_weight;
    }
}
