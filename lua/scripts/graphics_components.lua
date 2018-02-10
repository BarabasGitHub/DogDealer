require "table_functions"
require "graphics_techniques"

PlainColorMaterial = {
	technique = PlainColorTechnique;
	shadow_technique = ShadowTechnique;
}

PlainShadowedMaterial = {
	technique = ShadowedColorTechnique;
	shadow_technique = ShadowTechnique;
}

TerrainMaterial3D = {
	technique = PlanarTextureTechnique;
	texture = {
		diffuse = "grass_ground.dds";
	},
	shadow_technique = ShadowTechnique;
}

TerrainMaterial2D = {
	-- technique = TerrainTechnique;
	technique = TerrainNormalMapTechnique;
	texture = {
		diffuse = "grass_ground.dds";
		normal = "grass_ground_normal.dds";
	},
	shadow_technique = TerrainShadowTechnique;
}

NoiseShadowedMaterial = {
	technique = ShadowedNoiseTechnique;
	shadow_technique = ShadowTechnique;
}

SlabMaterial =
{
	technique = NormalMapTechnique;
	texture = {
		diffuse = "slab.dds";
		normal = "slab_normal.dds";
	},
	shadow_technique = ShadowTechnique;
}
TomatoMaterial = {
	technique = TextureTechnique;
	textures = {
		diffuse = "tomato.dds";
	};
	shadow_technique = ShadowTechnique;
}
GrassMaterial = {
	technique = LocalInstancedPremultipliedAlphaTechnique;

	textures = {
		diffuse = "grass.dds";
	};
	-- shadow_technique = LocalInstancedAlphaShadowTechnique;
	alpha_to_coverage = true;
}

BarkMaterial = {
	technique = NormalMapTechnique;
	textures = {
		diffuse = "beech_bark.dds";
		normal = "beech_bark_normal.dds";
	},
	shadow_technique = ShadowTechnique;
}

LeavesMaterial = {
	technique = InstancedPremultipliedAlphaTechnique;
	textures = {
		diffuse = "leaves.dds";
	};
	shadow_technique = InstancedAlphaShadowTechnique;
	alpha_to_coverage = true;
}

BrickWallMaterial = {
	technique = NormalMapTechnique;
	-- technique = TextureTechnique;
	textures = {
		diffuse = "brick_wall.dds";
		normal = "brick_wall_normal.dds";
	};
	shadow_technique = ShadowTechnique;
}

HedgeMaterial = {
	technique = TextureTechnique;
	textures = {
		diffuse = "hedge.dds";
	};
	shadow_technique = ShadowTechnique;
}

GraphicsComponents = {
	Slab =
	{
		mesh = "slab";
		material = SlabMaterial;
	},
	AxesOrientation =
	{
		mesh = "axis_model_small";
		material = PlainShadowedMaterial;
	},
	Terrain =
	{
		mesh = "terrain";
		material = SlabMaterial;
	},
	Tomato =
	{
		mesh = "tomato";
		material = TomatoMaterial;
	},
	GrassSubtle =
	{
		mesh = "grass_subtle";
		material = GrassMaterial;
	},
	GrassUpright =
	{
		mesh = "grass_upright";
		material = GrassMaterial;
	},
	{
		mesh = "doesn't matter";
		material = TerrainMaterial3D; --PlainShadowedMaterial;
		--material = PlainShadowedMaterial;
	},
	GeneratedTerrain2D =
	{
		mesh = "doesn't matter";
		material = TerrainMaterial2D; --PlainShadowedMaterial;
		--material = PlainShadowedMaterial;
	},
	Tree =
	{
		mesh = "tree";
		material = BarkMaterial;
	},
	Leaves =
	{
		mesh = "leaves";
		material = LeavesMaterial;
	},
	TreeTomatoes =
	{
		mesh = "tree_tomatoes";
		material = TomatoMaterial;
	},
	Cube =
	{
		-- mesh = "boxes_mesh";
		mesh = "cube";
		material = PlainShadowedMaterial;
	},
	Sphere =
	{
		mesh = "sphere";
		material = PlainShadowedMaterial;
	},
	Ramp =
	{
		mesh = "ramp";
		material = NoiseShadowedMaterial;
	},
	ObstacleArc =
	{
		mesh = "obstacle_arc";
		material = NoiseShadowedMaterial;
	},
	ObstacleGaps =
	{
		mesh = "obstacle_gaps";
		material = NoiseShadowedMaterial;
	},
	ObstacleSlope =
	{
		mesh = "obstacle_slope";
		material = NoiseShadowedMaterial;
	},
	ObstacleStairs =
	{
		mesh = "obstacle_stairs";
		material = NoiseShadowedMaterial;
	},
	Wall =
	{
		mesh = "wall";
		material = BrickWallMaterial;
	},
	WallCorner =
	{
		mesh = "wall_corner";
		material = BrickWallMaterial;
	},
	WallStairs =
	{
		mesh = "wall_stairs";
		material = BrickWallMaterial;
	},
	WallGate =
	{
		mesh = "wall_gate";
		material = BrickWallMaterial;
	},
	WallTower =
	{
		mesh = "wall_tower";
		material = BrickWallMaterial;
	},
	Multitower =
	{
		mesh = "multitower";
		material = PlainShadowedMaterial;
	},
	NavigationMesh =
	{
		mesh = "navigation_multitower";
		material = PlainShadowedMaterial;
	},
	Hedge =
	{
		mesh = "hedge";
		material = HedgeMaterial;
	},
	Sheep =
	{
		mesh = "sheep";
		material = PlainShadowedMaterial;
	},
}

local debug_meshes = {"debug_sphere"}

function GetAllMeshNames()
	local mesh_files = ShallowCopyTable(debug_meshes)
	for k,v in pairs(GraphicsComponents) do
	  if v.mesh ~= nil then
	    mesh_files[#mesh_files + 1] = v.mesh
	  end
	end
	return mesh_files
end
