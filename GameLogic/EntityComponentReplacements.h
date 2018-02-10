#pragma once
#include <vector>
#include <Conventions\EntityID.h>
#include <Conventions\EntityTemplateID.h>


struct EntityComponentReplacements
{
    std::vector<EntityID> entity_ids;
    std::vector<EntityTemplateID> template_ids;
};