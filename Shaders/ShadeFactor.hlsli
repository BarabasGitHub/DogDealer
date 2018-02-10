#include "LightBuffer.hlsli"

float ShadeFactor( float3 normal )
{
    return saturate( dot( normal, LightNormal ) );
}