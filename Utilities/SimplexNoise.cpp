#include "SimplexNoise.h"

#include <Math\Conversions.h>
#include <Math\FloatOperators.h>
#include <Math\IntegerOperators.h>
#include <Math\MathFunctions.h>

#include <Math\SSE.h>
#include <Math\SSEMathConversions.h>

#include <array>

using namespace Math;
using namespace SSE;

namespace
{

    // N dimensions
    // transform: output = input + factor * sum(input)
    // skew factor: (sqrt(N + 1) - 1) / N
    // unskew factor: (1/sqrt(N + 1) - 1) / N
    template<size_t N>
    struct SkewingFactors
    {
        const static float skew;
        const static float unskew;
    };

    template<size_t N>
    const float SkewingFactors<N>::skew = ( sqrt( N + 1.0f ) - 1 ) / N;
    template<size_t N>
    const float SkewingFactors<N>::unskew = ( 1.0f - ( 1.0f / sqrt( N + 1.0f ) ) ) / N;

    template<>
    const float SkewingFactors<3>::skew = 1.f / 3;
    template<>
    const float SkewingFactors<3>::unskew = 1.f / 6;

    float const radius_2d = 0.6f, radius_3d = radius_2d, radius_4d = 0.6f;

    float Sum( Float2 in ) noexcept { return in.x + in.y; }
    float Sum( Float3 in ) noexcept { return in.x + in.y + in.z; }
    float Sum( Float4 in ) noexcept { return in.x + in.y + in.z + in.w; }


    float Skew( float input ) noexcept
    {
        auto s = input * SkewingFactors<1>::skew;
        return input + s;
    }

    template<typename Type>
    Type Skew( Type input ) noexcept
    {
        auto skewing_factor = SkewingFactors<sizeof( Type ) / sizeof( input[0] )>::skew;
        return input + Sum( input ) * skewing_factor;
    }

    float Unskew( float input ) noexcept
    {
        auto s = input * SkewingFactors<1>::unskew;
        return input - s;
    }

    template<typename Type>
    Type Unskew( Type input ) noexcept
    {
        auto unskewing_factor = SkewingFactors<sizeof( Type ) / sizeof( input[0] )>::unskew;
        return input - Sum( input ) * unskewing_factor;
    }

    template<typename Type>
    Type FindCornerVertex( Type input ) noexcept
    {
        return Floor( input );
    }

    uint32_t FNVHashStartValue() noexcept
    {
        const uint32_t FNV_offset_basis = 2166136261U;
        return FNV_offset_basis;
    }

    uint32_t FNVHash( uint32_t value, uint32_t input ) noexcept
    {
        const uint32_t FNV_prime = 16777619U;
        value ^= ( input & 0x000000FF ) >> 0;
        value *= FNV_prime;
        value ^= ( input & 0x0000FF00 ) >> 8;
        value *= FNV_prime;
        value ^= ( input & 0x00FF0000 ) >> 16;
        value *= FNV_prime;
        value ^= ( input & 0xFF000000 ) >> 24;
        value *= FNV_prime;
        return value;
    }

    template<typename Type>
    uint_fast8_t GetHashValue( Type input ) noexcept
    {
        // we borrow the bitwise hash function from the standard library implementation, because we can.

        static_assert( sizeof( input ) >= sizeof( uint32_t ), "Type is smaller than an uint32_t." );
        static_assert( sizeof( input ) % sizeof( uint32_t ) == 0, "Size of type is not a multiple of uint32_t." );

        uint32_t value = FNVHashStartValue();

        uint32_t* unsigned_input = reinterpret_cast<uint32_t *>( &input );
        const uint32_t loops = sizeof( input ) / sizeof( uint32_t );
        for( uint32_t i = 0; i < loops; i++ )
        {
            value = FNVHash( value, unsigned_input[i] );
        }

        return static_cast<uint_fast8_t>( value );
    }


    uint_fast8_t GetHashValue( Float3 input ) noexcept
    {
        //return GetHashValue( Int3( input.x, input.y, input.z ) );
        return GetHashValue<Float3>( input + 0 );
    }


    Float2 GetGradient( Float2 vertex ) noexcept
    {
        const std::array<Float2, 8> gradients_2d = {Float2(1, 2), Float2(-1, 2), Float2(1, -2), Float2(-1, -2), Float2(2, 1), Float2(-2, 1), Float2(2, -1), Float2(-2, -1)};
        auto hash = GetHashValue( vertex );
        return gradients_2d[hash & 7u];
    }


    Float4 GetGradient( Float4 vertex ) noexcept
    {
        auto hash = GetHashValue( vertex );
        auto gradient = Float4FromInt4( { !!( hash & 1 ), !!( hash & 2 ), !!( hash & 4 ), !!( hash & 8 ) } );
        gradient *= 2;
        gradient -= 1;
        hash >>= 4;
        gradient[hash & 3u] = 0;
        hash >>= 2;
        gradient[hash] *= 2;
        return gradient;
    }

    template<typename FloatType>
    float GetValue( FloatType x, float t, FloatType vertex, FloatType* gradient_out = nullptr ) noexcept
    {
        auto gradient = GetGradient( vertex );

        auto t2 = t * t;
        auto t4 = t2 * t2;
        auto gdotx = Dot( gradient, x );
        auto value = gdotx * t4;
        if( gradient_out != nullptr ) *gradient_out += gradient * t4 - x * ( 8 * gdotx * t2 * t );
        return value;
    }

    std::array<uint_fast8_t, 4> GetOrder( Math::Float4 input ) noexcept
    {
        std::array<uint_fast8_t, 4> output = {};
        auto in_begin = &input.x;
        auto in_end = &input.w + 1;
        for( uint_fast8_t i = 0; i < 4; ++i )
        {
            auto index = static_cast<uint_fast8_t>( std::max_element( in_begin, in_end ) - in_begin );
            input[index] = std::numeric_limits<decltype( input.x )>::lowest();
            output[i] = index;
        }
        return output;
    }
}

// 1D noise
namespace
{
    IntegerVector VECTOR_CALL ShiftMix4( IntegerVector val )
    {
        return ExclusiveOr( val, ShiftRight32BitUnsigned( val, 23 ) );
    }


    IntegerVector VECTOR_CALL Hash32To32ParallelTwoAtTheTime( IntegerVector x )
    {
        // Murmur-inspired hashing.
        x = Swizzle32Bit<0, 0, 1, 1>( x );
        const IntegerVector kMul = Set( 0x76E7C763U, 0xcc9e2d51U, 0x1b873593U, 0u );
        auto ab = Multiply32Bit( x, Swizzle32Bit<0, 1, 0, 1>( kMul ) );
        ab = Add32Bit( ab, Swizzle32Bit<2, 0, 2, 0>( kMul ) );
        ab = ShiftMix4( ab );
        ab = ExclusiveOr( ab, x );
        ab = Multiply32Bit( ab, Swizzle32Bit<1, 2, 1, 2>( kMul ) );
        return ExclusiveOr( Swizzle32Bit<0, 2>( ab ), Swizzle32Bit<1, 3>( ab ) );
    }

    Float32Vector VECTOR_CALL GetGradients1( Float32Vector vertex )
    {
        vertex = Add( vertex, ZeroFloat32Vector() );
        IntegerVector hashes = Hash32To32ParallelTwoAtTheTime( CastToIntegerFromFloat( vertex ) );
        const IntegerVector one = Set( 1, 1, 1, 1 );
        const IntegerVector seven = Set( 7, 7, 7, 7 );
        IntegerVector grad = Add32Bit( one, And( hashes, seven ) );   // Gradient value 1.0, 2.0, ..., 8.0
        const IntegerVector eight = Set( 8, 8, 8, 8 );
        grad = Multiply32Bit( grad, Subtract32Bit( ShiftRight32BitUnsigned( And( hashes, eight ), 2 ), one ) ); // Set a random sign for the gradient
        return Float32FromSignedInteger32( grad ); // Multiply the gradient with the distance
    }
}

float SimplexNoise( float input_float, float * gradient ) noexcept
{
    const auto radius = c_ones;
    // no skew
    auto input = SetAll( input_float );
    auto lattice_point = Floor( input );
    //auto corner = lattice_point; // no unskew

    // point 0 and 1 and reversed
    auto vertex = AddSingle( lattice_point, c_ones );

    //auto relative_x = Subtract(input, corner);

    //float2 x = { relative_x, relative_x - 1 }; // no unskew
    auto x = Subtract( input, vertex );

    auto distance = Subtract( radius, Multiply( x, x ) );

    auto distance2 = Multiply(distance ,distance);
    auto distance4 = Multiply(distance2, distance2);

    auto gradients = GetGradients1( vertex );

    auto gdotx = Multiply(gradients, x); // dot in 1d is not much of a dot
    auto values = Multiply(distance4, gdotx);

    if( gradient )
    {
        const Float32Vector eight = { 8, 8, 8, 8 };
        auto part2 = Multiply( x, Multiply( Multiply( eight, gdotx), Multiply( distance2, distance ) ) );
        gradients = Subtract( Multiply( gradients, distance4 ), part2 );

        *gradient = GetSingle(gradients) + GetSingle( Swizzle<1>(gradients));
    }
    // The maximum value of this noise is 8*(3/4)^4 = 2.53125
    // A factor of 0.395 would scale to fit exactly within [-1,1]

    return 0.395f * (GetSingle( values) + GetSingle( Swizzle<1>(values) ));

}

// 2D noise
namespace
{
    //uint32_t Hash64To32Parallel( const IntegerVector x )
    //{
    //    // Murmur-inspired hashing.
    //    const auto kMul = Set( 0x76E7C763U, 0xcc9e2d51U, 0x1b873593U, 0 );
    //    auto x_swizzle = Swizzle32Bit<1, 0>( x );
    //    auto ab = ExclusiveOr( x, x_swizzle );
    //    ab = Multiply32Bit( ab, kMul );
    //    ab = ShiftMix( ab );
    //    auto ab_shuffle = Swizzle32Bit<1, 0>( ab );
    //    ab = ExclusiveOr( ab, ab_shuffle );
    //    auto const kMul_shuffle = Swizzle32Bit<2, 0>( kMul );
    //    ab = Multiply32Bit( ab, kMul_shuffle );
    //    SSE_ALIGN Unsigned4 abcd;
    //    AlignedStore( ab, &abcd );
    //    return abcd.x ^ abcd.y;
    //}

    IntegerVector VECTOR_CALL Hash64To32Parallel( const IntegerVector x, const IntegerVector y )
    {
        // Murmur-inspired hashing.
        const auto kMulx = SetAll( 0xA49A9713 );
        const auto kMuly = SetAll( 0x4B9B13BB );
        auto a = Add32Bit( Multiply32Bit( ExclusiveOr( x, y ), kMulx), kMuly);
        a = ShiftMix4( a );
        auto b = ExclusiveOr( y, a );
        b = Multiply32Bit( b, kMulx );
        b = ShiftMix4( b );
        b = Multiply32Bit(b, kMulx);
        return b;
    }

    inline Float32Vector VECTOR_CALL Skew2( Float32Vector in )
    {
        auto const skewing_factor = SetAll( SkewingFactors<2>::skew );
        auto sum = Dot2( in, skewing_factor );
        return Add( in, sum );
    }

    inline Float32Vector VECTOR_CALL Unskew2( Float32Vector in )
    {
        auto const skewing_factor = SetAll( SkewingFactors<2>::unskew );
        auto sum = Dot2( in, skewing_factor );
        return Subtract( in, sum );
    }

    inline Float32Vector VECTOR_CALL CalculateStep2( Float32Vector relative_position )
    {
        auto step = LessThanOrEqual( Swizzle<0,1,1,1>(relative_position), Swizzle<1, 1, 1, 1>( relative_position ) );
        if( AllSignBitsSet( step ) )
        {
            Float32Vector const c_0111 = { 0, 1, 1, 1 };
            return c_0111;
            //step = And( step, c_0111 ); // pretend x > y
        }
        else
        {
            return{ { 1, 0, 1, 1 } };
            //step = And( step, one );
        }
        //return step;
    }


    inline void VECTOR_CALL Calculate2DGradients(IntegerVector hash, std::array<Float32Vector, 2>& gradients)
    {
        // should be like this
        // auto gradientx = (1 - 2 * bool(hash & 1)) * (1 + bool(hash & 4));
        // auto gradienty = (1 - 2 * bool(hash & 2)) * (2 - bool(hash & 4));
        auto const one = SetAll(1);
        auto const two = Add32Bit(one, one);
        auto const four = Add32Bit(two, two);
        auto even = And(hash, one); // bool(hash & 1)
        auto even_quarter = And(hash, two); // 2 * bool(hash & 2)
        auto even_half = And(ShiftRight32BitUnsigned(hash, 2), one); // bool(hash & 4)

        auto gradientx = Multiply32Bit(Subtract32Bit(one, Add32Bit(even, even)), Add32Bit(one, even_half));
        auto gradienty = Multiply32Bit(Subtract32Bit(one, even_quarter), Subtract32Bit(two, even_half));
        gradients[0] = Float32FromSignedInteger32(gradientx);
        gradients[1] = Float32FromSignedInteger32(gradienty);
        // old
        // hash = And(hash, SetAll(7));
        // const std::array<Float2 const, 8> gradients_2d = { { { 1, 2 }, { -1, 2 }, { 1, -2 }, { -1, -2 }, { 2, 1 }, { -2, 1 }, { 2, -1 }, { -2, -1 } } };
        // Unsigned4 hashxyz = Unsigned4FromSSE( hash );
        // auto gradient0 = gradients_2d[hashxyz.x];
        // auto gradient1 = gradients_2d[hashxyz.y];
        // auto gradient2 = gradients_2d[hashxyz.z];
        // gradients = { {
        //     {gradient0.x, gradient1.x, gradient2.x },
        //     {gradient0.y, gradient1.y, gradient2.y } } };
    }
}

float SimplexNoise( Math::Float2 position_in, Float2* gradient_out ) noexcept
{
    auto position = SSEFromFloat2( position_in );
    auto skew = Skew2( position );
    auto lattice_point = FindCornerVertex( skew );
    auto corner = Unskew2( lattice_point );

    auto relative_position = Subtract(position, corner);

    //auto x0 = relative_position;
    auto step = CalculateStep2( relative_position );
    const Float32Vector unskewing = { SkewingFactors<2>::unskew, SkewingFactors<2>::unskew, 2 * SkewingFactors<2>::unskew, 2 * SkewingFactors<2>::unskew };
    auto x12 = Subtract(Swizzle<0,1,0,1>(relative_position), Subtract(step, unskewing));

    std::array<Float32Vector, 2> x = { { Shuffle<0, 0, 0, 2>( relative_position, x12 ), Shuffle<1, 1, 1, 3>( relative_position, x12 ) } };
    x[0] = Swizzle<1, 2, 3>( x[0] );
    x[1] = Swizzle<1, 2, 3>( x[1] );

    Float32Vector const radius = SetAll( 0.6f );
    auto distances = Subtract( radius, Add( Multiply( x[0], x[0] ), Multiply( x[1], x[1] ) ));
    auto const zero = ZeroFloat32Vector();
    distances = Max( distances, zero );

    auto vertex12 = Add(Swizzle<0,1,0,1>(lattice_point), step);

    // in the wrong order, 120, instead of 012
    auto vertexx = Shuffle<0, 2, 0>( vertex12, lattice_point );
    auto vertexy = Shuffle<1, 3, 1>( vertex12, lattice_point );
    auto hash = Swizzle32Bit<2,0,1>(Hash64To32Parallel( CastToIntegerFromFloat(Add(vertexx, zero)), CastToIntegerFromFloat(Add(vertexy, zero)) ));
    std::array<Float32Vector, 2> gradients;
    Calculate2DGradients(hash, gradients);

    auto gdotx = Add( Multiply(gradients[0], x[0]), Multiply(gradients[1], x[1]));
    auto distances2 = Multiply(distances, distances);
    auto distances4 = Multiply(distances2, distances2);

    auto const sixteen = SetAll( 16.f );
    auto value = Dot3(gdotx, distances4);
    value = MultiplySingle( value, sixteen );

    if( gradient_out )
    {
        auto gradient = InterleaveLow( Dot3( gradients[0], distances4 ), Dot3( gradients[1], distances4 ) );
        auto part2 = Multiply( Multiply(SetAll(8.f), gdotx), Multiply(distances2, distances ));
        gradient = Subtract( gradient, InterleaveLow( Dot3( x[0], part2 ), Dot3( x[1], part2 ) ) );

        gradient = Multiply(gradient, sixteen);
        *gradient_out = Float2FromSSE(gradient);
    }
    auto result = GetSingle(value);
    return result;
}


//// returns step 1 and 2, step 0 is all 0 and step 3 is all 1
//inline std::array<Float3, 2> CalculateSteps( Float3 const & relative_position ) noexcept
//{
//    // steps
//    auto stepA = Step( Swizzle<1, 2, 0>( relative_position ), relative_position );
//    if( stepA == 1 ) stepA = { 1, 1, 0 }; // pretend x > y > z
//    auto stepB = 1 - Swizzle<2, 0, 1>( stepA );
//
//    return{ {
//        { Min( stepA, stepB ) },
//        { Max( stepA, stepB ) },
//    } };
//}

//inline uint32_t ShiftMix( uint32_t val )
//{
//    return val ^ ( val >> 23 );
//}
//
//uint32_t Hash96To32Parallel( const void * x_in )
//{
//    // Murmur-inspired hashing.
//    auto x = reinterpret_cast<uint32_t const *>( x_in );
//    const auto kMul1 = 0x76E7C763U; //1994901347
//    const auto kMul2 = 0xcc9e2d51U;
//    const auto kMul3 = 0x1b873593U;
//    auto a = ( x[0] ^ x[1] ) * kMul1;
//    auto b = ( x[1] ^ x[2] ) * kMul2;
//    auto c = ( x[2] ^ x[0] ) * kMul3;
//    a = ShiftMix( a );
//    b = ShiftMix( b );
//    c = ShiftMix( c );
//    a ^= b;
//    b ^= c;
//    c ^= a;
//    a *= kMul3;
//    b *= kMul1;
//    c *= kMul2;
//    return a ^ b ^ c;
//}

// 3D noise
namespace
{

    //uint32_t Hash96To32Parallel( const IntegerVector x )
    //{
    //    // Murmur-inspired hashing.
    //    const auto kMul = Set( 0x76E7C763U, 0xcc9e2d51U, 0x1b873593U, 0 );
    //    auto x_swizzle = Swizzle32Bit<1, 2, 0, 3>( x );
    //    auto abc = ExclusiveOr( x, x_swizzle );
    //    abc = Multiply32Bit( abc, kMul );
    //    abc = ShiftMix( abc );
    //    auto abc_shuffle = Swizzle32Bit<1, 2, 0, 3>( abc );
    //    abc = ExclusiveOr( abc, abc_shuffle );
    //    auto const kMul_shuffle = Swizzle32Bit<2, 0, 1, 3>( kMul );
    //    abc = Multiply32Bit( abc, kMul_shuffle );
    //    SSE_ALIGN Unsigned4 abcd;
    //    AlignedStore( abc, &abcd );
    //    return abcd.x ^ abcd.y ^ abcd.z;
    //}


    uint32_t Hash96To32Parallel( const IntegerVector x )
    {
        const auto kMul = Set( 0x4B9B13BBu, 0x76E7C763U, 0x1b873593U, 0u );
        auto abc = ExclusiveOr( x, Swizzle32Bit<2,0,1>( x ) );
        //abc = Multiply32Bit(abc, kMul);
        //abc = ShiftMix( abc );
        //abc = ExclusiveOr( abc, Swizzle32Bit<1, 2, 0>( x ) );
        abc = Multiply32Bit( abc, kMul);
        abc = ShiftMix4( abc );
        //abc = Multiply32Bit( abc, kMul );
        auto abc_ = Unsigned4FromSSE( abc );
        return abc_.x ^ abc_.y ^ abc_.z;
    }


    inline Float32Vector VECTOR_CALL Skew3( Float32Vector in )
    {
        auto const skewing_factor = SetAll( SkewingFactors<3>::skew );
        auto sum = Dot3( in, skewing_factor );
        return Add( in, sum );
    }


    inline Float32Vector VECTOR_CALL Unskew3( Float32Vector in )
    {
        auto const skewing_factor = SetAll( SkewingFactors<3>::unskew );
        auto sum = Dot3( in, skewing_factor );
        return Subtract( in, sum );
    }


    // returns step 1 and 2, step 0 is all 0 and step 3 is all 1
    inline std::pair<Float32Vector, Float32Vector> VECTOR_CALL CalculateSteps3( Float32Vector relative_position ) noexcept
    {
        // steps
        auto swizzled = Swizzle<1, 2, 0, 3>( relative_position );
        auto stepA = LessThanOrEqual( swizzled, relative_position );
        if( AllSignBitsSet( stepA ) )
        {
            stepA = Shuffle<0, 1, 0, 1>(stepA, ZeroFloat32Vector()); // pretend x > y > z
        }

        stepA = And( stepA, c_ones );
        swizzled = Swizzle<2, 0, 1, 3>( stepA );
        auto stepB = Subtract( c_ones, swizzled );

        return std::make_pair( Min( stepA, stepB ), Max( stepA, stepB ) );
    }


    inline Float32Vector VECTOR_CALL GetGradient3( Float32Vector vertex ) noexcept
    {
        static const std::array<Float32Vector, 8> gradients_3d = { {
            { -1, -1, 1, 0 }, { 1, -1, -1, 0 }, { -1, 1, -1, 0 }, { 1, 1, 1, 0 },
            { 1, 1, -1, 0 }, { -1, 1, 1, 0 }, { 1, -1, 1, 0 }, { -1, -1, -1, 0 }
        } };
        auto hash = Hash96To32Parallel( CastToIntegerFromFloat( vertex ) );
        //gradient += float3( ( hash & 1u ? 1 : -1 ), ( hash & 2u ? 1 : -1 ), ( hash & 4u ? 1 : -1 ) );
        auto gradient = gradients_3d[hash & 7u];
        return gradient;
    }


    Float32Vector VECTOR_CALL CalculateDistances(Float32Vector x0, Float32Vector x1, Float32Vector x2 ) noexcept
    {
        Float32Vector const radius = SetAll( radius_3d );
        auto distances0 = Multiply( x0, x0 );
        auto distances1 = Multiply( x1, x1 );
        auto distances2 = Multiply( x2, x2 );
        auto distances01 = Add( distances0, distances1 );
        auto distances = Subtract( radius, distances2 );
        distances = Subtract( distances, distances01 );
        return distances;
    }
}


float SimplexNoise( Math::Float3 position_in, Float3* gradient_out ) noexcept
{
    auto position = SSEFromFloat3( position_in );
    auto skew = Skew3( position );
    auto lattice_point = FindCornerVertex( skew );
    auto corner = Unskew3( lattice_point );

    auto relative_position = Subtract( position, corner );
    auto steps = CalculateSteps3( relative_position );

    // vertices
    std::array<Float32Vector, 4> vertices;
    vertices[0] = lattice_point;// +steps[0];
    vertices[1] = Add( lattice_point, std::get<0>( steps ) );
    vertices[2] = Add( lattice_point, std::get<1>( steps ) );
    vertices[3] = Add( lattice_point, c_ones );// + steps[3];

    std::array<Float32Vector, 3> x;
    x[0] = relative_position;// -Unskew( steps[0] );
    x[1] = Subtract( relative_position, Subtract( std::get<0>( steps ), SetAll( SkewingFactors<3>::unskew ) ) );
    //x[2] = Subtract( relative_position, Subtract( std::get<1>( steps ), SetAll( 2 * SkewingFactors<3>::unskew ) ) );
    // 2 * unskew = 2 * 1/6 = 1 / 3 = skew
    x[2] = Subtract( relative_position, Subtract( std::get<1>( steps ), SetAll( SkewingFactors<3>::skew ) ) );
    auto x3 = Subtract( relative_position, SetAll( 0.5f ) ); //(steps[3] - 3 * SkewingFactors<3>::unskew); //== 1 - 0.5
    Transpose( x[0], x[1], x[2], x3 );

    Float32Vector distances = CalculateDistances( x[0], x[1], x[2] );

    std::array<Float32Vector, 4> gradients{ { ZeroFloat32Vector(), ZeroFloat32Vector(), ZeroFloat32Vector(), ZeroFloat32Vector() } };
    auto distance_to_compare = distances;
    for( auto i = 0u; i < 4; ++i )
    {
        //gradients[i] = ZeroFloat32Vector();
        if( GetSingle( distance_to_compare ) > 0 )
        {
            gradients[i] = GetGradient3( vertices[i] );
        }
        distance_to_compare = Swizzle<1, 2, 3>( distance_to_compare );
    }
    Transpose( gradients[0], gradients[1], gradients[2], gradients[3] );

    Float32Vector gdotx;
    {
        gdotx = Multiply( gradients[0], x[0] );
        gdotx = Add( gdotx, Multiply( gradients[1], x[1] ) );
        gdotx = Add( gdotx, Multiply( gradients[2], x[2] ) );
    }

    auto distances2 = Multiply( distances, distances );
    auto distances4 = Multiply( distances2, distances2 );
    auto const thirty_two = SetAll( 32.f );
    auto value = MultiplySingle( Dot4( gdotx, distances4 ), thirty_two );

    if( gradient_out != nullptr )
    {
        auto part2 = Multiply( SetAll( 8.f ), gdotx );
        auto distances3 = Multiply( distances, distances2 );
        part2 = Multiply( part2, distances3 );
        for( auto i = 0u; i < 3; ++i )
        {
            gradients[i] = Subtract( Multiply( gradients[i], distances4 ), Multiply( x[i], part2 ) );
        }
        gradients[3] = ZeroFloat32Vector();
        Transpose( gradients[0], gradients[1], gradients[2], gradients[3] );

        auto gradient = Add( gradients[0], gradients[1] );
        auto gradient2 = Add( gradients[2], gradients[3] );
        gradient = Add( gradient, gradient2 );
        gradient = Multiply( gradient, thirty_two );
        *gradient_out = Float3FromSSE( gradient );
    }

    return GetSingle( value );
}


float SimplexNoise( Math::Float4 position, Float4* gradient ) noexcept
{
    auto skew = Skew( position );
    auto lattice_point = FindCornerVertex( skew );
    auto corner = Unskew( lattice_point );

    auto relative_position = position - corner;

    auto x = relative_position;
    auto distance = radius_4d - Dot( x, x );
    if( gradient ) *gradient = 0;
    auto value = ( distance > 0 ) ? GetValue( x, distance, lattice_point, gradient ) : 0.f;

    Math::Float4 step = 0;
    for( auto const index : GetOrder( relative_position ) )
    {
        step[index] = 1;
        x = relative_position - Unskew( step );
        distance = radius_4d - Dot( x, x );
        if( distance > 0 )
        {
            auto vertex = lattice_point + step;
            value += GetValue( x, distance, vertex, gradient );
        }
    }

    if( gradient ) *gradient *= 24;
    return 24 * value;
}
