#pragma once

#include "EntityDescription.h"

#include <Conventions\EntityTemplateID.h>

#include <vector>

struct EntityTemplates
{
    std::vector<EntityDescription> descriptions;

    EntityTemplateID Add( EntityDescription description );
};

inline EntityTemplateID EntityTemplates::Add( EntityDescription description )
{
    EntityTemplateID id;
    id.index = EntityTemplateID::index_t( descriptions.size() );
    id.generation = 0;

    descriptions.emplace_back( std::move( description ) );
    return id;
}