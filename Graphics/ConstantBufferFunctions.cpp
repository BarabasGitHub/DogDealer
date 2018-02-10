#include "ConstantBufferFunctions.h"

#include "Device.h"
#include "ConstantBufferContainer.h"
#include "RenderComponent.h"
#include "ConstantBuffers.h"
#include "ConstantBufferTypeAndIDFunctions.h"
#include "FillConstantBuffer.h"

#include <Utilities\IntegerRange.h>
#include <Math\FloatMatrixTypes.h>

using namespace Graphics;

void Graphics::AddObjectConstantBuffer(Device& device, ConstantBufferContainer& container, RenderComponent& component)
{
    auto constant_buffer_size = uint32_t(sizeof(ConstantBuffers::Object));
    auto object_buffer = device.CreateConstantBuffer(constant_buffer_size);
    auto object_buffer_id = container.Add(object_buffer);

    component.buffers.emplace_back(ConstantBufferType::Object, object_buffer_id);
}


void Graphics::AddBoneConstantBuffer(Device& device, ConstantBufferContainer& container, RenderComponent& component)
{
    auto constant_buffer_size = uint32_t(sizeof(ConstantBuffers::Bones));
    auto bones_buffer = device.CreateConstantBuffer(constant_buffer_size);
    auto bones_buffer_id = container.Add(bones_buffer);
    component.buffers.emplace_back(ConstantBufferType::Bones, bones_buffer_id);
}


namespace
{
    template<typename IteratorType>
    void FillObjectBuffersImpl(
        Range<IteratorType> component_indices,
        Range<RenderComponent const *> components,
        Range<Math::Float4x4 const *> transforms,
        ConstantBufferContainer const & constant_buffer_container,
        DeviceContext& device_context)
    {
        assert(Size(components) == Size(transforms));
        for(auto i : component_indices)
        {
            auto id = GetBufferID(components[i].buffers, ConstantBufferType::Object);
            if(id != c_invalid_constant_buffer_id)
            {
                FillConstantBuffer<ConstantBuffers::Object>(
                    constant_buffer_container.GetBuffer(id),
                    device_context, [=](ConstantBuffers::Object* buffer)
                {
                    buffer->transform_matrix = transforms[i];
                    // buffer->base_color = { 0.3f, 0.15f, 0.1f, 1 };
                    buffer->base_color = {1.f, 1.f, 1.f, 1};
                });
            }
        }
    }
}


void Graphics::FillObjectBuffers(
    Range<RenderComponent const *> components,
    Range<Math::Float4x4 const *> transforms,
    ConstantBufferContainer const & constant_buffer_container,
    DeviceContext& device_context)
{
    FillObjectBuffersImpl(CreateIntegerRange(Size(components)), components, transforms, constant_buffer_container, device_context);
}


void Graphics::FillObjectBuffers(
    Range<uint32_t const*> component_indices,
    Range<RenderComponent const *> components,
    Range<Math::Float4x4 const *> transforms,
    ConstantBufferContainer const & constant_buffer_container,
    DeviceContext& device_context)
{
    FillObjectBuffersImpl(component_indices, components, transforms, constant_buffer_container, device_context);
}


namespace
{
    template<typename IteratorType>
    void FillBoneBuffersImpl(
        Range<IteratorType> component_indices,
        Range<RenderComponent const *> components,
        Range<EntityID const *> entities,
        Range<uint32_t const *> entity_to_pose,
        Range<uint32_t const *> pose_offsets,
        Range<Math::Float4x4 const *> bone_transforms,
        ConstantBufferContainer const & constant_buffer_container,
        DeviceContext& device_context
        )
    {
        assert(Size(components) == Size(entities));
        for(auto i : component_indices)
        {
            auto id = GetBufferID(components[i].buffers, ConstantBufferType::Bones);
            if(id != c_invalid_constant_buffer_id)
            {
                auto const entity_index = entities[i].index;
                auto const pose_index = entity_to_pose[entity_index];
                auto const pose_offset = pose_offsets[pose_index];
                auto const bone_count = pose_offsets[pose_index + 1] - pose_offset;
                auto const bone_transform = begin(bone_transforms) + pose_offset;

                FillConstantBuffer<ConstantBuffers::Bones>(
                    constant_buffer_container.GetBuffer(id),
                    device_context, [=](ConstantBuffers::Bones* buffer)
                {
                    std::copy_n(bone_transform, bone_count, buffer->matrices);
                });
            }
        }
    }
}


void Graphics::FillBoneBuffers(
    Range<RenderComponent const *> components,
    Range<EntityID const *> entities,
    Range<uint32_t const *> entity_to_pose,
    Range<uint32_t const *> pose_offsets,
    Range<Math::Float4x4 const *> bone_transforms,
    ConstantBufferContainer const & constant_buffer_container,
    DeviceContext& device_context
    )
{
    FillBoneBuffersImpl(CreateIntegerRange(Size(components)), components, entities, entity_to_pose, pose_offsets, bone_transforms, constant_buffer_container, device_context);
}


void Graphics::FillBoneBuffers(
    Range<uint32_t const *> component_indices,
    Range<RenderComponent const *> components,
    Range<EntityID const *> entities,
    Range<uint32_t const *> entity_to_pose,
    Range<uint32_t const *> pose_offsets,
    Range<Math::Float4x4 const *> bone_transforms,
    ConstantBufferContainer const & constant_buffer_container,
    DeviceContext& device_context
    )
{
    FillBoneBuffersImpl(component_indices, components, entities, entity_to_pose, pose_offsets, bone_transforms, constant_buffer_container, device_context);
}