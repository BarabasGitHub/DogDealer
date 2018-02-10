#include "PositionTextureStruct.hlsli"

SamplerState LinearSampler;
Texture2D DiffuseMap;

void main( PositionTexture input )
{
    float alpha = DiffuseMap.Sample( LinearSampler, input.uv ).a;
    clip( alpha - 0.5 );
    return;
}