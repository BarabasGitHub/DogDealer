#pragma once

#include "DeviceContext.h"
#include "Structures.h"

#include <Utilities\Range.h>

namespace Graphics
{
    class ShaderContainer;
    struct ConstantBufferContainer;

    InputLayoutID SetShaders( DisplayTechnique technique, ShaderContainer const & shader_container, DeviceContext& device_context );

    void SetConstantBuffers( Range<ConstantBufferTypeAndID const*> buffers,  DisplayTechnique technique,  ConstantBufferContainer const & buffer_container, ShaderContainer const & shader_container, DeviceContext& device_context );

    void SetTextures( DeviceContext& device_context, DisplayTechnique technique, ShaderContainer& shader_container, ID3D11ShaderResourceView* const * shader_resource_views, TextureType const * texture_types, unsigned type_and_resource_view_count );

    void UnSetTextures( ID3D11DeviceContext* device_context );
}