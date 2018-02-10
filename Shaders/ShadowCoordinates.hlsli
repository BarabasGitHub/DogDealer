#ifndef SHADOW_COORDINATES_GUARD
#define SHADOW_COORDINATES_GUARD

#include "LightBuffer.hlsli"
#include "Transforms.hlsli"

void ShadowCoordinates( in Position position_in, out Position output[c_max_number_of_light_transforms] )
{
    for(uint i = 0; i < c_max_number_of_light_transforms; ++i)
    {
        // first calculate the position on the light screen
        ProjectedPosition position = ProjectPosition( position_in, LightMatrices[i] );
        // then transform the screen space coordinates to texture coordinates
        position.xyz /= position.w;
        position.y = -position.y;
        position.xy *= .5;
        position.xy += .5;
        output[i] = position.xyz;
    }
}

#endif