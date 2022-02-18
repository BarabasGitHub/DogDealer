#include "CollisionEvent.h"

#include <Physics\ManifoldFunctions.h>

#include <Math\FloatOperators.h>

#include <Utilities\VectorHelper.h>
#include <Utilities\IntegerIterator.h>
#include <Utilities\Memory.h>

#include <memory>

using namespace Physics;

void Clear(CollisionEvents & ce)
{
    ce.bodies.clear();
    ce.manifolds.clear();
    ce.relative_positions.clear();
}


uint32_t Size(CollisionEvents const & ce)
{
    assert(ce.bodies.size() == ce.manifolds.size());
    assert(ce.bodies.size() == ce.relative_positions.size());
    return uint32_t(ce.bodies.size());
}


void Resize(size_t size, CollisionEvents & ce)
{
    ce.bodies.resize(size);
    ce.manifolds.resize(size);
    ce.relative_positions.resize(size);
}


void Reorder(Range<uint32_t const *> indices, CollisionEvents & ce)
{
    auto size = ce.bodies.size();
    // allocate enough memory to use as temporary buffer for all three vectors
    constexpr auto max_size = std::max(std::max(sizeof(BodyPair), sizeof(Math::Float3)), std::max(sizeof(Manifold), sizeof(uint8_t)));
    auto data_buffer = std::make_unique<uint8_t[]>(size * max_size);
    // copy and reorder each vector
    // bodies
    auto entity_buffer = static_cast<BodyPair*>(static_cast<void*>(data_buffer.get()));
    Copy(ce.bodies.data(), size, entity_buffer);
    ::Reorder<BodyPair>(CreateRange(entity_buffer, size), indices, ce.bodies);
    // positions
    auto position_buffer = static_cast<Math::Float3*>(static_cast<void*>(data_buffer.get()));
    Copy(ce.relative_positions.data(), size, position_buffer);
    ::Reorder<Math::Float3>(CreateRange(position_buffer, size), indices, ce.relative_positions);
    // manifolds
    auto manifold_buffer = static_cast<Manifold*>(static_cast<void*>(data_buffer.get()));
    Copy(ce.manifolds.data(), size, manifold_buffer);
    ::Reorder<Manifold>(CreateRange(manifold_buffer, size), indices, ce.manifolds);
}


void Append(CollisionEvents & a, CollisionEvents const & b)
{
    ::Append(a.bodies, b.bodies);
    ::Append(a.manifolds, b.manifolds);
    ::Append(a.relative_positions, b.relative_positions);
}


void Flip( BodyPair & bodies, Math::Float3 & relative_position, Manifold & manifold )
{
    using std::swap;
    swap( bodies.id1, bodies.id2 );
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


void PutLowerOrderBodyFirst( Range<uint32_t const *> body_to_order, CollisionEvents & events )
{
    FlipIf(events, [body_to_order](auto const & bodies)
    {
        return body_to_order[bodies.id2.index] < body_to_order[bodies.id1.index];
    });
}


void PutHigherOrderBodyFirst( Range<uint32_t const *> body_to_order, CollisionEvents & events )
{
    FlipIf(events, [body_to_order](auto const & bodies)
    {
        return body_to_order[bodies.id1.index] < body_to_order[bodies.id2.index];
    });
}
