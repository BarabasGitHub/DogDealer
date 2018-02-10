#include "BodyEntityMappingFunctions.h"
#include "BodyEntityMapping.h"

#include <Utilities\StdVectorFunctions.h>
#include <Utilities\VectorHelper.h>
#include <Utilities\IndexedHelp.h>
#include <Math\VectorAlgorithms.h>

void Physics::Add(
    EntityID entity,
    BodyID body,
    BodyEntityMapping& self
    )
{
    // store what entity the body belongs to
    EnsureElementExists(body.index, c_invalid_entity_id, self.entity_ids) = entity;

    // store what bodies belong to the entity
    auto body_end_index = EnsureElementExists(entity.index + 1, Last(self.entity_to_bodies), self.entity_to_bodies);
    Math::Add(1u, CreateRange(self.entity_to_bodies, entity.index + 1, Size(self.entity_to_bodies)));
    Insert(body, body_end_index, self.entity_bodies);
}


void Physics::Add(
    EntityID entity,
    Range<BodyID const *> bodies,
    BodyEntityMapping& self
    )
{
    // temporarily like this, I'm sure it can be done more efficient
    for(auto body : bodies)
    {
        Add(entity, body, self);
    }
}


void Physics::Remove(
    Range<EntityID const *> entities,
    BodyEntityMapping& self
    )
{
    for(auto entity : entities)
    {
        assert(entity != c_invalid_entity_id);
        auto body_start_index = GetOptional(self.entity_to_bodies, entity.index, Last(self.entity_to_bodies));
        auto body_end_index = GetOptional(self.entity_to_bodies, entity.index + 1, Last(self.entity_to_bodies));
        for(auto body : CreateRange(self.entity_bodies, body_start_index, body_end_index))
        {
            self.entity_ids[body.index] = c_invalid_entity_id;
        }
        self.entity_bodies.erase(begin(self.entity_bodies) + body_start_index, begin(self.entity_bodies) + body_end_index);
        // it is start - end on purpose, because I want to subtract end - start, so I add start - end (as uint32_t).
        Math::Add(uint32_t(body_start_index - body_end_index), CreateRange(self.entity_to_bodies, entity.index + 1, Size(self.entity_to_bodies)));
    }
}


void Physics::Remove(
    Range<BodyID const *> bodies,
    BodyEntityMapping& self
    )
{
    assert(begin(bodies) > &self.entity_bodies.back() || end(bodies) < &self.entity_bodies.front());
    for(auto body : bodies)
    {
        auto entity = self.entity_ids[body.index];
        self.entity_ids[body.index] = c_invalid_entity_id;
        auto body_start_index = self.entity_to_bodies[entity.index];
        auto body_end_index = self.entity_to_bodies[entity.index + 1];
        self.entity_bodies.erase(std::find(begin(self.entity_bodies) + body_start_index, begin(self.entity_bodies) + body_end_index, body));
        // subtract 1
        Math::Add(uint32_t(-1), CreateRange(self.entity_to_bodies, entity.index + 1, Size(self.entity_to_bodies)));
    }
}


void Physics::RemoveInReverse(
    Range<BodyID const *> bodies,
    BodyEntityMapping& self
    )
{
    auto count = Size(bodies);
    for (auto i = count; i > 0; )
    {
        i -= 1;
        auto body = bodies[i];
        auto entity = self.entity_ids[body.index];
        self.entity_ids[body.index] = c_invalid_entity_id;
        auto body_start_index = self.entity_to_bodies[entity.index];
        auto body_end_index = self.entity_to_bodies[entity.index + 1];
        self.entity_bodies.erase(std::find(begin(self.entity_bodies) + body_start_index, begin(self.entity_bodies) + body_end_index, body));
        // subtract 1
        Math::Add(uint32_t(-1), CreateRange(self.entity_to_bodies, entity.index + 1, Size(self.entity_to_bodies)));
    }
}



// bool Physics::IsValid(BodyID body, BodyEntityMapping const & self)
// {

// }

// returns the number of bodies
uint32_t Physics::BodyCount(
    EntityID entity,
    BodyEntityMapping const & self
    )
{
    assert(entity != c_invalid_entity_id);
    auto body_start_index = GetOptional(self.entity_to_bodies, entity.index, Last(self.entity_to_bodies));
    auto body_end_index = GetOptional(self.entity_to_bodies, entity.index + 1, Last(self.entity_to_bodies));
    return uint32_t(body_end_index - body_start_index);
}


Range<Physics::BodyID const *> Physics::Bodies(
    EntityID entity,
    BodyEntityMapping const & self
    )
{
    assert(entity != c_invalid_entity_id);
    auto body_start_index = GetOptional(self.entity_to_bodies, entity.index, Last(self.entity_to_bodies));
    auto body_end_index = GetOptional(self.entity_to_bodies, entity.index + 1, Last(self.entity_to_bodies));
    return CreateRange(self.entity_bodies, body_start_index, body_end_index);
}


EntityID Physics::Entity(
    BodyID body,
    BodyEntityMapping const & self
    )
{
    return GetOptional(self.entity_ids, body.index, c_invalid_entity_id);
}


void Physics::AppendBodies(
    Range<EntityID const *> entities,
    BodyEntityMapping const & self,
    std::vector<BodyID> & bodies
    )
{
    for(auto e : entities)
    {
        Append(bodies, Bodies(e, self));
    }
}


void Physics::BodyToElementToEntityToElement(
    Range<uint32_t const*> body_to_element,
    BodyEntityMapping const & self,
    std::vector<uint32_t> & entity_to_element
    )
{
    auto size = Size(body_to_element);
    for(size_t i = 0; i < size; i++)
    {
        auto entity_id = GetOptional(self.entity_ids, i, c_invalid_entity_id);
        if(entity_id != c_invalid_entity_id)
        {
            auto bodies = Bodies(entity_id, self);
            if(!IsEmpty(bodies))
            {
                auto index = First(bodies).index;
                AddIndexToIndices(entity_to_element, entity_id.index, body_to_element[index]);
            }
        }
    }
}
