#pragma once

#include "EntityTemplateID.h"
#include "Orientation.h"
#include "Velocity.h"

struct EntitySpawn
{
	EntityID entity_id;
    EntityTemplateID template_id;
    Orientation orientation;
    Velocity velocity;
};
