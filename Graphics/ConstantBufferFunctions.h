#pragma once

#include "DeviceContext.h"

#include <Utilities\Range.h>
#include <Conventions\EntityID.h>
#include <Math\ForwardDeclarations.h>


namespace Graphics
{
    struct Device;
    struct ConstantBufferContainer;
    struct RenderComponent;

    void AddObjectConstantBuffer(Device& device, ConstantBufferContainer& container, RenderComponent& component);
    void AddBoneConstantBuffer(Device& device, ConstantBufferContainer& container, RenderComponent& component);

    void FillObjectBuffers(
        Range<RenderComponent const *> components,
        Range<Math::Float4x4 const *> transforms,
        ConstantBufferContainer const & constant_buffer_container,
        DeviceContext& device_context
        );

    void FillObjectBuffers(
        Range<uint32_t const*> component_indices,
        Range<RenderComponent const *> components,
        Range<Math::Float4x4 const *> transforms,
        ConstantBufferContainer const & constant_buffer_container,
        DeviceContext& device_context
        );

    void FillBoneBuffers(
        Range<uint32_t const *> component_indices,
        Range<RenderComponent const *> components,
        Range<EntityID const *> entities,
        Range<uint32_t const *> entity_to_pose,
        Range<uint32_t const *> pose_offsets,
        Range<Math::Float4x4 const *> bone_transforms,
        ConstantBufferContainer const & constant_buffer_container,
        DeviceContext& device_context
        );
    
    void FillBoneBuffers(
        Range<RenderComponent const *> components,
        Range<EntityID const *> entities,
        Range<uint32_t const *> entity_to_pose,
        Range<uint32_t const *> pose_offsets,
        Range<Math::Float4x4 const *> bone_transforms,
        ConstantBufferContainer const & constant_buffer_container,
        DeviceContext& device_context
        );
}
