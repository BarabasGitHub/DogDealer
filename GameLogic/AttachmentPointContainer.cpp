#include "AttachmentPointContainer.h"

#include <Utilities/IndexedHelp.h>
#include <Utilities/VectorHelper.h>

namespace Logic
{
    void AttachmentPointContainer::AddEntity(EntityID const entity_id, AttachmentPoint const attachment_point)
    {
        auto const index = unsigned( entity_ids.size() );
        AddIndexToIndices( entity_to_data, entity_id.index, index );

        attachment_points.push_back(attachment_point);
        entity_ids.push_back(entity_id);
    }
}
