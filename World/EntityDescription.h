#pragma once

#include <Graphics\ResourceDescriptions.h>
#include <Physics\ResourceDescriptions.h>
#include <Animating\ResourceDescriptions.h>
#include <GameLogic\ResourceDescriptions.h>

#include <memory>
#include <vector>

struct EntityDescription
{
    std::vector<Graphics::RenderComponentDescription> render_component_desc;
    std::unique_ptr<Physics::ComponentDescription> physics_component_desc;
    std::unique_ptr<Animating::ComponentDescription> animating_component_desc;
    std::unique_ptr<Logic::MobileComponentDescription> logic_mobile_component_desc;
    std::unique_ptr<Logic::ItemComponentDescription> logic_item_component_desc;
    std::unique_ptr<Logic::DamageDealerComponentDescription> logic_damage_dealer_component_description;

    std::string name;
};