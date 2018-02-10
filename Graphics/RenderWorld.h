#pragma once
#include "DLL.h"

#include "Device.h"
#include "DeviceContext.h"
#include "SwapChain.h"

#include "2DTerrainConfiguration.h"
#include "3DTerrainSystemStructs.h"
#include "ConstantBufferContainer.h"
#include "IndexBufferContainer.h"
#include "LightContainer.h"
#include "MeshFunctions.h"
#include "RenderComponentContainer.h"
#include "ResourceManager.h"
#include "ResourceViewContainer.h"
#include "ShaderContainer.h"
#include "TextureContainer.h"
#include "VertexBufferContainer.h"
#include "WorldConfiguration.h"

#include <Conventions\Orientation.h>
#include <Conventions\PoseInfo.h>
#include <Conventions\EntityID.h>
#include <Conventions\PerspectiveViewParameters.h>

#include <Utilities\Range.h>
#include <Utilities\HRTimer.h>

namespace BoundingShapes
{
    struct OrientedBox;
    struct Sphere;
}

namespace Graphics
{
	struct RenderComponentDescription;

    class GRAPHICS_DLL RenderWorld
    {
        Device m_device;
        DeviceContext m_immediate_context;
        SwapChain m_swap_chain;

        RenderComponentContainer m_component_container;
        RenderComponentContainer m_external_debug_component_container;

        VertexBufferContainer	m_vertex_buffer_container;
        IndexBufferContainer	m_index_buffer_container;
        TextureContainer		m_texture_container;
        ResourceViewContainer	m_resource_view_container;
        ShaderContainer			m_shader_container;
        ConstantBufferContainer m_constant_buffer_container;
        LightContainer			m_light_container;

		ResourceManager m_resource_manager;

        Terrain3DData m_terrain_3d_data;
        Terrain2DData m_terrain_2d_data;
        DisplayTechnique m_sky_display_technique;
        Math::Float3 m_sky_color;
        ConstantBufferID m_sky_buffer_id;

        RenderTargetViewID m_final_render_target_view;
        DepthStencilViewID m_depth_stencil_view;
        Texture2DID m_depth_stencil_texture;

        // buffers, not sure this should stay this way
        ConstantBufferID m_projection_buffer_id, m_camera_buffer_id;
        std::array<ComPtr<ID3D11Buffer>, 2> m_instance_buffers;
        std::array<uint32_t, 2> m_instance_buffer_sizes = { { 0, 0 } };

		// same goes for these states
		ComPtr<ID3D11BlendState> m_alpha_coverage_blend_state;
		ComPtr<ID3D11BlendState> m_default_blend_state;
        ComPtr<ID3D11DepthStencilState> m_default_depth_stencil_state;
        ComPtr<ID3D11RasterizerState> m_default_rasterizer_state;

        HRTimer m_loop_timer;

    public:

        WorldConfiguration m_world_configuration;

        void Resize();

        void RenderFor( double start_time, float time_step, IndexedOrientations const & orientations, IndexedOffsetPoses const & poses, Orientation camera_orientation, Orientation previous_camera_orientation, PerspectiveViewParameters perspective_view_parameters );
        void DefaultInitialize(HWND const window);

        void CreateRenderComponent( RenderComponentDescription const & component_description, EntityID entity_id );

        void ReplaceRenderComponents( Range<RenderComponentDescription const *> component_descriptions, EntityID const entity_id );

        void CreateTerrainRenderComponent( RenderComponentDescription const & component_description,
                                        Math::Float3 const & terrain_center,
                                        float const update_distance,
                                        Math::Unsigned3 const & terrain_block_count,
                                        Math::Float3 const & terrain_block_dimensions,
                                        Math::Unsigned3 const & terrain_block_cube_count,
                                        std::vector<float> const & degradation_thresholds,
                                        Get3DSampleFunctionType get_sample_function,
                                        EntityID entity_id );

        void CreateTerrainRenderComponent(
            RenderComponentDescription const & component_description,
            Math::Float3 const & terrain_center,
            Math::Unsigned2 patch_dimensions,
            Math::Float2 patch_size,
            Range<float const *> lod_distances,
            Get2DSampleFunctionType get_sample_function,
            EntityID entity_id );

        void SetTerrainGrassTypes(std::vector<Graphics::RenderComponentDescription> const & descriptions, std::vector<std::array<float, c_lod_count>> const & densities);


        void RemoveEntities( std::vector<EntityID> const & entity_ids );

        void CreateLight( LightDescription const & light_description, EntityID entity_id );

        void UpdateTerrain(Math::Float3 position);

        void AdjustAllPositions(Math::Float3 adjustment);

        // create debug components, they only last for one tick
        void CreateDebugRenderComponents(Range<EntityID const*> entity_ids, Range<BoundingShapes::OrientedBox const*> boxes);
        // create debug components, they only last for one tick
        void CreateDebugRenderComponents(Range<EntityID const*> entity_ids, Range<BoundingShapes::Sphere const*> spheres);

        RenderWorld();
        ~RenderWorld();
    private:

        void CreateDebugRenderComponent( EntityID entity_id, BoundingShapes::AxisAlignedBox box );

        void SetTerrainGrassTypesTerrain2D(
            Range<Graphics::RenderComponentDescription const *> descriptions,
            Range<uint32_t const *> data_offsets,
            Range<float const *> densities,
            Range<float const *> distances);

        void SetTerrainGrassTypesTerrain3D(std::vector<Graphics::RenderComponentDescription> const & descriptions, std::vector<std::array<float, c_lod_count>> const & densities);

        void UpdateTerrain3D(Math::Float3 const position, bool const force_update = false);
        void UpdateTerrain2D(Math::Float3 position);

        void RenderSky();

        void Render( Range<DisplayTechnique const *> const display_techniques,
                     Range<uint32_t const *> component_indices,

                     Range<ConstantBufferTypeAndID const *> additional_constant_buffers
                     );

        void CreateDefaultStates();

        void CreateGlobalConstantBuffers();
        void FillPerFrameConstantBuffers( Math::Float3x3 camera_matrix, Math::Float4x4 projection_matrix, Math::Float3 camera_position );

        void FillWithGlobalConstantBufferIDs(Range<ConstantBufferTypeAndID *> buffers) const;

        void AddTerrainBlockRenderComponents(
            EntityID const terrain_entity_id,
            RenderComponentDescription const & block_render_description,
            Range<MeshData const *> mesh_data,
            RenderComponentContainer & component_container,
            ResourceManager & resource_manager,
            TextureContainer & texture_container,
            ResourceViewContainer & resource_view_container,
            Device & device);
    };

}
