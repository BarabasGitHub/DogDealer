#pragma once

#include <Conventions\AnimatingInstructions.h>
#include <Conventions\AnimationTemplateID.h>
#include <Conventions\AnimatingExternalParameterType.h>
#include <Math\FloatTypes.h>

#include <string>
#include <vector>
#include <map>

namespace Animating{

    struct AnimationBlenderNodeDescription
    {
        Math::Float2 center;
        float radius;
    };

    struct CircleBlenderDescription
    {
        ExternalParameterType parameter_type;
        std::vector<AnimationBlenderNodeDescription> nodes;
    };

    struct AnimationDescription
    {
        bool cyclic;
        float blend_time;
        float duration;

        // Optional, used for speed modifiers based on logic input
        ExternalParameterType external_parameter_type = ExternalParameterType::None;
        float external_parameter_factor = 1.0f;

        // Each mask of bone weights has one priority.
        std::vector<uint32_t> priorities;
        std::vector<std::vector<float>> bone_masks;

        // this is a vector because of the animation blender stuff
        std::vector<std::string> sequences;
        CircleBlenderDescription circle_blender;
    };

    struct ComponentDescription
    {
        std::string skeleton;
        std::map<AnimationStateType, AnimationDescription> animations;
    };
}