#include "RenderWorld.h"
#include "CreateDeviceAndSwapChain.h"

#include "DirectX\Direct3D11.h"

#include "ConstantBuffers.h"
#include "ConstantBufferFunctions.h"
#include "ConstantBufferTypeAndIDFunctions.h"
#include "FillConstantBuffer.h"
#include "LightAlgorithms.h"
#include "SetShaderResources.h"
#include "RenderComponentFunctions.h"
#include "2DTerrainSystem.h"
#include "3DTerrainSystem.h"

#include <BoundingShapes\IntersectionTests.h>
#include <BoundingShapes\AxisAlignedBoxFunctions.h>
#include <BoundingShapes\OrientedBox.h>

#include <Conventions\OrientationFunctions.h>
#include <Conventions\PerspectiveViewFunctions.h>

#include <Math\Conversions.h>
#include <Math\FloatMatrixOperators.h>
#include <Math\FloatMatrixTypes.h>
#include <Math\FloatTypes.h>
#include <Math\MathFunctions.h>
#include <Math\TransformFunctions.h>

#include <Utilities\IntegerRange.h>
#include <Utilities\Logger.h>
#include <Utilities\Memory.h>
#include <Utilities\StdVectorFunctions.h>

#include <array>
#include <utility> // for rel_ops
#include <vector>


using namespace Graphics;

namespace
{
    struct TargetsAndStates
    {
        uint32_t render_view_count;
        std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> render_views;
        ID3D11DepthStencilView* depth_stencil_view;

        // we should probably change it to a pointer or something
        std::vector<D3D11_VIEWPORT> view_ports;

        ComPtr<ID3D11RasterizerState> rasterizer_state;
        ComPtr<ID3D11DepthStencilState> depth_stencil_state;
    };
}

// Append size of mesh_data new render components to the end of the vectors of the render component container
// Initialize all attributes of the new components with the block_render_description
void RenderWorld::AddTerrainBlockRenderComponents(
    EntityID const terrain_entity_id,
    RenderComponentDescription const & block_render_description,
    Range<MeshData const *> mesh_data,
    RenderComponentContainer & component_container,
    ResourceManager & resource_manager,
    TextureContainer & texture_container,
    ResourceViewContainer & resource_view_container,
    Device & device)
{
    auto new_block_count = Size(mesh_data);
    // Get textures used by the terrain blocks
    std::vector<TextureTypeAndID> texture_list(Size(block_render_description.textures));
    resource_manager.ProvideTextureList(block_render_description.textures, texture_container, resource_view_container, device, texture_list);
    // Get techniques and alpha-to-coverage for terrain blocks
    auto display_technique = resource_manager.ProvideDisplayTechnique(block_render_description.technique);
    auto shadow_display_technique = resource_manager.ProvideDisplayTechnique(block_render_description.shadow_technique);

    auto alpha_to_coverage = block_render_description.alpha_to_coverage;

    // Initialize component (meshes will be overwritten later)
    auto component = RenderComponent();
    component.textures = texture_list;
    AddObjectConstantBuffer(m_device, m_constant_buffer_container, component);

    // insert new entries at end of vector
    auto new_size = component_container.entity_ids.size() + new_block_count;
    component_container.entity_ids.resize(new_size, terrain_entity_id);
    component_container.display_techniques.resize(new_size, display_technique);
    component_container.shadow_display_techniques.resize(new_size, shadow_display_technique);
    component_container.alpha_to_coverage.resize(new_size, alpha_to_coverage);
    component_container.instance_transform_buffers.resize(new_size, { { c_invalid_vertex_buffer_id, c_invalid_vertex_buffer_id } });
    component_container.orientation_offsets.resize(new_size, Math::Identity());

    component_container.components.reserve(new_size);
    component_container.bounding_boxes.reserve(new_size);
    for( auto const & mesh_entry : mesh_data )
    {
        component.mesh = mesh_entry.mesh;
        component_container.components.push_back(component);
        component_container.bounding_boxes.push_back(mesh_entry.bounding_box);
    }
    assert(IsInValidState(component_container));
}


RenderWorld::RenderWorld()
{
    m_world_configuration.render_sample_count = 1;
    m_world_configuration.maximum_texture_filtering_quality = TextureFiltering::Anisotropic_16x;
    m_world_configuration.display_bounding_boxes = false;
    m_world_configuration.force_wire_frame = false;
    m_world_configuration.render_external_debug_components = true;

    m_light_container.shadow_map_count = ConstantBuffers::Light::c_max_number_of_transforms;
}


RenderWorld::~RenderWorld()
{
}

// Creates a RenderComponent from the description, returning its ID
void RenderWorld::CreateRenderComponent( RenderComponentDescription const & component_description, EntityID entity_id )
{
    RenderComponent component;

    auto mesh_data = m_resource_manager.ProvideMeshData( component_description.mesh, m_index_buffer_container, m_vertex_buffer_container, m_device );
    component.mesh = mesh_data.mesh;
    component.textures.resize(Size(component_description.textures));
    m_resource_manager.ProvideTextureList( component_description.textures, m_texture_container, m_resource_view_container, m_device, component.textures );

    AddObjectConstantBuffer(m_device, m_constant_buffer_container, component);

    auto display_technique = m_resource_manager.ProvideDisplayTechnique( component_description.technique );
    auto shadow_display_technique = m_resource_manager.ProvideDisplayTechnique( component_description.shadow_technique );

    if(m_shader_container.GetProperties( display_technique.vertex_shader_id ).animated)
    {
        AddBoneConstantBuffer(m_device, m_constant_buffer_container, component);
    }

    AddRenderComponent(
        entity_id,
        std::move(component),
        display_technique,
        shadow_display_technique,
        mesh_data.bounding_box,
        component_description.alpha_to_coverage,
        {{c_invalid_vertex_buffer_id, c_invalid_vertex_buffer_id}},
        Math::Identity(),
        m_component_container);

    CreateDebugRenderComponent(entity_id, mesh_data.bounding_box);
}


void RenderWorld::CreateDebugRenderComponent( EntityID entity_id, BoundingShapes::AxisAlignedBox box )
{
    if(!m_world_configuration.display_bounding_boxes) // don't do anything if we don't want to display them
    {
        return;
    }

    RenderComponent component;
    auto mesh_data = m_resource_manager.ProvideMeshData("debug_wire_box", m_index_buffer_container, m_vertex_buffer_container, m_device);
    component.mesh = mesh_data.mesh;
    component.textures = {};

    AddObjectConstantBuffer(m_device, m_constant_buffer_container, component);

    DisplayTechniqueDescription tech_desc;
    tech_desc.vertex_shader = "ABBAVertexShader";
    tech_desc.pixel_shader  = "PlainColorPixelShader";

    DisplayTechnique technique = m_resource_manager.ProvideDisplayTechnique( tech_desc );
    DisplayTechnique shadow_technique( c_invalid_shader_id );

    Math::Float4x4 offset_transform = AffineTransform(box.center, box.extent);

    AddRenderComponent(entity_id, component, technique, shadow_technique, mesh_data.bounding_box, false, {{c_invalid_vertex_buffer_id, c_invalid_vertex_buffer_id}}, offset_transform, m_component_container);
}


void RenderWorld::CreateDebugRenderComponents( Range<EntityID const*> entity_ids, Range<BoundingShapes::OrientedBox const*> boxes)
{
    assert(Size(entity_ids) == Size(boxes));

    RenderComponent component;
    auto mesh_data = m_resource_manager.ProvideMeshData("debug_wire_box", m_index_buffer_container, m_vertex_buffer_container, m_device);
    component.mesh = mesh_data.mesh;
    component.textures = {};


    DisplayTechniqueDescription tech_desc;
    tech_desc.vertex_shader = "PlainColorVertexShader";
    tech_desc.pixel_shader  = "PlainColorPixelShader";

    DisplayTechnique technique = m_resource_manager.ProvideDisplayTechnique( tech_desc );
    DisplayTechnique shadow_technique( c_invalid_shader_id );

    for (auto i = 0u; i < Size(entity_ids); ++i)
    {
        component.buffers.clear();
        AddObjectConstantBuffer(m_device, m_constant_buffer_container, component);
        auto& box = boxes[i];
        Math::Float4x4 offset_transform = AffineTransform(box.center, box.rotation, box.extent);

        AddRenderComponent(entity_ids[i], component, technique, shadow_technique, mesh_data.bounding_box, false, {{c_invalid_vertex_buffer_id, c_invalid_vertex_buffer_id}}, offset_transform, m_external_debug_component_container);
    }
}


void RenderWorld::CreateDebugRenderComponents( Range<EntityID const*> entity_ids, Range<BoundingShapes::Sphere const*> spheres)
{
    assert(Size(entity_ids) == Size(spheres));

    RenderComponent component;
    auto mesh_data = m_resource_manager.ProvideMeshData("debug_sphere", m_index_buffer_container, m_vertex_buffer_container, m_device);
    component.mesh = mesh_data.mesh;
    component.textures = {};


    DisplayTechniqueDescription tech_desc;
    tech_desc.vertex_shader = "PlainColorVertexShader";
    tech_desc.pixel_shader  = "PlainColorPixelShader";

    DisplayTechnique technique = m_resource_manager.ProvideDisplayTechnique( tech_desc );
    DisplayTechnique shadow_technique( c_invalid_shader_id );

    for (auto i = 0u; i < Size(entity_ids); ++i)
    {
        component.buffers.clear();
        AddObjectConstantBuffer(m_device, m_constant_buffer_container, component);
        auto& sphere = spheres[i];
        Math::Float4x4 offset_transform = AffineTransform(sphere.center, Math::Float3(sphere.radius));

        AddRenderComponent(entity_ids[i], component, technique, shadow_technique, mesh_data.bounding_box, false, {{c_invalid_vertex_buffer_id, c_invalid_vertex_buffer_id}}, offset_transform, m_external_debug_component_container);
    }
}


// Creates a RenderComponent from the description, returning its ID
void RenderWorld::ReplaceRenderComponents( Range<RenderComponentDescription const *> const component_descriptions, EntityID entity_id )
{
    auto start_search = begin( m_component_container.entity_ids );
    for( size_t i = 0; i < Size( component_descriptions ); i++ )
    {
        auto & component_description = component_descriptions[i];

        start_search = std::find( start_search, end( m_component_container.entity_ids ), entity_id );
        auto index = uint32_t( start_search - begin( m_component_container.entity_ids ) );
        // valid index
        if( index == Size( m_component_container.entity_ids ) )
        {
            Grow(m_component_container);
        }
        else
        {
            // TODO: clean up the component resources that are soon to be replaced
            //ReleaseAllResources(
            //    m_component_container.components[index],
            //    m_vertex_buffer_container,
            //    m_index_buffer_container,
            //    m_texture_container,
            //    m_resource_view_container,
            //    m_constant_buffer_container);

        }

        // create stuff after cleaning up
        RenderComponent component;

        auto mesh_data = m_resource_manager.ProvideMeshData(component_description.mesh, m_index_buffer_container, m_vertex_buffer_container, m_device);
        component.mesh = mesh_data.mesh;
        component.textures.resize(Size(component_description.textures));
        m_resource_manager.ProvideTextureList(component_description.textures, m_texture_container, m_resource_view_container, m_device, component.textures);
        AddObjectConstantBuffer(m_device, m_constant_buffer_container, component);
        component.buffers.emplace_back(ConstantBufferType::Light, m_light_container.constant_buffers[0]);

        auto display_technique = m_resource_manager.ProvideDisplayTechnique(component_description.technique);
        auto shadow_display_technique = m_resource_manager.ProvideDisplayTechnique(component_description.shadow_technique);
        if(m_shader_container.GetProperties(display_technique.vertex_shader_id).animated)
        {
            AddBoneConstantBuffer(m_device, m_constant_buffer_container, component);
        }

        // skip this entry for the next search, we recalculate from the start in case we had to resize
        start_search = begin(m_component_container.entity_ids) + index + 1;
        assert( index < Size( m_component_container.entity_ids ) );
        m_component_container.components[index] = std::move( component );
        m_component_container.display_techniques[index] = display_technique;
        m_component_container.shadow_display_techniques[index] = shadow_display_technique;
        m_component_container.bounding_boxes[index] = mesh_data.bounding_box;
        m_component_container.alpha_to_coverage[index] = component_description.alpha_to_coverage;
        m_component_container.instance_transform_buffers[index] = { { c_invalid_vertex_buffer_id, c_invalid_vertex_buffer_id } };
        m_component_container.orientation_offsets[index] = Math::Identity();

        assert( IsInValidState( m_component_container ) );
    }

    // remove any remaining components
    for( start_search = std::find( start_search, end( m_component_container.entity_ids ), entity_id );
         start_search < end( m_component_container.entity_ids );
         start_search = std::find( start_search, end( m_component_container.entity_ids ), entity_id ) )
    {
        RemoveRenderComponent(uint32_t(start_search - begin(m_component_container.entity_ids)), m_component_container);
    }

    if(m_world_configuration.display_bounding_boxes)
    {
        for( size_t i = 0; i < Size( component_descriptions ); i++ )
        {
            auto & component_description = component_descriptions[i];
            auto mesh_data = m_resource_manager.ProvideMeshData( component_description.mesh, m_index_buffer_container, m_vertex_buffer_container, m_device );
            CreateDebugRenderComponent(entity_id, mesh_data.bounding_box);
        }
    }
}


void RenderWorld::CreateTerrainRenderComponent(RenderComponentDescription const & component_description,
                                                Math::Float3 const & terrain_center,
                                                float const update_distance,
                                                Math::Unsigned3 const & terrain_block_count,
                                                Math::Float3 const & terrain_block_dimensions,
                                                Math::Unsigned3 const & terrain_block_cube_count,
                                                std::vector<float> const & degradation_thresholds,
                                                Get3DSampleFunctionType get_sample_function,
                                                EntityID entity_id)
{
    InitializeTerrain3DData(terrain_center, update_distance, terrain_block_count, terrain_block_dimensions, terrain_block_cube_count, degradation_thresholds, move(get_sample_function), component_description, entity_id, m_terrain_3d_data);

    auto test_position = terrain_center;

    // Force terrain update
    bool force_update = true;
    UpdateTerrain3D(test_position, force_update);
}


void RenderWorld::CreateTerrainRenderComponent(
    RenderComponentDescription const & component_description,
    Math::Float3 const & terrain_center,
    Math::Unsigned2 patch_dimensions,
    Math::Float2 patch_size,
    Range<float const *> lod_distances,
    Get2DSampleFunctionType get_sample_function,
    EntityID entity_id )
{
    // temporarily here, should be input probably
    auto edge_block_count = 128;
    Math::Unsigned2 total_size(edge_block_count, edge_block_count);

    // 2x2km visibility is probably good enough.

    InitializeTerrainData(
        // input forms
        entity_id,
        terrain_center,
        lod_distances,
        get_sample_function,
        patch_dimensions,
        patch_size,
        total_size,
        component_description,
        // overpaid managers
        m_resource_manager,
        m_constant_buffer_container,
        m_texture_container,
        m_resource_view_container,
        m_index_buffer_container,
        m_device,
        m_immediate_context,
        // actual output
        m_terrain_2d_data
        );

    InitializePatchData(
        m_device,
        m_immediate_context,
        m_texture_container,
        m_resource_view_container,
        m_constant_buffer_container,
        m_terrain_2d_data
        );

    auto old_size = Size(m_component_container.bounding_boxes);
    AddRenderComponents(m_terrain_2d_data, m_component_container);
    auto new_size = Size(m_component_container.bounding_boxes);
    for( auto i = old_size; i < new_size; ++i )
    {
         CreateDebugRenderComponent(m_terrain_2d_data.entity_id, m_component_container.bounding_boxes[i]);
    }
}

void RenderWorld::SetTerrainGrassTypes(std::vector<Graphics::RenderComponentDescription> const & descriptions, std::vector<std::array<float, c_lod_count>> const & densities)
{
    // The 3d terrain needs to set it before it seems
    // auto valid_3d_terrain = m_terrain_3d_data.terrain_entity_id != c_invalid_entity_id;
    auto valid_2d_terrain = m_terrain_2d_data.entity_id != c_invalid_entity_id;
    // assert(valid_3d_terrain != valid_2d_terrain);
    // if( valid_3d_terrain )
    // {
        SetTerrainGrassTypesTerrain3D(descriptions, densities);
    // }
    if( valid_2d_terrain )
    {
        // SCAMPY
        std::vector<uint32_t> data_offsets(Size(descriptions) + 1);
        for (auto i = 0u; i < Size(descriptions); )
        {
            i += 1;
            data_offsets[i] = c_lod_count * i;
        }
        std::vector<float> distances(Size(descriptions) * c_lod_count);
        assert(Size(m_terrain_2d_data.lod_distances) == c_lod_count);
        for (auto i = 0u; i < Size(descriptions); ++i)
        {
            Copy(m_terrain_2d_data.lod_distances.data(), c_lod_count, distances.data() + c_lod_count * i);
        }
        SetTerrainGrassTypesTerrain2D(descriptions, data_offsets, ReinterpretRange<float const>(CreateRange(densities)), distances);
    }
}


void RenderWorld::SetTerrainGrassTypesTerrain2D(
    Range<Graphics::RenderComponentDescription const *> descriptions,
    Range<uint32_t const *> data_offsets,
    Range<float const *> densities,
    Range<float const *> distances)
{
    assert(m_terrain_2d_data.entity_id != c_invalid_entity_id);
    assert(Size(descriptions) + 1 == Size(data_offsets));
    assert(Size(distances) == Size(densities));
    assert(m_terrain_2d_data.grass.empty());

    InitializeGrassData(
        descriptions,
        data_offsets,
        densities,
        distances,
        m_terrain_2d_data.total_size,
        m_resource_manager,
        m_index_buffer_container,
        m_vertex_buffer_container,
        m_texture_container,
        m_resource_view_container,
        m_constant_buffer_container,
        m_device,
        m_terrain_2d_data.grass,
        m_terrain_2d_data.grass_lod_data_offsets,
        m_terrain_2d_data.grass_lod_densities,
        m_terrain_2d_data.grass_lod_distances
        );
}


void RenderWorld::SetTerrainGrassTypesTerrain3D(std::vector<Graphics::RenderComponentDescription> const & descriptions, std::vector<std::array<float, c_lod_count>> const & densities)
{
    m_terrain_3d_data.grass_parameters.descriptions = descriptions;
    m_terrain_3d_data.grass_parameters.densities = densities;

    // Get bounds for individual grass types
    for (auto & description : descriptions)
    {
        auto grass_mesh = m_resource_manager.ProvideMeshData(description.mesh, m_index_buffer_container, m_vertex_buffer_container, m_device);

        m_terrain_3d_data.grass_parameters.bounds.push_back(grass_mesh.bounding_box);
    }
}


void RenderWorld::UpdateTerrain(Math::Float3 position)
{
    auto valid_3d_terrain = m_terrain_3d_data.terrain_entity_id != c_invalid_entity_id;
    auto valid_2d_terrain = m_terrain_2d_data.entity_id != c_invalid_entity_id;
    assert(valid_3d_terrain != valid_2d_terrain);
    if( valid_3d_terrain )
    {
        UpdateTerrain3D(position);
    }
    if( valid_2d_terrain )
    {
        UpdateTerrain2D(position);
    }
}


void RenderWorld::UpdateTerrain2D(Math::Float3 position)
{
    assert(m_terrain_2d_data.entity_id != c_invalid_entity_id);

    UpdatePatchData(
        position,
        m_device,
        m_immediate_context,
        m_texture_container,
        m_resource_view_container,
        m_constant_buffer_container,
        m_terrain_2d_data
        );

    UpdateGrassBuffers(
        position,
        100,
        m_terrain_2d_data,
        m_vertex_buffer_container,
        m_device,
        m_immediate_context
        );

    RemoveRenderComponents(m_terrain_2d_data.entity_id, m_component_container);
    auto old_size = Size(m_component_container.bounding_boxes);
    AddRenderComponents(m_terrain_2d_data, m_component_container);
    AddGrassRenderComponents(m_terrain_2d_data, m_component_container);
    auto new_size = Size(m_component_container.bounding_boxes);
    for( auto i = old_size; i < new_size; ++i )
    {
         CreateDebugRenderComponent(m_terrain_2d_data.entity_id, m_component_container.bounding_boxes[i]);
    }
}


void RenderWorld::UpdateTerrain3D(Math::Float3 const position, bool const force_update)
{
    if( m_terrain_3d_data.terrain_entity_id == c_invalid_entity_id ) return;
    // Check if update is needed
    // Get distance of player from highest-quality point
    auto offset = position - m_terrain_3d_data.real_center;

    // Check for leaving block
    // Use circular metric with radius being half the diagonal of a block
    bool leaving_block = Norm(offset) > m_terrain_3d_data.update_distance;

    if (leaving_block || force_update)
    {
        // Call terrain update
        std::vector<MeshData> display_meshes;
        std::vector<GrassBufferVector> display_grass_buffers;
        std::vector<GrassBoundsVector> display_grass_buffer_bounds;

        std::vector<MeshData> unload_meshes;
        std::vector<VertexBufferID> unload_instance_buffers;
        UpdateTerrainBlocks(position,
                        m_terrain_3d_data,
                        display_meshes,
                        display_grass_buffers,
                        display_grass_buffer_bounds,
                        unload_meshes,
                        unload_instance_buffers,
                        m_index_buffer_container,
                        m_vertex_buffer_container,
                        m_device);

        // Unload pruned terrain block meshes
        for (auto& mesh_data : unload_meshes)
        {
            auto& mesh = mesh_data.mesh;

            m_index_buffer_container.RemoveIndexBuffer(mesh.index_id);
            for( auto vertex_id : mesh.vertex_ids )
            {
                m_vertex_buffer_container.Remove( vertex_id );
            }
        }

        // Unload pruned grass instance buffers
        for( auto& buffer : unload_instance_buffers )
        {
            m_vertex_buffer_container.Remove( buffer );
        }

        // ERASE OLD TERRAIN BLOCK RENDER COMPONENTS:
        RemoveRenderComponents(m_terrain_3d_data.terrain_entity_id, m_component_container);

        // Initialize new render components to be used for the terrain blocks
        AddTerrainBlockRenderComponents(
            m_terrain_3d_data.terrain_entity_id,
            m_terrain_3d_data.block_render_description,
            display_meshes,
            m_component_container,
            m_resource_manager,
            m_texture_container,
            m_resource_view_container,
            m_device);

        if( m_world_configuration.display_bounding_boxes )
        {
            for( auto& mesh_entry : display_meshes )
            {
                CreateDebugRenderComponent(m_terrain_3d_data.terrain_entity_id, mesh_entry.bounding_box);
            }
        }

        // Iterate over display buffers, each index representing a block
        for (auto b = 0u; b < display_grass_buffers.size(); b++)
        {
            // Spawn all grass types for the b-th block
            // Iterate over grass types
            assert( display_grass_buffers.front().size() == m_terrain_3d_data.grass_parameters.densities.size() );
            for (auto g = 0u; g < m_terrain_3d_data.grass_parameters.densities.size(); g++)
            {
                // Get description for current grass type
                auto description = m_terrain_3d_data.grass_parameters.descriptions[g];

                // Get mesh for current type from resource manager
                auto grass_mesh = m_resource_manager.ProvideMeshData(description.mesh, m_index_buffer_container, m_vertex_buffer_container, m_device);
                RenderComponent grass_component;
                grass_component.mesh = grass_mesh.mesh;

                // Set bounds of component to bounds of grass instance buffer
                // to represent all instances
                grass_mesh.bounding_box = display_grass_buffer_bounds[b][g];

                // Get grass texture list
                grass_component.textures.resize(Size(description.textures));
                m_resource_manager.ProvideTextureList(description.textures, m_texture_container, m_resource_view_container, m_device, grass_component.textures);
                AddObjectConstantBuffer(m_device, m_constant_buffer_container, grass_component);

                // Get techniques
                auto grass_technique = m_resource_manager.ProvideDisplayTechnique(description.technique);
                auto grass_shadow_technique = m_resource_manager.ProvideDisplayTechnique(description.shadow_technique);

                AddRenderComponent(
                    m_terrain_3d_data.terrain_entity_id,
                    grass_component,
                    grass_technique,
                    grass_shadow_technique,
                    grass_mesh.bounding_box,
                    description.alpha_to_coverage,
                    display_grass_buffers[b][g],
                    Math::Identity(),
                    m_component_container);
            }
        }


        if( m_world_configuration.display_bounding_boxes )
        {
            // Iterate over display buffers, each index representing a block
            for (auto b = 0u; b < display_grass_buffers.size(); b++)
            {
                // Spawn all grass types for the b-th block
                // Iterate over grass types
                assert( display_grass_buffers.front().size() == m_terrain_3d_data.grass_parameters.densities.size() );
                for (auto g = 0u; g < m_terrain_3d_data.grass_parameters.densities.size(); g++)
                {
                    CreateDebugRenderComponent(m_terrain_3d_data.terrain_entity_id, display_grass_buffer_bounds[b][g]);
                }
            }
        }
    }
}


void RenderWorld::CreateLight( LightDescription const & description, EntityID entity_id )
{
    assert(m_light_container.entity_ids.empty() && "Only one light source is supported at the moment.");
    // ::CreateLight(m_device, m_texture_container, m_resource_view_container, m_light_container, entity_id, { 4096u, 4096u });  // was 2048^2
    // ::CreateLight(m_device, m_texture_container, m_resource_view_container, m_light_container, entity_id, { 2048u, 2048u });  // was 2048^2
    ::CreateLight(entity_id, description, { 1024u, 1024u }, m_device, m_texture_container, m_resource_view_container, m_constant_buffer_container, m_light_container);  // was 2048^2
    for( auto & component : m_component_container.components)
    {
        for( auto & buffer_type_and_id : component.buffers)
        {
            if(buffer_type_and_id.type == ConstantBufferType::Light)
            {
                buffer_type_and_id.id = m_light_container.constant_buffers[0];
                break;
            }
        }
    }
}


namespace
{
    void RemoveShaders( ShaderContainer& shader_container, DisplayTechnique display_technique )
    {
        shader_container.Remove( display_technique.vertex_shader_id );
        // TODO: the other shaders
        shader_container.Remove( display_technique.pixel_shader_id );
    }
}


void RenderWorld::RemoveEntities( std::vector<EntityID> const & entity_ids )
{
    for( auto id : entity_ids )
    {
        // TODO: Remove resources used by components
        RemoveRenderComponents(id, m_component_container);

        auto found = std::find( begin( m_light_container.entity_ids ), end( m_light_container.entity_ids ), id );
        if( found != end( m_light_container.entity_ids ) )
        {
            auto light_index = found - begin( m_light_container.entity_ids );

            m_light_container.depth_stencil_views.erase( begin( m_light_container.depth_stencil_views ) + light_index );
            m_light_container.shader_resource_views.erase( begin( m_light_container.shader_resource_views ) + light_index );
            m_light_container.textures.erase( begin( m_light_container.textures ) + light_index );
            m_light_container.view_ports.erase( begin( m_light_container.view_ports ) + light_index );
        }
    }
}


void RenderWorld::Resize()
{
    // release all render targets
    m_immediate_context->OMSetRenderTargets( 0, nullptr, nullptr );

    m_resource_view_container.RemoveRenderTargetView( m_final_render_target_view );
    m_resource_view_container.RemoveDepthStencilView( m_depth_stencil_view );

    m_texture_container.RemoveTexture2D( m_depth_stencil_texture );

    m_swap_chain.ResizeBuffers();

    // create
    auto back_buffer = m_swap_chain.GetBuffer( 0 );
    m_final_render_target_view = m_resource_view_container.AddRenderTargetView( m_device.CreateRenderTargetView( back_buffer ) );

    m_depth_stencil_texture = m_texture_container.CreateDepthStencilTexture2D( m_device, back_buffer, DXGI_FORMAT_D32_FLOAT_S8X24_UINT );
    auto depth_stencil_texture = m_texture_container.GetTexture2D( m_depth_stencil_texture );

    m_depth_stencil_view = m_resource_view_container.AddDepthStencilView( m_device.CreateDepthStencilView( depth_stencil_texture ) );

    //// update the camera data
}


void RenderWorld::DefaultInitialize( HWND const window )
{
    std::tie(m_device, m_swap_chain) = CreateDefaultDeviceAndSwapchain( window, m_world_configuration.render_sample_count );
    m_immediate_context = m_device.GetImmediateContext();

    // create depth stencil state
    //auto depth_stencil_state = m_device.CreateDepthStencilState(true, false);

    // create rendertargetview
    auto back_buffer = m_swap_chain.GetBuffer( 0 );
    m_final_render_target_view = m_resource_view_container.AddRenderTargetView( m_device.CreateRenderTargetView( back_buffer ) );

    m_depth_stencil_texture = m_texture_container.CreateDepthStencilTexture2D( m_device, back_buffer, DXGI_FORMAT_D32_FLOAT_S8X24_UINT );
    auto depth_stencil_texture = m_texture_container.GetTexture2D( m_depth_stencil_texture );

    m_depth_stencil_view = m_resource_view_container.AddDepthStencilView( m_device.CreateDepthStencilView( depth_stencil_texture ) );

    InitializeLightContainer( m_device, m_light_container );

    m_resource_manager.PreLoadShaderFiles( m_world_configuration.maximum_texture_filtering_quality, m_shader_container, m_device );

    CreateGlobalConstantBuffers();
    CreateDefaultStates();

    // TODO: Disable by default
    m_resource_manager.CreateDebugResources(m_index_buffer_container, m_vertex_buffer_container, m_device);

    DisplayTechniqueDescription sky_technique;
    sky_technique.vertex_shader = "SkyVertexShader";
    sky_technique.pixel_shader = "SkyPixelShader";
    m_sky_display_technique = m_resource_manager.ProvideDisplayTechnique( sky_technique );
    m_sky_color = {.6f, .75f, 1.0f};
    // m_sky_color = {.0f, .0f, .0f};
}


namespace
{

    template<typename Type>
    inline void OrderData( Range<EntityID const * > ids, Range<Type const *> data, Range<uint32_t const *> indices, Range<Type *> output )
    {
        assert(Size(output) == Size( ids ) );
        auto size = Size( ids );
        for( auto i = 0u; i < size; ++i )
        {
            auto index = ids[i].index;
            auto data_index = indices[index];
            output[i] = data[data_index];
        }
    }


    void SetRenderTargetsAndStates( ID3D11DeviceContext* context, TargetsAndStates const & targets_and_states )
    {
        // things I have no idea about
        context->OMSetRenderTargets( targets_and_states.render_view_count, targets_and_states.render_views.data(), targets_and_states.depth_stencil_view );
        context->RSSetViewports( uint32_t( targets_and_states.view_ports.size() ), targets_and_states.view_ports.data() );

        // for all kinds of blending and rasterizing
        context->RSSetState( targets_and_states.rasterizer_state );

        context->OMSetDepthStencilState( targets_and_states.depth_stencil_state, 1 );
    }


    void ClearTargets( ID3D11DeviceContext* const context, ID3D11RenderTargetView* const * const render_views, uint32_t const render_view_count, ID3D11DepthStencilView* const depth_stencil_view )
    {
        float color[] = { 0.f, 0.f, 0.f, 1.0f };
        //float color[] = { 0.005f, 0.005f, .01f, 1.0f }; // space colour
        auto index = render_view_count;
        while( index-- > 0 ) context->ClearRenderTargetView( render_views[index], color );
        context->ClearDepthStencilView( depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
    }


    void ClearAndSet( ID3D11DeviceContext* const device_context, TargetsAndStates targets_and_states )
    {
        UnSetTextures( device_context );

        ClearTargets( device_context, targets_and_states.render_views.data(), targets_and_states.render_view_count, targets_and_states.depth_stencil_view );

        SetRenderTargetsAndStates( device_context, targets_and_states );
    }



    struct ColorRenderSorter
    {
        ColorRenderSorter( RenderComponentContainer const * rcc, ShaderContainer const * sc ) :
            m_component_container( rcc ),
            m_shader_container( sc )
        {
            assert(rcc != nullptr);
            assert(sc != nullptr);
        }


        bool operator()( uint32_t const index1, uint32_t const index2 )
        {
            auto const & display_technique1 = m_component_container->display_techniques[index1];
            auto const & display_technique2 = m_component_container->display_techniques[index2];

            auto const vertex_shader_id1 = display_technique1.vertex_shader_id;
            auto animated1 = m_shader_container->IsValid( vertex_shader_id1 ) && m_shader_container->GetProperties( vertex_shader_id1 ).animated;
            auto const vertex_shader_id2 = display_technique2.vertex_shader_id;
            auto animated2 = m_shader_container->IsValid( vertex_shader_id2 ) && m_shader_container->GetProperties( vertex_shader_id2 ).animated;

            auto alpha_to_coverage1 = m_component_container->alpha_to_coverage[index1];
            auto alpha_to_coverage2 = m_component_container->alpha_to_coverage[index2];

            return std::tie( alpha_to_coverage1, animated1, display_technique1, m_component_container->components[index1].textures ) <
                std::tie( alpha_to_coverage2, animated2, display_technique2, m_component_container->components[index2].textures );
        };


        RenderComponentContainer const * m_component_container;
        ShaderContainer const * m_shader_container;
    };


    struct ShadowSorter
    {
        ShadowSorter( RenderComponentContainer const * rcc, ShaderContainer const * sc ) :
            m_component_container(rcc),
            m_shader_container(sc)
        {
            assert(rcc != nullptr);
            assert(sc != nullptr);
        }


        bool operator()(uint32_t const index1, uint32_t const index2 ) const
        {
            auto const & display_technique1 = m_component_container->shadow_display_techniques[index1];
            auto const & display_technique2 = m_component_container->shadow_display_techniques[index2];

            auto const vertex_shader_id1 = display_technique1.vertex_shader_id;
            auto animated1 = m_shader_container->IsValid( vertex_shader_id1 ) && m_shader_container->GetProperties( vertex_shader_id1 ).animated;
            auto const vertex_shader_id2 = display_technique2.vertex_shader_id;
            auto animated2 = m_shader_container->IsValid( vertex_shader_id2 ) && m_shader_container->GetProperties( vertex_shader_id2 ).animated;

            auto alpha_to_coverage1 = m_component_container->alpha_to_coverage[index1];
            auto alpha_to_coverage2 = m_component_container->alpha_to_coverage[index2];

            return std::tie( alpha_to_coverage1, animated1, display_technique1 ) < std::tie( alpha_to_coverage2, animated2, display_technique2 );
        }


        RenderComponentContainer const * m_component_container;
        ShaderContainer const * m_shader_container;
    };



    void MergeAllComponentIndices(
        Range<uint32_t const *> color_render_component_indices,
        Range<uint32_t const *> shadow_render_component_indices_offsets,
        Range<uint32_t const *> shadow_render_component_indices,
        size_t max_possible_components,
        std::vector<uint32_t> & all_to_be_rendered_components
        )
    {
        assert(Size(color_render_component_indices) <= max_possible_components);

        ResetSize(all_to_be_rendered_components, max_possible_components * 2);
        auto indices_so_far_begin = begin(all_to_be_rendered_components);
        auto new_indices_begin = begin(all_to_be_rendered_components) + max_possible_components;

        Copy(begin(color_render_component_indices), Size(color_render_component_indices), &*indices_so_far_begin);
        auto indices_so_far_end = indices_so_far_begin + Size(color_render_component_indices);

        for( auto shadow_index = 0u; shadow_index + 1 < Size(shadow_render_component_indices_offsets); ++shadow_index )
        {
            auto shadow_render_component_indices_for_one_map = CreateRange(
                shadow_render_component_indices,
                shadow_render_component_indices_offsets[shadow_index], shadow_render_component_indices_offsets[shadow_index + 1]
                );
            auto new_indices_end = std::set_union(
                indices_so_far_begin, indices_so_far_end,
                begin(shadow_render_component_indices_for_one_map), end(shadow_render_component_indices_for_one_map),
                new_indices_begin
                );
            swap(new_indices_begin, indices_so_far_begin);
            indices_so_far_end = new_indices_end;
        }
        // first erase everything after what we need
        all_to_be_rendered_components.erase(indices_so_far_end, end(all_to_be_rendered_components));
        // then erase stuff before what we need if it exists (less copying this way)
        all_to_be_rendered_components.erase(begin(all_to_be_rendered_components), indices_so_far_begin);
    }
}


void RenderWorld::RenderFor( double start_time, float time_step, IndexedOrientations const & orientations, IndexedOffsetPoses const & poses, Orientation new_camera_orientation, Orientation previous_camera_orientation, PerspectiveViewParameters perspective_view_parameters )
{
    m_loop_timer.Start();

    if(m_world_configuration.render_external_debug_components)
    {
        Append(m_external_debug_component_container, m_component_container);
    }

    std::vector<Math::Float4x4> transforms( Size( orientations.orientations ) );
    CreateTransforms( orientations.orientations, -new_camera_orientation.position, transforms );

    auto new_camera_matrix = RotationToFloat3x3( Conjugate(new_camera_orientation.rotation) );

    std::vector<Math::Float4x4> previous_transforms( Size( orientations.previous_orientations ) );
    CreateTransforms( orientations.previous_orientations, -previous_camera_orientation.position, previous_transforms );

    auto previous_camera_matrix = RotationToFloat3x3( Conjugate(previous_camera_orientation.rotation) );

    auto projection_matrix = PerspectiveFieldOfViewVertical( perspective_view_parameters );

    std::vector<Math::Float4x4> bone_states( Size( poses.bone_states ) );
    CreateTransforms( poses.bone_states, bone_states );
    std::vector<Math::Float4x4> previous_bone_states( Size( poses.previous_bone_states ) );
    CreateTransforms( poses.previous_bone_states, previous_bone_states );

    std::vector<Math::Float4x4> const & component_offset_transforms = m_component_container.orientation_offsets;

    std::vector<Math::Float4x4> blended_transforms( Size( transforms ) );
    std::vector<Math::Float4x4> blended_bone_states( Size( bone_states ) );

    std::vector<Math::Float4x4> ordered_blended_transforms( Size( m_component_container.entity_ids ) );

    std::vector<Math::Float4x4> light_matrices( Size(m_light_container.entity_ids) * m_light_container.shadow_map_count );
    std::vector<uint32_t> color_render_component_indices, shadow_render_component_indices_offsets, shadow_render_component_indices, all_to_be_rendered_components(m_component_container.entity_ids.size());

    std::vector<BoundingShapes::AxisAlignedBox> transformed_boxes( Size( m_component_container.bounding_boxes ) );

    // render at least once
    auto blend_factor = Math::Min( float( start_time ) / time_step, 1.0f );
    //auto blend_factor = float( start_time ) / time_step;

    auto rendered_frames = 0u;
    for( ; blend_factor <= 1; m_loop_timer.Stop(), blend_factor = float( m_loop_timer.GetSeconds() + start_time ) / time_step, ++rendered_frames,
         Log( [&]()
    {
        return "Total rendering time this tick: " + std::to_string( this->m_loop_timer.GetMilliSeconds() ) + " ms";
    } ) )
    {
        BlendTransforms( transforms, previous_transforms, blend_factor, blended_transforms );

        BlendTransforms( bone_states, previous_bone_states, blend_factor, blended_bone_states );

        OrderData<Math::Float4x4>( m_component_container.entity_ids, blended_transforms, orientations.indices, ordered_blended_transforms );

        for (auto i = 0u; i < ordered_blended_transforms.size(); i++)
        {
            ordered_blended_transforms[i] *= component_offset_transforms[i];
        }

        auto const blended_camera_matrix = Lerp( previous_camera_matrix, new_camera_matrix, blend_factor );

        auto camera_projection = projection_matrix * CombineFloat3x3AndTranslation(blended_camera_matrix, 0);

        FillPerFrameConstantBuffers( blended_camera_matrix, projection_matrix, Lerp(previous_camera_orientation.position, new_camera_orientation.position, blend_factor) );

        Transform( m_component_container.bounding_boxes, ordered_blended_transforms, transformed_boxes );
        color_render_component_indices.clear();
        BoundingShapes::IntersectFrustum( transformed_boxes, camera_projection, color_render_component_indices );

        // we support only one light
        assert( m_light_container.entity_ids.size() == 1 );
        shadow_render_component_indices_offsets.clear();
        shadow_render_component_indices_offsets.resize( Size( light_matrices ) + 1, 0 );
        shadow_render_component_indices.clear();
        Math::Quaternion light_rotation;
        BlendRotations( m_light_container.entity_ids, orientations, blend_factor, CreateRange(&light_rotation, 1) );
        auto light_buffer = m_constant_buffer_container.GetBuffer(m_light_container.constant_buffers[0]);
        FillLightBuffer(
            light_rotation,
            m_light_container.colors[0],
            transformed_boxes,
            perspective_view_parameters,
            CombineFloat3x3AndTranslation(blended_camera_matrix, 0),
            m_immediate_context,
            light_buffer,
            light_matrices,
            shadow_render_component_indices_offsets,
            shadow_render_component_indices );

        MergeAllComponentIndices(
            color_render_component_indices,
            shadow_render_component_indices_offsets,
            shadow_render_component_indices,
            Size(m_component_container.entity_ids),
            all_to_be_rendered_components
            );


        FillObjectBuffers(all_to_be_rendered_components, m_component_container.components, ordered_blended_transforms, m_constant_buffer_container, m_immediate_context);
        FillBoneBuffers(
            all_to_be_rendered_components, m_component_container.components, m_component_container.entity_ids,
            poses.pose_from_entity, poses.pose_offsets, blended_bone_states,
            m_constant_buffer_container, m_immediate_context
            );


        {
            TargetsAndStates targets_and_states;
            targets_and_states.depth_stencil_state = m_light_container.depth_stencil_state;
            // SCAMP SCAMP
            targets_and_states.rasterizer_state = m_light_container.rasterizer_state;
            targets_and_states.render_views.fill( nullptr );
            targets_and_states.render_view_count = 0;

            // SCAMP ?
            for( auto light_index : CreateIntegerRange( m_light_container.entity_ids.size() ) )
            {
                targets_and_states.view_ports.clear();
                targets_and_states.view_ports.emplace_back( m_light_container.view_ports[light_index] );

                for( auto map_index = 0u; map_index < m_light_container.shadow_map_count; ++map_index )
                {
                    auto shadow_render_component_indices_for_this_map = CreateRange( shadow_render_component_indices, shadow_render_component_indices_offsets[map_index], shadow_render_component_indices_offsets[map_index + 1] );
                    // std::sort( begin( shadow_render_component_indices_for_this_map ), end( shadow_render_component_indices_for_this_map ), ShadowSorter( &m_component_container, &m_shader_container ) );

                    auto depth_stencil_index = light_index * m_light_container.shadow_map_count + map_index;
                    targets_and_states.depth_stencil_view = m_resource_view_container.GetDepthStencilView( m_light_container.depth_stencil_views[depth_stencil_index] );

                    ClearAndSet( m_immediate_context, targets_and_states );

                    auto single_light_transform_buffer_id = m_light_container.transform_constant_buffers[depth_stencil_index];
                    auto single_light_transform_buffer = m_constant_buffer_container.GetBuffer(single_light_transform_buffer_id);
                    FillConstantBufferWithData( single_light_transform_buffer, m_immediate_context, ConstantBuffers::SingleLightTransform{ light_matrices[depth_stencil_index] } );

                    std::array<ConstantBufferTypeAndID, 4> extra_buffers;
                    FillWithGlobalConstantBufferIDs(CreateRange(extra_buffers, 0, 3));
                    extra_buffers[3] = {ConstantBufferType::SingleLightTransform, single_light_transform_buffer_id};

                    Render(
                        m_component_container.shadow_display_techniques,
                        shadow_render_component_indices_for_this_map,

                        extra_buffers
                    );
                }
            }
        }



        //static auto scamp = true;

        //if( scamp )
        //{
        //    FillConstantBuffer<ConstantBuffers::Projection>( constant_buffers.projection_buffer, m_immediate_context, [&light_matrices, blended_camera_matrix]( ConstantBuffers::Projection* buffer )
        //    {
        //        buffer->matrix = light_matrices[0] * Inverse(blended_camera_matrix);
        //    } );

        //    //FillConstantBuffer<ConstantBuffers::Camera>( constant_buffers.camera_buffer, m_immediate_context, []( ConstantBuffers::Camera* buffer )
        //    //{
        //    //    buffer->matrix = Math::Float4x4( Math::Identity() );
        //    //} );
        //}

        {
            TargetsAndStates targets_and_states;
            targets_and_states.depth_stencil_state = m_default_depth_stencil_state;
            targets_and_states.rasterizer_state = m_default_rasterizer_state;

            targets_and_states.render_views[0] = m_resource_view_container.GetRenderTargetView( m_final_render_target_view );
            targets_and_states.render_view_count = 1;
            targets_and_states.depth_stencil_view = m_resource_view_container.GetDepthStencilView( m_depth_stencil_view );
            auto back_buffer = m_swap_chain.GetBuffer( 0 );
            CD3D11_VIEWPORT viewport( back_buffer, targets_and_states.render_views[0] );
            targets_and_states.view_ports.emplace_back( viewport );

            std::sort( begin( color_render_component_indices ), end( color_render_component_indices ), ColorRenderSorter(&m_component_container, &m_shader_container) );

            ClearAndSet( m_immediate_context, targets_and_states );

            std::array<ConstantBufferTypeAndID, 4> extra_buffers;
            FillWithGlobalConstantBufferIDs(CreateRange(extra_buffers, 0, 3));
            extra_buffers[3] = {ConstantBufferType::Light, m_light_container.constant_buffers[0]};

            Render(
                m_component_container.display_techniques,
                color_render_component_indices,

                extra_buffers
                );
        }

        RenderSky();

        m_swap_chain.Present();
    }

    Log( [rendered_frames]()
    {
        return "Rendered " + std::to_string( rendered_frames ) + " frames this tick.";
    } );

    // remove all external debug render components again
    if(m_world_configuration.render_external_debug_components)
    {
        Shrink(ComponentCount(m_external_debug_component_container), m_component_container);
    }
    // always clear, in case components were added but we didn't render them
    // also release all resources, because we know they are not shared with anything else
    ReleaseAllResources(m_external_debug_component_container.components, m_vertex_buffer_container, m_index_buffer_container, m_texture_container, m_resource_view_container, m_constant_buffer_container);
    Clear(m_external_debug_component_container);
}



void RenderWorld::CreateDefaultStates()
{
    m_alpha_coverage_blend_state = m_device.CreateAlphaCoverageBlendState();
    m_default_blend_state = m_device.CreateBlendState();
    m_default_depth_stencil_state = m_device.CreateDepthStencilState();
    auto rasterizer_description = m_device.GetDefaultRasterizerDescription();
    if(m_world_configuration.force_wire_frame)
    {
        rasterizer_description.FillMode = D3D11_FILL_WIREFRAME;
    }
    m_default_rasterizer_state = m_device.CreateRasterizerState(rasterizer_description);
}


namespace
{
    uint32_t GetVertexBufferIDIndex( VertexBufferType const type )
    {
        switch( type )
        {
        case Graphics::VertexBufferType::Position:
            return c_positions_index;
        case Graphics::VertexBufferType::Normal:
            return c_normals_index;
        case Graphics::VertexBufferType::Tangent:
            return c_tangent_index;
        case Graphics::VertexBufferType::Texture:
            return c_texture_index;
        case Graphics::VertexBufferType::Color:
            return c_colors_index;
        case Graphics::VertexBufferType::Bone:
            return c_bone_index;
        case Graphics::VertexBufferType::WorldMatrix:
        default:
            assert( false );
            return std::numeric_limits<uint32_t>::max();
        }
    }

    IndexBufferInfo SetIndexBufferAndTopology( IndexBufferID index_id, DeviceContext & context, IndexBufferContainer const & index_buffer_container )
    {
        auto index_info = index_buffer_container.GetIndexBufferInfo( index_id );
        auto index_buffer = index_buffer_container.GetIndexBuffer( index_id );

        context->IASetIndexBuffer( index_buffer, ToDXGIFormat( index_info.type ), index_info.offset );
        context->IASetPrimitiveTopology( index_info.topology );

        return index_info;
    }


    void SetVertexBuffers( Range<VertexBufferID const *> vertex_ids, Range<VertexBufferType const*> vertex_buffer_types, DeviceContext & context, VertexBufferContainer const & vertex_buffer_container)
    {
        // set IA stuff
        // vertex buffers
        auto vertex_buffer_count = uint32_t( Size(vertex_buffer_types) );
        std::array<ID3D11Buffer*, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> vertex_buffers;
        std::array<uint32_t, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> strides;
        std::array<uint32_t, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> offsets;
        for(auto index = 0u; index < vertex_buffer_count; ++index )
        {
            auto const type = vertex_buffer_types[index];
            auto system_generated = (type == VertexBufferType::SystemGenerated);
            if(!system_generated)
            {
                auto vertex_buffer_index = GetVertexBufferIDIndex(type);
                // skip if we don't have to set anything, either System generated or instanced
                assert(vertex_buffer_index < Size(vertex_ids));
                auto vertex_info = vertex_buffer_container.GetBufferInfo(vertex_ids[vertex_buffer_index]);
                assert(vertex_info.type == type);

                vertex_buffers[index] = vertex_buffer_container.GetBuffer(vertex_ids[vertex_buffer_index]);
                offsets[index] = vertex_info.offset;
            }
            else
            {
                vertex_buffers[index] = nullptr;
                offsets[index] = 0;
            }
            strides[index] = GetStride(type);
        }

        context->IASetVertexBuffers( 0, vertex_buffer_count, vertex_buffers.data(), strides.data(), offsets.data() );
    }


    uint32_t GetInstanceBufferIndex( VertexBufferType const type )
    {
        switch( type )
        {
        case Graphics::VertexBufferType::Color:
            return 1;
        case Graphics::VertexBufferType::Position:
        case Graphics::VertexBufferType::WorldMatrix:
            return 0;
        default:
            assert( false );
            return std::numeric_limits<uint32_t>::max();
        }
    }

    void SetInstanceBuffers( Range<ID3D11Buffer* const *> buffers, Range<uint32_t const*> buffer_data_offets,  Range<VertexBufferType const *> buffer_types, uint32_t slot_offset, DeviceContext & context )
    {
        auto const vertex_buffer_count = uint32_t( Size(buffer_types) );
        std::array<ID3D11Buffer*, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> vertex_buffers;
        std::array<uint32_t, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> strides;
        std::array<uint32_t, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> offsets;
        for( auto index : CreateIntegerRange( vertex_buffer_count ) )
        {
            auto const type = buffer_types[index];
            auto system_generated = type == Graphics::VertexBufferType::SystemGenerated;
            if(!system_generated)
            {
                auto vertex_buffer_index = GetInstanceBufferIndex(type);
                // skip if we don't have to set anything, either System generated or instanced
                assert(vertex_buffer_index < Size(buffers));

                vertex_buffers[index] = buffers[vertex_buffer_index];
                strides[index] = GetStride(type);
                offsets[index] = buffer_data_offets[vertex_buffer_index];
            }
            else
            {
                vertex_buffers[index] = nullptr;
                strides[index] = 0;
                offsets[index] = 0;
            }
        }

        context->IASetVertexBuffers( slot_offset, vertex_buffer_count, vertex_buffers.data(), strides.data(), offsets.data() );
    }

}


void RenderWorld::RenderSky()
{
    SetShaders(m_sky_display_technique, m_shader_container, m_immediate_context);
    std::array<ConstantBufferTypeAndID, 5> buffer_stuff;
    buffer_stuff[0] = {ConstantBufferType::Sky, m_sky_buffer_id};
    buffer_stuff[1] = {ConstantBufferType::Light, m_light_container.constant_buffers[0]};
    FillWithGlobalConstantBufferIDs(CreateRange(buffer_stuff, 2, Size(buffer_stuff)));
    SetConstantBuffers( buffer_stuff, m_sky_display_technique, m_constant_buffer_container, m_shader_container, m_immediate_context );
    m_immediate_context->OMSetBlendState( m_default_blend_state, nullptr, D3D11_DEFAULT_SAMPLE_MASK );
    m_immediate_context->IASetIndexBuffer( nullptr, DXGI_FORMAT(0), 0 );
    m_immediate_context->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    m_immediate_context->IASetVertexBuffers( 0, 0, nullptr, nullptr, nullptr );
    m_immediate_context->Draw( 6, 0 );
}


void RenderWorld::Render(
    Range<DisplayTechnique const *> const display_techniques,
    Range<uint32_t const *> component_indices,

    Range<ConstantBufferTypeAndID const *> additional_constant_buffers
    )
{
    Log( [component_indices]()
    {
        return "Rendering " + std::to_string( Size(component_indices) ) + " components.";
    } );

    using namespace std::rel_ops;
    Mesh last_used_mesh;
    last_used_mesh.index_id = c_invalid_index_buffer_id;
    last_used_mesh.vertex_ids.fill( c_invalid_vertex_buffer_id );

    DisplayTechnique last_used_display_technique( c_invalid_shader_id );

    Range<TextureTypeAndID const *> last_used_component_textures;
    auto changed_shaders = true;

    InputLayoutID last_used_input_layout_id = c_invalid_layout_id;
    Range<VertexBufferType const *> last_used_vertex_input_buffer_types = {nullptr, nullptr};

    IndexBufferInfo last_used_index_buffer_info;
    last_used_index_buffer_info.index_count = 0;

    std::vector<ConstantBufferTypeAndID> last_used_constant_buffers;
    std::array<VertexBufferID,2> instance_transform_buffers = { { c_invalid_vertex_buffer_id, c_invalid_vertex_buffer_id } };

    bool is_instanced = false;

    bool alpha_to_coverage = false;
    m_immediate_context->OMSetBlendState( m_default_blend_state, nullptr, D3D11_DEFAULT_SAMPLE_MASK );

    for( auto i : component_indices )
    {
        auto const & display_technique = display_techniques[i];
        if( !m_shader_container.IsValid( display_technique.vertex_shader_id ) &&
            // rest
            !m_shader_container.IsValid( display_technique.pixel_shader_id ) )
        {
            continue;
        }

        bool current_alpha_to_coverage = !!m_component_container.alpha_to_coverage[i];
        if( current_alpha_to_coverage && !alpha_to_coverage )
        {
            m_immediate_context->OMSetBlendState( m_alpha_coverage_blend_state, nullptr, D3D11_DEFAULT_SAMPLE_MASK );
        }
        else if( !current_alpha_to_coverage && alpha_to_coverage )
        {
            m_immediate_context->OMSetBlendState( m_default_blend_state, nullptr, D3D11_DEFAULT_SAMPLE_MASK );
        }
        alpha_to_coverage = current_alpha_to_coverage;


        if( display_technique != last_used_display_technique )
        {
            last_used_display_technique = display_technique;
            auto input_layout_id = SetShaders( last_used_display_technique, m_shader_container, m_immediate_context );
            if( input_layout_id != last_used_input_layout_id )
            {
                last_used_input_layout_id = input_layout_id;
                last_used_vertex_input_buffer_types = m_shader_container.GetVertexBufferTypes( last_used_input_layout_id );
                is_instanced = !m_shader_container.GetInstanceBufferTypes( last_used_input_layout_id ).empty();
            }

            changed_shaders = true;
        }


        auto const & component = m_component_container.components[i];
        if( changed_shaders || !Math::Equal(CreateRange(component.textures), last_used_component_textures, std::equal_to<>()))
        {
            changed_shaders = false;
            last_used_component_textures = component.textures;

            // textures
            std::array<ID3D11ShaderResourceView*, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT> shader_resource_view_array;
            std::array<TextureType, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT> texture_types;
            // SCAMP ?
            auto shadow_textures_size = uint32_t( m_light_container.shader_resource_views.size() );
            for( auto index : CreateIntegerRange( shadow_textures_size ) )
            {
                shader_resource_view_array[index] = m_resource_view_container.GetShaderResourceView( m_light_container.shader_resource_views[index] );
                texture_types[index] = TextureType::Shadow;
            }

            auto const texture_list_size = uint32_t( Size(last_used_component_textures) );
            for( auto const index : CreateIntegerRange( texture_list_size ) )
            {
                auto const & texture = last_used_component_textures[index];
                shader_resource_view_array[index + shadow_textures_size] = m_resource_view_container.GetShaderResourceView( texture.view_id );
                texture_types[index + shadow_textures_size] = texture.type;
            }
            SetTextures( m_immediate_context, last_used_display_technique, m_shader_container, reinterpret_cast<ID3D11ShaderResourceView* const *>( shader_resource_view_array.data() ), texture_types.data(), texture_list_size + shadow_textures_size );
        }

        if( changed_shaders || component.buffers != last_used_constant_buffers )
        {
            last_used_constant_buffers = component.buffers;
            Append(last_used_constant_buffers, additional_constant_buffers);
            SetConstantBuffers( last_used_constant_buffers, last_used_display_technique, m_constant_buffer_container, m_shader_container, m_immediate_context );
            ::Shrink(last_used_constant_buffers, Size(additional_constant_buffers));
        }

        if( changed_shaders || component.mesh != last_used_mesh )
        {
            last_used_mesh = component.mesh;
            last_used_index_buffer_info = SetIndexBufferAndTopology( last_used_mesh.index_id, m_immediate_context, m_index_buffer_container );
            SetVertexBuffers( last_used_mesh.vertex_ids, last_used_vertex_input_buffer_types, m_immediate_context, m_vertex_buffer_container );
        }

        instance_transform_buffers = m_component_container.instance_transform_buffers[i];

        if( instance_transform_buffers != std::array<VertexBufferID, 2>{{ c_invalid_vertex_buffer_id, c_invalid_vertex_buffer_id }} )
        {
            assert(is_instanced);
            uint32_t size = 0;
            std::array<uint32_t, 2> offsets = { 0, 0 };
            auto const & instance_buffer_types = m_shader_container.GetInstanceBufferTypes( last_used_input_layout_id );

            std::array<ID3D11Buffer*, 2> instance_buffers = { { nullptr, nullptr } };
            uint32_t instance_buffers_count = 0;

            size = 0;
            assert( instance_buffers.size() >= instance_transform_buffers.size() );
            auto max = instance_transform_buffers.size();
            auto buffer_index = 0u;
            auto itb_index = 0u;
            while( itb_index < max )
            {
                if(m_vertex_buffer_container.IsValid(instance_transform_buffers[itb_index]))
                {
                    auto buffer_info = m_vertex_buffer_container.GetBufferInfo(instance_transform_buffers[0]);
                    assert(buffer_info.vertex_count != 0 && (size == 0 || size == buffer_info.vertex_count));
                    size = buffer_info.vertex_count;
                    offsets[buffer_index] = buffer_info.offset;
                    instance_buffers[buffer_index] = m_vertex_buffer_container.GetBuffer(instance_transform_buffers[itb_index]);
                    buffer_index += 1;
                }
                itb_index += 1;
            }
            instance_buffers_count = buffer_index;

            // // SCAMP do pseudo instancing
            // assert(instance_buffers_count == 1);
            // for (auto b = 0u; b < size; ++b)
            // {
            //     uint32_t offset = offsets[0] + b * sizeof(HLSL::float4x4);
            //     SetInstanceBuffers( CreateRange( instance_buffers.data(), instance_buffers_count ), CreateRange(&offset, 1), instance_buffer_types, uint32_t(Size(last_used_vertex_input_buffer_types)), m_immediate_context );
            //     m_immediate_context->DrawIndexed( last_used_index_buffer_info.index_count, 0, 0 );
            // }

            SetInstanceBuffers( CreateRange( instance_buffers.data(), instance_buffers_count ), offsets, instance_buffer_types, uint32_t(Size(last_used_vertex_input_buffer_types)), m_immediate_context );

            m_immediate_context->DrawIndexedInstanced( last_used_index_buffer_info.index_count, size, 0, 0, 0 );
        }
        else
        {
            m_immediate_context->DrawIndexed( last_used_index_buffer_info.index_count, 0, 0 );
        }
    }
}


void RenderWorld::FillWithGlobalConstantBufferIDs(Range<ConstantBufferTypeAndID *> buffers) const
{
    buffers[0] = {ConstantBufferType::Camera, m_camera_buffer_id};
    buffers[1] = {ConstantBufferType::Projection, m_projection_buffer_id};
    buffers[2] = {ConstantBufferType::Sky, m_sky_buffer_id};
}


void RenderWorld::CreateGlobalConstantBuffers()
{
    uint32_t size = sizeof( ConstantBuffers::Projection );
    auto projection_buffer = m_device.CreateConstantBuffer( size );
    m_projection_buffer_id = m_constant_buffer_container.Add( projection_buffer );
    size = sizeof( ConstantBuffers::Camera );
    auto camera_buffer = m_device.CreateConstantBuffer( size );
    m_camera_buffer_id = m_constant_buffer_container.Add( camera_buffer );
    size = sizeof( ConstantBuffers::Sky );
    auto sky_buffer = m_device.CreateConstantBuffer( size );
    m_sky_buffer_id = m_constant_buffer_container.Add( sky_buffer );
}


void RenderWorld::FillPerFrameConstantBuffers( Math::Float3x3 camera_matrix, Math::Float4x4 projection_matrix, Math::Float3 camera_position )
{
    auto projection_buffer = m_constant_buffer_container.GetBuffer( m_projection_buffer_id );
    auto camera_buffer = m_constant_buffer_container.GetBuffer( m_camera_buffer_id );
    auto sky_buffer = m_constant_buffer_container.GetBuffer( m_sky_buffer_id );

    FillConstantBufferWithData( projection_buffer, m_immediate_context, ConstantBuffers::Projection{ projection_matrix } );
    FillConstantBufferWithData( camera_buffer, m_immediate_context, ConstantBuffers::Camera{ Inverse(projection_matrix * CombineFloat3x3AndTranslation(camera_matrix, 0)), camera_matrix, camera_position } );
    FillConstantBufferWithData( sky_buffer, m_immediate_context, ConstantBuffers::Sky{m_sky_color});
}


void RenderWorld::AdjustAllPositions(Math::Float3 adjustment)
{
    m_terrain_3d_data.real_center += adjustment;
    m_terrain_2d_data.center += adjustment;
}
