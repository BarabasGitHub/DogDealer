#pragma once

#include "BodyID.h"

#include <Utilities\Range.h>
#include <Conventions\EntityID.h>

#include <vector>

namespace Physics
{
    struct BodyEntityMapping;

    void Add(
        EntityID entity, 
        BodyID body,
        BodyEntityMapping& self
        );

    void Add(
        EntityID entity, 
        Range<BodyID const *> bodies, 
        BodyEntityMapping& self
        );

    void Remove(
        Range<EntityID const *> entities, 
        BodyEntityMapping& self
        );

    void Remove(
        Range<BodyID const *> bodies,
        BodyEntityMapping& self
        );

    void RemoveInReverse(Range<BodyID const*> bodies, BodyEntityMapping & self);

    // bool IsValid(BodyID body, BodyEntityMapping const & self);
    
    // returns the number of bodies
    uint32_t BodyCount(
        EntityID entity, 
        BodyEntityMapping const & self
        );
    
    Range<BodyID const *> Bodies(
        EntityID entity, 
        BodyEntityMapping const & self
        );
    
    void AppendBodies(
        Range<EntityID const *> entities, 
        BodyEntityMapping const & self, 
        std::vector<BodyID> & bodies
        );
    
    // returns the entity for the body
    EntityID Entity(
        BodyID body, 
        BodyEntityMapping const & self
        );

    void BodyToElementToEntityToElement(
        Range<uint32_t const*> body_to_element, 
        BodyEntityMapping const & self, 
        std::vector<uint32_t> & entity_to_element
        );
}
