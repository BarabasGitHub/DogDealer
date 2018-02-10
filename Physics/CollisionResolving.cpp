#include "CollisionResolving.h"
#include <Conventions\CollisionManifold.h>
#include <Conventions\EntityID.h>
#include <Conventions\Orientation.h>
#include <Math\FloatOperators.h>
#include <Math\MathFunctions.h>


namespace Physics
{
    void ResolveKinematicStatic( 
        Range<BodyPair const *> collision_bodies, 
        Range<Manifold const *> manifolds, 
        Range<uint32_t const *> entity_to_orientation, 
        Range<Orientation *> orientations 
        )
    {
        assert(Size(collision_bodies) == Size(manifolds));
        for( auto i = 0u; i < Size(collision_bodies); ++i )
        {
            auto & manifold = manifolds[i];
            auto & orientation = orientations[entity_to_orientation[collision_bodies[i].id1.index]];
            Math::Float3 movement = 0;
            for( auto j = 0u; j < manifold.contact_point_count; j++ )
            {
                movement = Max( movement, manifold.penetration_depths[j] * manifold.separation_axes[j] );
            }
            orientation.position += movement;
        }
    }


    void ResolveStaticKinematic( 
        Range<BodyPair const *> collision_bodies, 
        Range<Manifold const *> manifolds, 
        Range<uint32_t const *> entity_to_orientation, 
        Range<Orientation *> orientations 
        )
    {
        assert(Size(collision_bodies) == Size(manifolds));
        for( auto i = 0u; i < Size(collision_bodies); ++i )
        {
            auto & manifold = manifolds[i];
            auto & orientation = orientations[entity_to_orientation[collision_bodies[i].id2.index]];
            Math::Float3 movement = 0;
            for( auto j = 0u; j < manifold.contact_point_count; j++ )
            {
                movement = Max( movement, manifold.penetration_depths[j] * manifold.separation_axes[j] );
            }
            orientation.position += movement;
        }
    }


    void ResolveKinematicKinematic( 
        Range<BodyPair const *> collision_bodies, 
        Range<Manifold const *> manifolds, 
        Range<uint32_t const *> entity_to_orientation, 
        Range<Orientation *> orientations 
        )
    {
        assert(Size(collision_bodies) == Size(manifolds));
        for( auto i = 0u; i < Size(collision_bodies); ++i )
        {
            auto & manifold = manifolds[i];
            Math::Float3 movement = 0;
            for( auto j = 0u; j < manifold.contact_point_count; j++ )
            {
                movement = Max( movement, manifold.penetration_depths[j] * manifold.separation_axes[j] );
            }
            movement /= 2;
            auto entity_pair = collision_bodies[i];
            orientations[entity_to_orientation[entity_pair.id1.index]].position += movement;
            orientations[entity_to_orientation[entity_pair.id2.index]].position -= movement;
        }
    }
}
