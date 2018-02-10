#pragma once

#include <Conventions\EntityID.h>
#include <Conventions\AnimationTemplateID.h>
#include <Conventions\AnimatingInstructions.h>

#include <Utilities\Range.h>

#include <map>

namespace Logic
{
    struct EntityAnimations
    {

        // map for each animation type, holding the entity and animation template
        std::map<AnimationStateType, std::map<EntityID, AnimationTemplateID>> animation_template_ids;
    };

    // set the animation templates in the entity map to the animations map, overwriting any existing template ids
    void SetEntityAnimations( std::map<AnimationStateType, std::map<EntityID, AnimationTemplateID>> & animations, EntityID const entity_id, std::map<AnimationStateType, AnimationTemplateID> const & map );

    // adds the animation for each entity to the instructions with a default blend mode of BlendMode::Mixing
    void AddAnimations(Range<EntityID const *> entities, std::map<EntityID, AnimationTemplateID> const & animations, uint32_t layer, EntityAnimatingInstructions & instructions);
    // adds the animation for each entity to the instructions
    void AddAnimations(Range<EntityID const *> entities, std::map<EntityID, AnimationTemplateID> const & animations, uint32_t layer, Animating::BlendMode blend_mode, EntityAnimatingInstructions & instructions);
    // adds the animation of type animation_type for each entity to the instructions with a default blend mode of BlendMode::Mixing
    void AddAnimations(Range<EntityID const *> entities, std::map<AnimationStateType, std::map<EntityID, AnimationTemplateID>> const & animations, AnimationStateType animation_type, uint32_t layer, EntityAnimatingInstructions & instructions);
    // adds the animation of type animation_type for each entity to the instructions
    void AddAnimations(Range<EntityID const *> entities, std::map<AnimationStateType, std::map<EntityID, AnimationTemplateID>> const & animations, AnimationStateType animation_type, uint32_t layer, Animating::BlendMode const blend_mode, EntityAnimatingInstructions & instructions);


    void RemoveEntityAnimations( EntityID const & entity_id, EntityAnimations& entity_animations );
}


