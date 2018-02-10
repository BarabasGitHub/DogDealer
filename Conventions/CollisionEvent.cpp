#include "CollisionEvent.h"

#include <Physics\ManifoldFunctions.h>

#include <Math\FloatOperators.h>

#include <Utilities\VectorHelper.h>
#include <Utilities\IntegerIterator.h>
#include <Utilities\Memory.h>

#include <memory>


void Clear(CollisionEvents & ce)
{
    ce.entities.clear();
    ce.manifolds.clear();
    ce.relative_positions.clear();
}


uint32_t Size(CollisionEvents const & ce)
{
    assert(ce.entities.size() == ce.manifolds.size());
    assert(ce.entities.size() == ce.relative_positions.size());
    return uint32_t(ce.entities.size());
}


void Resize(size_t size, CollisionEvents & ce)
{
    ce.entities.resize(size);
    ce.manifolds.resize(size);
    ce.relative_positions.resize(size);
}


void Reorder(Range<uint32_t const *> indices, CollisionEvents & ce)
{
    auto size = ce.entities.size();
    // allocate enough memory to use as temporary buffer for all three vectors
    auto data_buffer = std::make_unique<uint8_t[]>(size * std::max({sizeof(EntityPair), sizeof(Math::Float3), sizeof(Manifold), sizeof(uint8_t)}));
    // copy and reorder each vector
    // entities
    auto entity_buffer = static_cast<EntityPair*>(static_cast<void*>(data_buffer.get()));
    Copy(ce.entities.data(), size, entity_buffer);
    Reorder<EntityPair>(CreateRange(entity_buffer, size), indices, ce.entities);
    // positions
    auto position_buffer = static_cast<Math::Float3*>(static_cast<void*>(data_buffer.get()));
    Copy(ce.relative_positions.data(), size, position_buffer);
    Reorder<Math::Float3>(CreateRange(position_buffer, size), indices, ce.relative_positions);
    // manifolds
    auto manifold_buffer = static_cast<Manifold*>(static_cast<void*>(data_buffer.get()));
    Copy(ce.manifolds.data(), size, manifold_buffer);
    Reorder<Manifold>(CreateRange(manifold_buffer, size), indices, ce.manifolds);
}


void Append(CollisionEvents & a, CollisionEvents const & b)
{
    Append(a.entities, b.entities);
    Append(a.manifolds, b.manifolds);
    Append(a.relative_positions, b.relative_positions);
}


void Flip( EntityPair & entities, Math::Float3 & relative_position, Manifold & manifold )
{
    using std::swap;
    swap( entities.id1, entities.id2 );
    relative_position = -relative_position;
    for( auto & axis : manifold.separation_axes )
    {
        axis = -axis;
    }
    for( auto & position : manifold.positions )
    {
        // plus because already negated the relative position
        position += relative_position;
    }
}


void PutLowerOrderEntityFirst( Range<uint32_t const *> entity_to_order, CollisionEvents & events )
{
    FlipIf(events, [entity_to_order](EntityPair const & entities)
    {
        return entity_to_order[entities.id2.index] < entity_to_order[entities.id1.index];
    });
}


void PutHigherOrderEntityFirst( Range<uint32_t const *> entity_to_order, CollisionEvents & events )
{
    FlipIf(events, [entity_to_order](EntityPair const & entities)
    {
        return entity_to_order[entities.id1.index] < entity_to_order[entities.id2.index];
    });
}