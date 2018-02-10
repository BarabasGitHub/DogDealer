#pragma once
#include "TextureType.h"

#include <Math\FloatTypes.h>

#include <string>
#include <utility> // for pair
#include <vector>

namespace Graphics{

	typedef std::vector<std::pair<TextureType, std::string>> TextureNames;

	struct DisplayTechniqueDescription
	{
		std::string vertex_shader;
		std::string pixel_shader;
	};

	struct RenderComponentDescription
	{
		std::string name;

		std::string mesh;

		DisplayTechniqueDescription technique;
		TextureNames textures;

        DisplayTechniqueDescription shadow_technique;

        bool alpha_to_coverage = false;
	};

    struct LightDescription
    {
        Math::Float3 color;
        // type of light, maybe color?
    };
}