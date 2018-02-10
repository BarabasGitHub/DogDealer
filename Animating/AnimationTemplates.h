#pragma once

#include <Conventions\AnimationTemplateID.h>

#include <vector>
namespace Animating
{
    struct AnimationTemplate
    {
        bool cyclic;
        float blend_time;
        float duration;

        // Optional, used for speed modifiers based on logic input
        ExternalParameterType external_parameter_type;
        float external_parameter_factor;

        // Parallel vectors of priorities and skeleton bone weights [one priority per bone set]
        std::vector<uint32_t> priorities;        
        std::vector<std::vector<float>> bone_masks;

        // This is a vector because of the animation blender stuff
        std::vector<SequenceID> sequence_ids;
        AnimationBlenderID blender_id;
    };

    struct AnimationTemplates
    {
        std::vector<AnimationTemplate> descriptions;

        AnimationTemplateID Add( AnimationTemplate description );
    };

    inline AnimationTemplateID AnimationTemplates::Add( AnimationTemplate description )
    {
        AnimationTemplateID id;
        id.index = AnimationTemplateID::index_t( descriptions.size() );
        id.generation = 0;

        descriptions.emplace_back( std::move( description ) );
        return id;
    }
}