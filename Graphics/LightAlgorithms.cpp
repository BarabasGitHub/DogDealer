#include "LightAlgorithms.h"

#include "ConstantBufferContainer.h"
#include "ConstantBuffers.h"
#include "Device.h"
#include "FillConstantBuffer.h"
#include "LightContainer.h"
#include "ResourceDescriptions.h"
#include "ResourceViewContainer.h"
#include "TextureContainer.h"

#include <Math\IntegerTypes.h>
#include <Math\TransformFunctions.h>
#include <Math\MathFunctions.h>

#include <BoundingShapes\AxisAlignedBoxFunctions.h>
#include <BoundingShapes\PlaneFunctions.h>
#include <BoundingShapes\IntersectionTests.h>
#include <BoundingShapes\FrustumFunctions.h>

#include <Utilities\IntegerRange.h>
#include <Utilities\VectorHelper.h>
#include <Utilities\MinMaxFunctions.h>

#include <vector>

namespace Graphics
{

    void CreateLight(
        EntityID entity_id,
        LightDescription const & description,
        Math::Unsigned2 texture_dimensions,
        Device& device,
        TextureContainer& texture_container,
        ResourceViewContainer& resource_view_container,
        ConstantBufferContainer& constant_buffer_containter,
        LightContainer & light_container )
    {
        // first make all resources
        auto texture_description = CD3D11_TEXTURE2D_DESC( DXGI_FORMAT_R32_TYPELESS, texture_dimensions.x, texture_dimensions.y );
        texture_description.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        texture_description.MipLevels = 1;
        texture_description.ArraySize = light_container.shadow_map_count;
        auto texture = device.CreateTexture2D( texture_description );
        auto depth_description = CD3D11_DEPTH_STENCIL_VIEW_DESC( D3D11_DSV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_D32_FLOAT );
        for( auto i = 0u; i < light_container.shadow_map_count; i++ )
        {
            //depth_description.Texture2D.MipSlice = D3D11CalcSubresource( 0, i, 2 );
            depth_description.Texture2DArray.FirstArraySlice = i;
            depth_description.Texture2DArray.ArraySize = 1;
            depth_description.Texture2DArray.MipSlice = 0;
            auto depth_view = device.CreateDepthStencilView( texture, &depth_description );
            auto depth_view_id = resource_view_container.AddDepthStencilView( depth_view );
            light_container.depth_stencil_views.push_back( depth_view_id );
        }
        auto shader_resource_description = CD3D11_SHADER_RESOURCE_VIEW_DESC( D3D11_SRV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_R32_FLOAT );
        auto shader_view = device.CreateShaderResourceView( texture, &shader_resource_description );

        // then add them in the containers afterwards
        auto texture_id = texture_container.AddTexture( texture );
        auto shader_view_id = resource_view_container.AddShaderResourceView( shader_view );

        auto light_constant_buffer_size = uint32_t(sizeof(ConstantBuffers::Light));
        auto light_constant_buffer = device.CreateConstantBuffer(light_constant_buffer_size);
        auto light_constant_buffer_id = constant_buffer_containter.Add(light_constant_buffer);

        // and put the ids in the light container.
        light_container.textures.push_back( texture_id );
        light_container.view_ports.push_back( CD3D11_VIEWPORT( 0.0f, 0.0f, float(texture_dimensions.x), float(texture_dimensions.y) ) );
        light_container.shader_resource_views.push_back( shader_view_id );
        light_container.entity_ids.push_back( entity_id );
        light_container.constant_buffers.push_back(light_constant_buffer_id);
        light_container.colors.push_back(description.color);

        for( auto i = 0u; i < light_container.shadow_map_count; ++i )
        {
            auto constant_buffer_size = uint32_t(sizeof(ConstantBuffers::SingleLightTransform));
            auto constant_buffer = device.CreateConstantBuffer(constant_buffer_size);
            auto constant_buffer_id = constant_buffer_containter.Add(constant_buffer);
            light_container.transform_constant_buffers.push_back(constant_buffer_id);
        }
    }


    void InitializeLightContainer( Device& device, LightContainer & light_container )
    {
        light_container.depth_stencil_state = CreateShadowMapDepthStencilState( device );
        light_container.rasterizer_state = CreateShadowMapRasterizerState( device );
    }


    ComPtr<ID3D11RasterizerState> CreateShadowMapRasterizerState( Device& device )
    {

        CD3D11_RASTERIZER_DESC rasterizer_state_description( D3D11_DEFAULT );
        rasterizer_state_description.CullMode = D3D11_CULL_FRONT;
        rasterizer_state_description.FrontCounterClockwise = true;
        // No idea what good values are here
        rasterizer_state_description.DepthBias = 0;
        rasterizer_state_description.DepthBiasClamp = 0.0f;
        rasterizer_state_description.SlopeScaledDepthBias = 0.0f;
        rasterizer_state_description.MultisampleEnable = true;

        return device.CreateRasterizerState( rasterizer_state_description );
    }


    ComPtr<ID3D11DepthStencilState> CreateShadowMapDepthStencilState( Device& device )
    {
        CD3D11_DEPTH_STENCIL_DESC description( D3D11_DEFAULT );
        // Depth test parameters
        description.DepthEnable = true;
        description.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        description.DepthFunc = D3D11_COMPARISON_LESS;

        // Stencil test parameters
        description.StencilEnable = false;
        return device.CreateDepthStencilState( description );
    }


    namespace
    {
        Math::Float4x4 CreateDirectionalLightTransformFromLightSpaceMinMax( MinMax<Math::Float3> minmax, Math::Float3x3 const & rotation_matrix )
        {
            auto size = minmax.max - minmax.min;
            auto projection = Math::OrthographicView( size.y, size.x, minmax.min.z, minmax.max.z );
            auto translation = minmax.max + minmax.min;
            translation *= Math::Float3{ .5f, .5f, 1.f };

            return projection * Math::CombineFloat3x3AndTranslation( rotation_matrix, -translation );
        }


        MinMax<Math::Float3> FindLightSpaceMinMax( Math::Quaternion const & rotation, Range<Math::Float3 const *> points )
        {
            assert( !IsEmpty( points ) );
            auto inverse_rotaiton = Conjugate( rotation );
            // use the first point to initialize min and max
            MinMax<Math::Float3> minmax;
            minmax.min = minmax.max = Rotate( First( points ), inverse_rotaiton );
            // skip the first point
            PopFirst( points );
            for( auto const & point : points )
            {
                auto tranformed_point = Rotate( point, inverse_rotaiton );
                minmax = Update( minmax, tranformed_point );
            }
            return minmax;
        }
    }


    Math::Float4x4 CreateDirectionalLightTransform( Math::Quaternion const & rotation, Range<Math::Float3 const *> points )
    {
        auto minmax = FindLightSpaceMinMax( rotation, points );
        return CreateDirectionalLightTransformFromLightSpaceMinMax( minmax, Math::RotationToFloat3x3( Conjugate(rotation) ) );
    }


    namespace
    {
        template<typename IndexIteratorType>
        Math::Float4x4 CreateDirectionalLightTransformImpl( Range<IndexIteratorType> to_be_processed_indices, Range<BoundingShapes::AxisAlignedBox const *> bounding_boxes, Range<Math::Float4x4 const *> transforms, Math::Quaternion rotation )
        {
            assert( Size( bounding_boxes ) == Size( transforms ) );

            // initialize min and max
            MinMax<Math::Float3> minmax;
            minmax.max = std::numeric_limits<float>::lowest();
            minmax.min = std::numeric_limits<float>::max();

            auto const rotation_matrix = Math::RotationToFloat4x4( Conjugate( rotation ) );

            for( auto const i : to_be_processed_indices )
            {
                auto const transform = rotation_matrix * transforms[i];
                auto const tranformed_box = Transform( bounding_boxes[i], transform );
                auto box_minmax = GetMinMax( tranformed_box );
                minmax = Combine( minmax, box_minmax );
            }

            return CreateDirectionalLightTransformFromLightSpaceMinMax( minmax, Math::Float4x4ToFloat3x3(rotation_matrix) );
        }
    }


    Math::Float4x4 CreateDirectionalLightTransform( Range<BoundingShapes::AxisAlignedBox const *> bounding_boxes, Range<Math::Float4x4 const *> transforms, Math::Quaternion rotation )
    {
        return CreateDirectionalLightTransformImpl( CreateIntegerRange( Size(bounding_boxes) ), bounding_boxes, transforms, rotation );
    }


    Math::Float4x4 CreateDirectionalLightTransform( Range<uint32_t const *> to_be_processed_indices, Range<BoundingShapes::AxisAlignedBox const *> bounding_boxes, Range<Math::Float4x4 const *> transforms, Math::Quaternion rotation )
    {
        return CreateDirectionalLightTransformImpl( to_be_processed_indices, bounding_boxes, transforms, rotation );
    }


    namespace
    {
        template<typename IndexIteratorType>
        MinMax<Math::Float3> FindLightSpaceMinMax( Range<IndexIteratorType> to_be_processed_indices, Range<BoundingShapes::AxisAlignedBox const *> bounding_boxes, Math::Float4x4 inverse_rotation_matrix )
        {
            MinMax<Math::Float3> minmax = { std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest() };
            for( auto const i : to_be_processed_indices )
            {
                auto const tranformed_box = Transform( bounding_boxes[i], inverse_rotation_matrix );
                auto box_minmax = GetMinMax( tranformed_box );
                minmax = Combine( minmax, box_minmax );
            }
            return minmax;
        }


        template<typename IndexIteratorType>
        MinMax<Math::Float3> FindLightSpaceMinMax( Range<IndexIteratorType> to_be_processed_indices, Range<BoundingShapes::AxisAlignedBox const *> bounding_boxes, Math::Quaternion rotation, Math::Float3x3 & inverse_rotation_matrix_out )
        {
            auto const inverse_rotation_matrix = Math::RotationToFloat4x4( Conjugate( rotation ) );
            auto minmax = FindLightSpaceMinMax( to_be_processed_indices, bounding_boxes, inverse_rotation_matrix );
            inverse_rotation_matrix_out = Math::Float4x4ToFloat3x3( inverse_rotation_matrix );
            return minmax;
        }


        template<typename IndexIteratorType>
        MinMax<Math::Float3> FindLightSpaceMinMax( Range<IndexIteratorType> to_be_processed_indices, Range<BoundingShapes::AxisAlignedBox const *> bounding_boxes, Math::Quaternion rotation )
        {
            Math::Float3x3 rotation_matrix;
            return FindLightSpaceMinMax( to_be_processed_indices, bounding_boxes, rotation, rotation_matrix );
        }
    }


    Math::Float4x4 CreateDirectionalLightTransform( Range<uint32_t const *> to_be_processed_indices, Range<BoundingShapes::AxisAlignedBox const *> bounding_boxes, Math::Quaternion rotation )
    {
        Math::Float3x3 rotation_matrix;
        auto minmax = FindLightSpaceMinMax( to_be_processed_indices, bounding_boxes, rotation, rotation_matrix);
        return CreateDirectionalLightTransformFromLightSpaceMinMax( minmax, rotation_matrix );
    }


    Math::Float4x4 CreateDirectionalLightTransform( Range<uint32_t const *> to_be_processed_indices, Range<BoundingShapes::AxisAlignedBox const *> bounding_boxes, Math::Quaternion rotation, MinMax<Math::Float3> bounds )
    {
        Math::Float3x3 rotation_matrix;
        auto unbounded_minmax = FindLightSpaceMinMax( to_be_processed_indices, bounding_boxes, rotation, rotation_matrix);
        // apply bounds
        MinMax<Math::Float3> minmax;
        minmax.max = Math::Min( bounds.max, unbounded_minmax.max );
        minmax.min = Math::Max( bounds.min, unbounded_minmax.min );

        return CreateDirectionalLightTransformFromLightSpaceMinMax( minmax, rotation_matrix );
    }


    Math::Float3 CalculateLightNormal( Math::Quaternion const & rotation )
    {
        return Rotate( Math::Float3( 0, 0, 1 ), rotation );
    }


    void FillLightBuffers( Range<Math::Quaternion const *> light_rotations, Range<ID3D11Buffer * const *> const light_buffers, Range<uint32_t const*> const color_render_component_indices, Range<BoundingShapes::AxisAlignedBox const *> const bounding_boxes, Range<Math::Float4x4 const*> const transforms, ID3D11DeviceContext* const device_context, Range<Math::Float4x4 *> const light_transforms )
    {
        assert( Size( bounding_boxes ) == Size( transforms ) );
        assert( Size( light_rotations ) == Size( light_buffers ) );
        assert( Size( light_transforms ) == Size( light_rotations ) );

        for( auto light_index : CreateIntegerRange( Size(light_rotations) ) )
        {
            auto rotation = light_rotations[light_index];
            auto projection_matrix = CreateDirectionalLightTransform( color_render_component_indices, bounding_boxes, transforms, rotation );
            light_transforms[light_index] = projection_matrix;

            FillConstantBuffer<ConstantBuffers::LightSingleMap>( light_buffers[light_index], device_context,
                    [projection_matrix, rotation]( ConstantBuffers::LightSingleMap* buffer )
            {
                buffer->transform = projection_matrix;
                auto normal = CalculateLightNormal( rotation );
                buffer->normal = normal;
            } );

        }
    }


    namespace
    {
        Math::Float4x4 CreateDirectionalLightTransform( Range<uint32_t const *> to_be_processed_indices, Range<BoundingShapes::AxisAlignedBox const *> bounding_boxes, Math::Float4x4 inverse_rotation_matrix, MinMax<Math::Float3> bounds )
        {
            auto unbounded_minmax = FindLightSpaceMinMax( to_be_processed_indices, bounding_boxes, inverse_rotation_matrix );
            // apply bounds
            MinMax<Math::Float3> minmax;
            minmax.max = Math::Min( bounds.max, unbounded_minmax.max );
            minmax.min = Math::Max( bounds.min, unbounded_minmax.min );

            return CreateDirectionalLightTransformFromLightSpaceMinMax( minmax, Float4x4ToFloat3x3( inverse_rotation_matrix ) );
        }
    }


    void FillLightBuffer( Math::Quaternion const light_rotation, Math::Float3 const light_color, Range<BoundingShapes::AxisAlignedBox const *> const bounding_boxes, PerspectiveViewParameters perspective_view, Math::Float4x4 const & camera_matrix, ID3D11DeviceContext* const device_context, ID3D11Buffer * const light_buffer, Range<Math::Float4x4 *> const light_transforms, Range<uint32_t *> visible_box_indices_offsets, std::vector<uint32_t> & visible_box_indices)
    {
        using namespace BoundingShapes;
        using namespace Math;
        assert(Size(light_transforms) + 1 == Size(visible_box_indices_offsets));
        assert(Size(light_transforms) <= ConstantBuffers::Light::c_max_number_of_transforms); // maximum number of light transforms
        auto shadow_map_count = Size(light_transforms);
        auto light_normal = CalculateLightNormal( light_rotation );

        std::array<float, ConstantBuffers::Light::c_max_number_of_transforms + 1> plane_distances;
        plane_distances[0] = perspective_view.near_z;
        for( auto i = 1u; i < shadow_map_count; ++i )
        {
            auto factor = float(i) / float(shadow_map_count);
            auto uniform = Lerp(perspective_view.near_z, perspective_view.far_z, factor);
            auto logarithmic = perspective_view.near_z * std::pow( perspective_view.far_z / perspective_view.near_z, float( i ) / float( shadow_map_count ) );
            auto distance = Lerp( uniform, logarithmic, .95f );
            plane_distances[i] = distance;
        }
        plane_distances[shadow_map_count] = perspective_view.far_z;

        auto projection_height = cot( perspective_view.field_of_view / 2 );
        auto projection_width = projection_height / perspective_view.aspect_ratio;

        // get the boxes which intersect with the area between the different planes
        for( auto i = 0u; i < shadow_map_count; ++i )
        {
            auto part_matrix = PerspectiveHeightWidth( projection_height, projection_width, plane_distances[i], plane_distances[i + 1] ) * camera_matrix;
            std::array<Math::Float3, 8> corners;
            GetFrustumCorners(part_matrix, corners);
            auto lightspace_minmax = FindLightSpaceMinMax( light_rotation, corners );
            auto inverse_light_rotation_matrix = RotationToFloat4x4( Conjugate( light_rotation ) );
            auto light_projection_matrix = CreateDirectionalLightTransformFromLightSpaceMinMax( lightspace_minmax, Float4x4ToFloat3x3(inverse_light_rotation_matrix) );
            std::array<Plane, 5> light_planes = { {
                GetFarPlane( light_projection_matrix ),
                GetRightPlane( light_projection_matrix ),
                GetLeftPlane( light_projection_matrix ),
                GetTopPlane( light_projection_matrix ),
                GetBottomPlane( light_projection_matrix )
            } };

            auto index_offset = uint32_t(Size(visible_box_indices));
            visible_box_indices_offsets[i] = index_offset;
            Intersect(bounding_boxes, light_planes, visible_box_indices);
            if( !IsEmpty( CreateRange( visible_box_indices, index_offset, Size(visible_box_indices) ) ) )
            {
                lightspace_minmax.max.z = std::numeric_limits<float>::max();
                light_projection_matrix = CreateDirectionalLightTransform( 
                    CreateRange( visible_box_indices, index_offset, Size(visible_box_indices)),
                    bounding_boxes, 
                    inverse_light_rotation_matrix, 
                    lightspace_minmax 
                    );
            }
            light_transforms[i] = light_projection_matrix;
        }
        visible_box_indices_offsets[shadow_map_count] = uint32_t(Size(visible_box_indices));


        FillConstantBuffer<ConstantBuffers::Light>( light_buffer, device_context,
                [light_transforms, light_normal, light_color, &plane_distances]( ConstantBuffers::Light* buffer )
        {
            buffer->normal = light_normal;
            buffer->color = light_color;
            for( auto i = 0u; i < ConstantBuffers::Light::c_max_number_of_transforms; ++i )
            {
                buffer->far_planes[i] = -plane_distances[i+1];
            }
            assert(Size(light_transforms) == ConstantBuffers::Light::c_max_number_of_transforms);
            std::copy_n(begin(light_transforms), Size(light_transforms), buffer->transform);
        } );
    }
}