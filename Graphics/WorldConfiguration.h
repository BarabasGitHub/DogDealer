#pragma once

namespace Graphics
{
    enum struct TextureFiltering;

    struct WorldConfiguration
    {
        unsigned render_sample_count;
        TextureFiltering maximum_texture_filtering_quality;
        bool display_bounding_boxes;
        bool force_wire_frame;
        bool render_external_debug_components;
    };
}
