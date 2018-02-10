PlainColorTechnique = {
	vertex_shader	= "PlainColorVertexShader";
	hull_shader		= null;
	domain_shader	= null;
	geometry_shader = null;
	pixel_shader	= "PlainColorPixelShader";
	compute_shader	= null;
}


ShadowedColorTechnique = {
	vertex_shader = "ShadowedColorVertexShader";
	pixel_shader = "ShadowedColorPixelShader";
}

TextureTechnique = {
	vertex_shader	= "TexturedShadowedVertexShader";
	pixel_shader	= "TexturedShadowedPixelShader";
}

NormalMapTechnique = {
	vertex_shader	= "NormalMapVertexShader";
	pixel_shader	= "NormalMapPixelShader";
}


ShadowedNoiseTechnique = {
	vertex_shader	= "ShadowedColorWorldVertexShader";
	pixel_shader	= "NoiseShadowedPixelShader";
}

AnimatedTechnique = {
	vertex_shader	= "AnimatedVertexShader";
	pixel_shader	= "PlainColorPixelShader";
}

AnimatedTextureTechnique = {
	vertex_shader	= "AnimatedTexturedShadowedVertexShader";
	pixel_shader	= "TexturedShadowedPixelShader";
}

InstancedPremultipliedAlphaTechnique = {
	vertex_shader = "InstancedTexturedShadowedVertexShader";
	pixel_shader  = "TexturedShadowedPixelShader";
}

-- uses the instanced transform to transform the sub-object instances locally before transforming the complete object
LocalInstancedPremultipliedAlphaTechnique = {
	vertex_shader = "LocalInstancedTexturedShadowedVertexShader";
	pixel_shader  = "TexturedShadowedAlphaDistancePixelShader";
}

PlanarTextureTechnique = {
	vertex_shader	= "PlanarTexturedShadowedVertexShader";
	pixel_shader	= "TexturedShadowedPixelShader";
}

TerrainTechnique = {
	vertex_shader	= "TerrainVertexShader";
	pixel_shader	= "TexturedShadowedPixelShader";
}

TerrainNormalMapTechnique = {
	vertex_shader	= "TerrainTangentVertexShader";
	pixel_shader	= "NormalMapPixelShader";
}


-- shadow casting techniques

ShadowTechnique = {
	vertex_shader = "ShadowVertexShader";
	pixel_shader = "ShadowPixelShader";
}

AnimatedShadowTechnique = {
	vertex_shader = "AnimatedShadowVertexShader";
	pixel_shader = "ShadowPixelShader";
}

AlphaShadowTechnique = {
	vertex_shader = "ShadowTextureVertexShader";
	pixel_shader = "ShadowAlphaPixelShader";
}

InstancedAlphaShadowTechnique = {
	vertex_shader = "InstancedShadowTextureVertexShader";
	pixel_shader = "ShadowAlphaPixelShader";
}

-- uses the instanced transform to transform the sub-object instances locally before transforming the complete object
LocalInstancedAlphaShadowTechnique = {
	vertex_shader = "LocalInstancedShadowTextureVertexShader";
	pixel_shader = "ShadowAlphaPixelShader";
}


TerrainShadowTechnique = {
	vertex_shader	= "ShadowTerrainVertexShader";
	pixel_shader	= "ShadowPixelShader";
}
