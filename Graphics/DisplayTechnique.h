#pragma once

#include "IDs.h"

#include <tuple> // for tie

namespace Graphics
{
    struct DisplayTechnique
    {
        VertexShaderID vertex_shader_id;
        HullShaderID hull_shader_id;
        DomainShaderID domain_shader_id;
        GeometryShaderID geometry_shader_id;
        PixelShaderID pixel_shader_id;

        DisplayTechnique( ) = default;
        explicit DisplayTechnique( UniformHandle invalid_shader_id ) :
            vertex_shader_id( ToHandle<VertexShaderID>( invalid_shader_id ) ),
            hull_shader_id( ToHandle<HullShaderID>( invalid_shader_id ) ),
            domain_shader_id( ToHandle<DomainShaderID>( invalid_shader_id ) ),
            geometry_shader_id( ToHandle<GeometryShaderID>( invalid_shader_id ) ),
            pixel_shader_id( ToHandle<PixelShaderID>( invalid_shader_id ) )
        {
        }
    };

    inline bool operator<( DisplayTechnique const & first, DisplayTechnique const & second )
    {
        return
            std::tie( first.hull_shader_id, first.domain_shader_id, first.vertex_shader_id, first.geometry_shader_id, first.pixel_shader_id ) <
            std::tie( second.hull_shader_id, second.domain_shader_id, second.vertex_shader_id, second.geometry_shader_id, second.pixel_shader_id );
    }

    inline bool operator ==( DisplayTechnique const & first, DisplayTechnique const & second )
    {
        return
            first.vertex_shader_id == second.vertex_shader_id &&
            first.hull_shader_id == second.hull_shader_id &&
            first.domain_shader_id == second.domain_shader_id &&
            first.geometry_shader_id == second.geometry_shader_id &&
            first.pixel_shader_id == second.pixel_shader_id;
    }
}