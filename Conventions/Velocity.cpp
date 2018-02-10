#include "Velocity.h"
#include <Math\MathFunctions.h>
#include <Utilities\VectorHelper.h>

using namespace Math;

Velocity GetOptionalVelocity( IndexedVelocities const & indexed_velocities, EntityID const entity_id, Velocity velocity /*default value*/ )
{

    auto velocity_index = GetOptional(indexed_velocities.indices, entity_id.index);
    velocity = GetOptional<Float3>(indexed_velocities.velocities, velocity_index, velocity);
    return velocity;
}


Math::Float3 GetOptionalAngularVelocity( IndexedVelocities const & indexed_velocities, EntityID const entity_id, Math::Float3 angular_velocity /*default value*/ )
{
    auto const velocity_index = GetOptional(indexed_velocities.indices, entity_id.index);
    angular_velocity = GetOptional<Float3>(indexed_velocities.angular_velocity, velocity_index, angular_velocity);
    return angular_velocity;
}


Math::Float3 CalculateVelocityAtPosition( IndexedVelocities const & indexed_velocities, EntityID const entity_id, Math::Float3 position )
{
    auto velocity = GetOptionalVelocity(indexed_velocities, entity_id, 0);
    auto angular_velocity = GetOptionalAngularVelocity(indexed_velocities, entity_id, 0);
    velocity += Cross(angular_velocity, position);
    return velocity;
}


void GetEntityVelocities(Range<EntityID const *> entities, IndexedVelocities const & indexed_velocities, Range<Math::Float3 *> velocities)
{
	assert(Size(entities) == Size(velocities));
	auto count = Size(entities);
	for (auto i = 0u; i < count; ++i)
	{
		auto& entity_id = entities[i];		
		auto entity_velocity = GetOptionalVelocity(indexed_velocities, entity_id, Math::Float3(0,0,0));

		velocities[i] = entity_velocity;
	}
}