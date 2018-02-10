#pragma once

#include <Utilities\Handle.h>
#include <Utilities\HandleIDPair.h>

namespace Physics
{
    // so we have something unique to define a handle with
    struct Body;
    // define an entity id
    typedef Handle<Body> BodyID;
    typedef HandleIDPair<Body> BodyPair;

    // and define an invalid id constant, so we can check against it
    const BodyID c_invalid_body_id = { BodyID::index_t(-1), BodyID::generation_t(-1) };
}
