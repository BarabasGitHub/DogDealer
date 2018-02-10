SamplerState NormalMapSampler;
Texture2D NormalMap;


float3 NormalFromNormalMap( float2 uv, float3x3 TBN )
{
    float3 normal = NormalMap.Sample( NormalMapSampler, uv ).rgb;
    normal.xy *= 255.0f / 128.0f;
    normal.xy -= 1.0f;
    normal = normalize( mul( normal, TBN ) );
    return normal;
}

float3 NormalFromNormalMap( float2 uv, float3 tangent, float3 bitangent, float3 normal )
{
    float3x3 TBN = { tangent, bitangent, normal };
    return NormalFromNormalMap( uv, TBN );
}

