#pragma once

namespace Graphics{

	enum struct ShaderType {
        Vertex, 
        Hull, 
        Domain,
        Geometry, 
        Pixel, 
        Compute,
        None
    };
}