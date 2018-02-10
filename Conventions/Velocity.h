#pragma once

#include <Math\FloatTypes.h>
#include <Conventions\EntityID.h>
#include <vector>

#include <Utilities\Range.h>


typedef Math::Float3 Velocity;

struct IndexedVelocities{
	std::vector<Velocity> velocities;
    std::vector<Math::Float3> angular_velocity;
	std::vector<unsigned> indices;
};


Velocity GetOptionalVelocity( IndexedVelocities const & indexed_velocities, EntityID const entity_id, Velocity velocity = 0 /*default value*/ );
Math::Float3 GetOptionalAngularVelocity( IndexedVelocities const & indexed_velocities, EntityID const entity_id, Math::Float3 angular_velocity = 0 /*default value*/ );

// position in world coordinates, but relative to the position of the entity
Math::Float3 CalculateVelocityAtPosition( IndexedVelocities const & indexed_velocities, EntityID const entity_id, Math::Float3 position );

// gets the velocities of all entities in 'entities'
void GetEntityVelocities(Range<EntityID const *> entities, IndexedVelocities const & indexed_velocities, Range<Math::Float3 *> velocities);