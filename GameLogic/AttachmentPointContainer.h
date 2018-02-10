#pragma once
#include <Conventions\EntityID.h>
#include <Conventions\SkeletonAttachmentPoint.h>

namespace Logic
{
    struct AttachmentPointContainer
    {
        // Inventory owners
        std::vector<EntityID> entity_ids;
        std::vector<unsigned> entity_to_data;

        // Parallel to above, for now only a single item 
        // and attachment point per Entity which is assumed to be equipped
        std::vector<AttachmentPoint> attachment_points;

        void AddEntity(EntityID const entity_id, AttachmentPoint const attachment_point);

		// TODO: Add remove function
    };

	
}
