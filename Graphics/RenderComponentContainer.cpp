#include "RenderComponentContainer.h"

#include <Utilities\VectorHelper.h>
#include <Utilities\StdVectorFunctions.h>

using namespace Graphics;


// void Graphics::AddRenderComponents(
//     EntityID entity_id,
//     RenderComponent component,
//     DisplayTechnique technique,
//     DisplayTechnique shadow_technique,
//     BoundingShapes::AxisAlignedBox bounding_box,
//     bool alpha_to_coverage,
//     std::array<VertexBufferID, 2> instance_buffers,
//     Math::Float4x4 orientation_offset,
//     RenderComponentContainer & self)
// {
//     self.entity_ids.emplace_back( entity_id );
//     self.components.emplace_back( std::move( component ) );
//     self.display_techniques.emplace_back( technique );
//     self.shadow_display_techniques.emplace_back( shadow_technique );
//     self.bounding_boxes.emplace_back( bounding_box );
//     self.alpha_to_coverage.emplace_back( alpha_to_coverage );
//     self.instance_transform_buffers.push_back( instance_buffers );
//     self.orientation_offsets.emplace_back(orientation_offset);

//     assert( IsInValidState( self ) );
// }


void Graphics::AddRenderComponent(
    EntityID entity_id,
    RenderComponent component,
    DisplayTechnique technique,
    DisplayTechnique shadow_technique,
    BoundingShapes::AxisAlignedBox bounding_box,
    bool alpha_to_coverage,
    std::array<VertexBufferID, 2> instance_buffers,
    Math::Float4x4 orientation_offset,
    RenderComponentContainer & self)
{
    self.entity_ids.emplace_back( entity_id );
    self.components.emplace_back( std::move( component ) );
    self.display_techniques.emplace_back( technique );
    self.shadow_display_techniques.emplace_back( shadow_technique );
    self.bounding_boxes.emplace_back( bounding_box );
    self.alpha_to_coverage.emplace_back( alpha_to_coverage );
    self.instance_transform_buffers.push_back( instance_buffers );
    self.orientation_offsets.emplace_back(orientation_offset);

    assert( IsInValidState( self ) );
}


void Graphics::Append(RenderComponentContainer const & other, RenderComponentContainer & self)
{
    ::Append(self.entity_ids, other.entity_ids);
    ::Append(self.components, other.components);
    ::Append(self.display_techniques, other.display_techniques);
    ::Append(self.shadow_display_techniques, other.shadow_display_techniques);
    ::Append(self.bounding_boxes, other.bounding_boxes);
    ::Append(self.alpha_to_coverage, other.alpha_to_coverage);
    ::Append(self.instance_transform_buffers, other.instance_transform_buffers);
    ::Append(self.orientation_offsets, other.orientation_offsets);
}


size_t Graphics::ComponentCount(RenderComponentContainer const & self)
{
    assert(IsInValidState(self));
    return Size(self.entity_ids);
}


void Graphics::RemoveRenderComponents(EntityID id, RenderComponentContainer & self)
{
    auto i = uint32_t(self.entity_ids.size());
    while( i > 0 )
    {
        --i;
        if(self.entity_ids[i] == id)
        {
            RemoveRenderComponent(i, self);
        }
    }
}


void Graphics::RemoveRenderComponent(uint32_t index, RenderComponentContainer & self)
{
    SwapAndPrune(
        index,
        self.entity_ids,
        self.components,
        self.display_techniques,
        self.shadow_display_techniques,
        self.bounding_boxes,
        self.alpha_to_coverage,
        self.instance_transform_buffers,
        self.orientation_offsets);
    assert( IsInValidState( self ) );
}


void Graphics::Grow(RenderComponentContainer & self)
{
    self.entity_ids.emplace_back();
    self.components.emplace_back();
    self.display_techniques.emplace_back();
    self.shadow_display_techniques.emplace_back();
    self.bounding_boxes.emplace_back();
    self.alpha_to_coverage.emplace_back();
    self.instance_transform_buffers.emplace_back();
    self.orientation_offsets.emplace_back();
    assert( IsInValidState( self ) );
}


void Graphics::Shrink(size_t count, RenderComponentContainer & self)
{
    assert(IsInValidState(self));
    ::Shrink(self.entity_ids, count);
    ::Shrink(self.components, count);
    ::Shrink(self.display_techniques, count);
    ::Shrink(self.shadow_display_techniques, count);
    ::Shrink(self.bounding_boxes, count);
    ::Shrink(self.alpha_to_coverage, count);
    ::Shrink(self.instance_transform_buffers, count);
    ::Shrink(self.orientation_offsets, count);
}


bool Graphics::IsInValidState( RenderComponentContainer const & container )
{
    auto size = container.entity_ids.size();
    return size == container.components.size() &&
        size == container.display_techniques.size() &&
        size == container.shadow_display_techniques.size() &&
        size == container.bounding_boxes.size() &&
        size == container.alpha_to_coverage.size() &&
        size == container.instance_transform_buffers.size() &&
        size == container.orientation_offsets.size();
}


void Graphics::Clear(RenderComponentContainer & self)
{
    ::Clear(self.entity_ids);
    ::Clear(self.components);
    ::Clear(self.display_techniques);
    ::Clear(self.shadow_display_techniques);
    ::Clear(self.bounding_boxes);
    ::Clear(self.alpha_to_coverage);
    ::Clear(self.instance_transform_buffers);
    ::Clear(self.orientation_offsets);
}
