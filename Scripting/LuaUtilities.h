#pragma once

#include <Utilities\Handle.h>
#include <Utilities\SimplexNoise.h>

#include <vector>

//#include <Conventions\AnimationType.h>

// forward declarations
struct lua_State;
struct EntityDescription;

namespace Graphics
{
    struct RenderComponentDescription;
    struct WorldConfiguration;
}

namespace Physics
{
    struct WorldConfiguration;
}

namespace Animating
{
	struct ComponentDescription;
}

void ReadGraphicsConfiguration( lua_State * L, int table_index, Graphics::WorldConfiguration& configuration );
void ReadPhysicsConfiguration( lua_State * L, int table_index, Physics::WorldConfiguration& configuration );

std::vector<Graphics::RenderComponentDescription> ReadRenderComponents(lua_State * L, int table_index );
Animating::ComponentDescription ReadAnimatingComponent(lua_State * L, int table_index );

EntityDescription ReadEntityDescription( lua_State * L, int index );

void ReadNoiseParameters( lua_State * L, int index, std::vector<NoiseParameters<Math::Float3>>& noise3d );
void ReadNoiseParameters( lua_State * L, int index, std::vector<NoiseParameters<Math::Float2>>& noise2d );
// nullptr <-> nill specialization ?
