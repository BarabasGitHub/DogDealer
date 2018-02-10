float CalculateDistanceAlpha(float4 screenspace_position)
{
    float far = 0.02;
    float close = 0.03;
    float2 band = float2(far, close);
    band *= screenspace_position.w;
    return smoothstep(band.x, band.y, screenspace_position.z);
}
