#include "EntityAnimations.h"

namespace Logic
{
    void SetEntityAnimations( std::map<AnimationStateType, std::map<EntityID, AnimationTemplateID>> & animations, EntityID const entity_id, std::map<AnimationStateType, AnimationTemplateID> const & map )
    {
        for( auto entry : map )
        {
            // [state type][entity id] = template id
            animations[entry.first][entity_id] = entry.second;
        }
    }


    void AddAnimations( Range<EntityID const *> entities,
                        std::map<EntityID, AnimationTemplateID> const & animations,
                        uint32_t layer,
                        EntityAnimatingInstructions & instructions)
    {
        AddAnimations(entities, animations, layer, Animating::BlendMode::Mixing, instructions);
    }


    void AddAnimations( Range<EntityID const *> entities,
                        std::map<EntityID, AnimationTemplateID> const & animations,
                        uint32_t layer,
                        Animating::BlendMode const blend_mode,
                        EntityAnimatingInstructions & instructions)
    {
        instructions.entity_ids.insert( end( instructions.entity_ids ), begin( entities ), end( entities ) );
        for( auto entity : entities )
        {
            auto template_id = animations.at( entity );
            AnimatingInstruction instruction;
            instruction.layer_index = layer;
            instruction.template_id = template_id;
            instruction.blend_mode = blend_mode;
            instructions.instructions.emplace_back(instruction);
        }
    }


    void AddAnimations(
        Range<EntityID const *> entities,
        std::map<AnimationStateType, std::map<EntityID, AnimationTemplateID>> const & animations,
        AnimationStateType animation_type,
        uint32_t layer,
        EntityAnimatingInstructions & instructions
        )
    {
        if( !IsEmpty(entities) )
        {
            AddAnimations(entities, animations.at(animation_type), layer, instructions);
        }
    }


    void AddAnimations(
        Range<EntityID const *> entities,
        std::map<AnimationStateType, std::map<EntityID, AnimationTemplateID>> const & animations,
        AnimationStateType animation_type,
        uint32_t layer,
        Animating::BlendMode const blend_mode,
        EntityAnimatingInstructions & instructions
        )
    {
        if( !IsEmpty(entities))
        {
            AddAnimations(entities, animations.at(animation_type), layer, blend_mode, instructions);
        }
    }


    // Removes animations of all types for the given entity
    void RemoveEntityAnimations( EntityID const & entity_id, EntityAnimations& entity_animations )
    {
        // Iterate over all animation state types
        for( auto & state_type_map : entity_animations.animation_template_ids )
        {
            // If type contains entry for entity, remove it
            auto & animation_map = state_type_map.second;
            animation_map.erase( entity_id );
        }
    }
}