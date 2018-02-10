#include "ProjectileContainer.h"

#include <Utilities\IndexedHelp.h>
#include <Utilities\VectorHelper.h>
#include <Utilities\ContainerHelpers.h>

namespace Logic{

    void ProjectileContainer::AddProjectile(EntityID const projectile_entity, EntityID const emitter_entity)
    {
        auto index = uint32_t(projectile_entities.size());

        projectile_entities.push_back(projectile_entity);
        emitter_entities.push_back(emitter_entity);

        AddIndexToIndices(entity_to_data, projectile_entity.index, index );
    }


    void ProjectileContainer::RemoveProjectile(EntityID const entity_id)
    {
        RemoveProjectiles(CreateRange(&entity_id, 1));
    }


    void ProjectileContainer::RemoveProjectiles(Range<EntityID const *> entities)
    {
        RemoveIDsWithSwapAndPrune(entities, entity_to_data, projectile_entities, emitter_entities);
    }
}