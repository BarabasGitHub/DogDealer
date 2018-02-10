#pragma once

#include <Utilities\Handle.h>
#include <Utilities\HandleIDPair.h>

// so we have something unique to define a handle with
struct Entity;
// define an entity id
typedef Handle<Entity> EntityID;
typedef HandleIDPair<Entity> EntityPair;

// and define an invalid id constant, so we can check against it
const EntityID c_invalid_entity_id = { EntityID::index_t(-1), EntityID::generation_t(-1) };
