#pragma once

#include "DirectX\Direct3D11.h"

#include <Conventions\EntityID.h>
#include <Conventions\PerspectiveViewParameters.h>

#include <Math\FloatTypes.h>
#include <Math\FloatMatrixTypes.h>

#include <Utilities\Range.h>
#include <Utilities\MinMax.h>
#include <vector>

// forward declarations
namespace BoundingShapes
{
    struct AxisAlignedBox;
}
namespace Graphics
{
    struct Device;
    struct TextureContainer;
    struct ResourceViewContainer;
    struct ConstantBufferContainer;
    struct LightContainer;
    struct RenderComponent;
    struct LightDescription;
}

namespace Graphics
{
    // Creates all necessary resources for a light; shadow texture, depth view, shader resource view, constant buffer. And adds an entry in the LightContainer
    void CreateLight(
        EntityID entity_id,
        LightDescription const & description,
        Math::Unsigned2 texture_dimensions,
        Device& device,
        TextureContainer& texture_container,
        ResourceViewContainer& resource_view_container,
        ConstantBufferContainer& constant_buffer_containter,
        LightContainer & light_container );

    void InitializeLightContainer( Device& device, LightContainer & light_container );

    ComPtr<ID3D11RasterizerState> CreateShadowMapRasterizerState( Device& device );
    ComPtr<ID3D11DepthStencilState> CreateShadowMapDepthStencilState( Device& device );

    // make a transformation such that the light frustum contains all given points
    Math::Float4x4 CreateDirectionalLightTransform( Math::Quaternion const & rotation, Range<Math::Float3 const *> points );
    // make a transformation such that the light frustum contains all components
    Math::Float4x4 CreateDirectionalLightTransform( Range<BoundingShapes::AxisAlignedBox const *> bounding_boxes, Range<Math::Float4x4 const *> transforms, Math::Quaternion rotation );
    // make a transformation such that the light frustum contains all bounding boxes indicated by the to_be_processsed_indices
    Math::Float4x4 CreateDirectionalLightTransform( Range<uint32_t const *> to_be_processed_indices, Range<BoundingShapes::AxisAlignedBox const *> bounding_boxes, Range<Math::Float4x4 const *> transforms, Math::Quaternion rotation );
    // make a transformation such that the light frustum contains all bounding boxes indicated by the to_be_processsed_indices (expects transformed boxes)
    Math::Float4x4 CreateDirectionalLightTransform( Range<uint32_t const *> to_be_processed_indices, Range<BoundingShapes::AxisAlignedBox const *> bounding_boxes, Math::Quaternion rotation );
    // same as above but do not exceed the given minmax (in light space)
    Math::Float4x4 CreateDirectionalLightTransform( Range<uint32_t const *> to_be_processed_indices, Range<BoundingShapes::AxisAlignedBox const *> bounding_boxes, Math::Quaternion rotation, MinMax<Math::Float3> minmax );

    // calculate the normal of the light in world space
    Math::Float3 CalculateLightNormal( Math::Quaternion const & rotation );

    // fills the light buffers and returns the calculated light transforms
    void FillLightBuffers( Range<Math::Quaternion const *> light_rotations, Range<ID3D11Buffer * const *> const light_buffers, Range<uint32_t const*> const color_render_component_indices, Range<BoundingShapes::AxisAlignedBox const *> const bounding_boxes, Range<Math::Float4x4 const*> const transforms, ID3D11DeviceContext* const device_context, Range<Math::Float4x4 *> const light_transforms );

    // calculates the light transforms for multiple shadow maps and boxes that intersect each light frustum
    void FillLightBuffer(
        Math::Quaternion const light_rotation,
        Math::Float3 const light_color,
        Range<BoundingShapes::AxisAlignedBox const *> const bounding_boxes,
        PerspectiveViewParameters perspective_view_parameters,
        Math::Float4x4 const & camera_matrix,
        ID3D11DeviceContext* const device_context,
        ID3D11Buffer * const light_buffer,
        Range<Math::Float4x4 *> const light_transforms,
        Range<uint32_t *> visible_box_indices_offsets,
        std::vector<uint32_t> & visible_box_indices
        );
}