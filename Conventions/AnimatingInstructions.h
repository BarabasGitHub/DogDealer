#pragma once
#include <vector>
#include <Conventions\EntityID.h>
#include <Conventions\AnimationTemplateID.h>
#include <Conventions\AnimatingBlendModes.h>

enum struct AnimationStateType { Idle, Turning, Motion, Throw, 
                                ReadyStrikeLeft, HoldStrikeLeft, StrikeLeft, 
                                ReadyStrikeRight, HoldStrikeRight, StrikeRight, 
                                ReadyStrikeUp, HoldStrikeUp, StrikeUp, 
                                ReadyStrikeDown, HoldStrikeDown, StrikeDown, 
                                BlockLeft, BlockRight, BlockUp, BlockDown,
                                TorsoTilting,
                                StrikeRebound, 
                                Flinch, Dying, Dead, Invalid };

struct AnimatingInstruction
{
	unsigned layer_index;
    AnimationTemplateID template_id;

    Animating::BlendMode blend_mode = Animating::BlendMode::Mixing;
};

struct EntityAnimatingInstructions
{
    std::vector<EntityID> entity_ids;
    std::vector<AnimatingInstruction> instructions;
};