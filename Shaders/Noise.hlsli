// N dimensions
// transform: output = input + factor * sum(input)
// skew factor: (sqrt(N + 1) - 1) / N
// unskew factor: (1/sqrt(N + 1) - 1) / N



uint2 ShiftMix( uint2 val )
{
    return val ^ ( val >> 23 );
}

uint ShiftMix( uint val )
{
    return val ^ ( val >> 23 );
}

uint3 ShiftMix( uint3 val )
{
    return val ^ ( val >> 23 );
}

uint4 ShiftMix( uint4 val )
{
    return val ^ ( val >> 23 );
}


uint2 Hash32To32Parallel( uint2 x )
{
    // Murmur-inspired hashing.
    const uint3 kMul = { 0x76E7C763U, 0xcc9e2d51U, 0x1b873593U };
    uint4 ab = x.xxyy * kMul.xyxy + kMul.zxzx;
    ab = ShiftMix( ab );
    ab ^= x.xxyy;
    ab *= kMul.yzyz;
    return ab.xz ^ ab.yw;
}


float2 GetGradients1( float2 vertex )
{
    uint2 hashes = Hash32To32Parallel( asuint( vertex + 0 ) );
    float2 grad = 1.0f + ( hashes & 7 );   // Gradient value 1.0, 2.0, ..., 8.0
    grad *= ( int2( ( hashes & 8 ) / 4 ) - 1 );         // Set a random sign for the gradient
    return grad;
}


float SimplexNoise( float input, out float gradient )
{
    const float radius = 1.f;
    // no skew
    float lattice_point = floor( input );
    float corner = lattice_point; // no unskew

    float2 vertex = { lattice_point, lattice_point + 1 };

    float relative_x = input - corner;

    float2 x = { relative_x, relative_x - 1 }; // no unskew

    float2 distance = radius - x*x;

    float2 distance2 = distance * distance;
    float2 distance4 = distance2 * distance2;

    float2 gradients = GetGradients1( vertex );

    float2 gdotx = gradients * x; // dot in 1d is not much of a dot
    float2 values = distance4 * gdotx;

    gradients = gradients * distance4 - x * ( 8 * gdotx * distance2 * distance );

    gradient = gradients.x + gradients.y;
    // The maximum value of this noise is 8*(3/4)^4 = 2.53125
    // A factor of 0.395 would scale to fit exactly within [-1,1]
    return 0.395f * ( values.x + values.y );

}

static const float skew2d = ( sqrt( 2.f + 1.0f ) - 1.f ) / 2.f;

static const float unskew2d = ( 1.0f - ( 1.0f / sqrt( 2.f + 1.0f ) ) ) / 2.f;
float2 Skew( float2 input )
{
    return input + dot( input, skew2d);
}


float2 Unskew( float2 input )
{
    return input - dot( input, unskew2d);
}

float2 FindCornerVertex( float2 input )
{
    return floor( input );
}

uint Hash64To32( const uint2 x )
{
    // Murmur-inspired hashing.
    const uint2 kMul = { 0xA49A9713, 0x4B9B13BB };
    uint a = ( x.x ^ x.y ) * kMul.x + kMul.y;
    a = ShiftMix( a );
    uint b = ( x.y ^ a ) * kMul.x;
    b = ShiftMix( b );
    b *= kMul.x;
    return b;
}

uint3 Hash64To32Parallel( const uint3 x, const uint3 y )
{
    // Murmur-inspired hashing.
    const uint2 kMul = { 0xA49A9713, 0x4B9B13BB };
    uint3 a = ( x ^ y ) * kMul.x + kMul.y;
    a = ShiftMix( a );
    uint3 b = ( y ^ a ) * kMul.x;
    b = ShiftMix( b );
    b *= kMul.x;
    return b;
}

float SimplexNoise( float2 position, out float2 gradient )
{
    float2 skew = Skew( position );
    float2 lattice_point = FindCornerVertex( skew );
    float2 corner = Unskew( lattice_point );

    float2 relative_position = position - corner;

    const float2 step1 = relative_position.x > relative_position.y ? float2( 1, 0 ) : float2( 0, 1 );

    float2 x0 = relative_position;
    float2 x1 = relative_position - Unskew(step1);
    float2 x2 = relative_position - Unskew(float2(1, 1));

    float2x3 x = {
        x0.x, x1.x, x2.x,
        x0.y, x1.y, x2.y };

    float3 distances = 0.6 - (x[0] * x[0] + x[1] * x[1]);

    float2 vertex0 = lattice_point;
    float2 vertex1 = lattice_point + step1;
    float2 vertex2 = lattice_point + 1;

    float3 vertexx = {vertex0.x, vertex1.x, vertex2.x};
    float3 vertexy = {vertex0.y, vertex1.y, vertex2.y};
    uint3 hash = Hash64To32Parallel( asuint(vertexx + 0), asuint(vertexy + 0) );

    const float2 gradients_2d[8] = { { 1, 2 }, { -1, 2 }, { 1, -2 }, { -1, -2 }, { 2, 1 }, { -2, 1 }, { 2, -1 }, { -2, -1 } };
    float2 gradient0 = gradients_2d[hash.x & 7];
    float2 gradient1 = gradients_2d[hash.y & 7];
    float2 gradient2 = gradients_2d[hash.z & 7];

    float2x3 gradients = {
        gradient0.x, gradient1.x, gradient2.x,
        gradient0.y, gradient1.y, gradient2.y };

    float3 gdotx = gradients[0] * x[0] + gradients[1] * x[1];
    distances = max( distances, 0 );
    float3 distances2 = distances *  distances;
    float3 distances4 = distances2 * distances2;

    float value = dot(gdotx, distances4);

    gradient = mul(gradients, distances4);
    float3 part2 = ( 8 * gdotx * distances2 * distances );
    gradient -= mul(x, part2);

    gradient *= 16;
    return 16 * value;
}



uint Hash96To32Parallel( const uint3 x )
{
    // Murmur-inspired hashing.
    //const uint3 kMul = { 0xA49A9713, 0xcc9e2d51U, 0x4B9B13BB };
    //const uint3 kMul = { 0x76E7C763U, 0xcc9e2d51U, 0x1b873593U };
    const uint3 kMul = { 0x4B9B13BB, 0x76E7C763U, 0x1b873593U };
    uint3 abc = x ^ x.zxy;
        //abc *= kMul;
        //abc = ShiftMix( abc );
        //abc ^= x.yzx;
        abc *= kMul;
    abc = ShiftMix( abc );
    //abc *= kMul;
    return abc.x ^ abc.y ^ abc.z;
}


//
//uint Hash96To32Parallel( const uint3 x )
//{
//    //// Murmur-inspired hashing.
//    //const uint3 kMul = { 0xA49A9713, 0xcc9e2d51U, 0x4B9B13BB };
//    uint2 hash = Hash32To32Parallel( x.xz );
//        hash ^= x.y;
//    //hash *= kMul.x;
//    //hash = ShiftMix( hash );
//    hash = Hash32To32Parallel( hash );
//    return hash.y ^ hash.x;
//}


float3 FindCornerVertex( float3 input )
{
    return floor( input );
}

// returns step 1 and 2, step 0 is all 0 and step 3 is all 1
float2x3 CalculateSteps3( float3 relative_position )
{
    // steps
    float3 stepA = step( relative_position.yzx, relative_position );
    if( all(stepA) )
    {
        stepA = float3( 1, 1, 0 );
    }
    float3 stepB = 1 - stepA.zxy;

    return float2x3( min( stepA, stepB ), max( stepA, stepB ) );
}

float3 GetGradient3( float3 vertex )
{
    static const float3 gradients_3d[4] = {
        { -1, -1, 1}, { 1, -1, -1}, { -1, 1, -1 }, { 1, 1, 1 }
        //{ 1, 1, -1 }, { -1, 1, 1 }, { 1, -1, 1}, { -1, -1, -1 }
    };
    uint hash = Hash96To32Parallel( asuint( vertex + 0 ) );
    float3 gradient = gradients_3d[hash & 3u];
    gradient *= ( hash & 4 ) ? -1 : 1;
    return gradient;
}


float SimplexNoise( float3 position, out float3 gradient )
{
    //float3 skew = Skew( position );
    float3 skew = position + dot( position, 1.f/3 );
    float3 lattice_point = FindCornerVertex( skew );
    //float3 corner = Unskew( lattice_point );
    float3 corner = lattice_point - dot(lattice_point, 1.f/6);

    float3 relative_position = position - corner;
    float2x3 steps = CalculateSteps3( relative_position );

    // vertices
    float4x3 vertices;
    vertices[0] = lattice_point;// +steps[0];
    vertices[1] = lattice_point + steps[0];
    vertices[2] = lattice_point + steps[1];
    vertices[3] = lattice_point + 1;// + steps[3];

    float4x3 x;
    x[0] = relative_position;// -Unskew( steps[0] );
    x[1] = relative_position - ( steps[0] - 1.f/6 );
    //x[2] = relative_position - ( steps[1] - 2 * unskew3d );
    x[2] = relative_position - ( steps[1] - 1.f/3 );
    x[3] = relative_position - 0.5; //(steps[3] - 3 * SkewingFactors<3>::unskew); //== 1 - 0.5

    float3x4 x_t = transpose( x );
    float4 distances;
    {
        const float4 radius = 0.6;
        distances = x_t[0] * x_t[0] + x_t[1] * x_t[1] + x_t[2] * x_t[2];
        distances = radius - distances;
    }

    float4 distances2 = distances * distances;
    float4 distances4 = distances2 * distances2;

    float4x3 gradients = 0;

    [unroll]
    for( uint i = 0u; i < 4; ++i )
    {
        if( distances[i] > 0 )
        {
            gradients[i] = GetGradient3( vertices[i] );
        }
    }

    float3x4 gradients_t = transpose( gradients );
    float4 gdotx = x_t[0] * gradients_t[0] + x_t[1] * gradients_t[1] + x_t[2] * gradients_t[2];

    float value = dot( gdotx, distances4 ) * 32;

    //if( gradient_out != nullptr )
    {
        float4 part2 = 8 * gdotx;
        part2 *= distances2;
        part2 *= distances;
        [unroll]
        for( uint i = 0u; i < 3; ++i )
        {
            gradients_t[i] = ( gradients_t[i] * distances4 ) - ( x_t[i] * part2 );
        }
        gradients = transpose( gradients_t );

        gradient = gradients[0] + gradients[1] + gradients[2] + gradients[3];
        gradient *= 32;
    }

    return value;
}


float SimplexNoise( float position )
{
    float gradient;
    return SimplexNoise(position, gradient);
}

float SimplexNoise( float2 position )
{
    float2 gradient;
    return SimplexNoise(position, gradient);
}

float SimplexNoise( float3 position )
{
    float3 gradient;
    return SimplexNoise(position, gradient);
}