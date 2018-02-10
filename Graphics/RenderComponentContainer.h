#pragma once

#include "Structures.h"
#include "IDs.h"
#include <Math\FloatMatrixTypes.h>
#include <Conventions\EntityID.h>

#include <array>
#include <vector>

namespace Graphics
{
    // forward declarations

    struct RenderComponentContainer
    {
        std::vector<EntityID> entity_ids;
        std::vector<RenderComponent> components;
        std::vector<DisplayTechnique> display_techniques;
        std::vector<DisplayTechnique> shadow_display_techniques;
        std::vector<BoundingShapes::AxisAlignedBox> bounding_boxes;
        // should be bool, but vector<bool> is a turd
        std::vector<uint8_t> alpha_to_coverage;
        std::vector<std::array<VertexBufferID,2>> instance_transform_buffers;
        std::vector<Math::Float4x4> orientation_offsets;
    };


    void AddRenderComponent(
        EntityID entity_id,
        RenderComponent component,
        DisplayTechnique technique,
        DisplayTechnique shadow_technique,
        BoundingShapes::AxisAlignedBox bounding_box,
        bool alpha_to_coverage,
        std::array<VertexBufferID, 2> instance_buffers,
        Math::Float4x4 orientation_offset,
        RenderComponentContainer & self);

    void Append(RenderComponentContainer const & other, RenderComponentContainer & self);

    size_t ComponentCount(RenderComponentContainer const & self);

    void RemoveRenderComponents(EntityID id, RenderComponentContainer & self);
    void RemoveRenderComponent(uint32_t component_index, RenderComponentContainer & self);
    void Clear(RenderComponentContainer & self);

    // adds room for one more render component
    void Grow(RenderComponentContainer & self);
    void Shrink(size_t count, RenderComponentContainer & self);

    // checks if the container is still in a valid state, that is it thecks the sizes of all vectors
    bool IsInValidState( RenderComponentContainer const & container );
}
