#pragma once

#include <Math\FloatTypes.h>
#include <Math\IntegerTypes.h>
#include <Math\FloatMatrixTypes.h>

// #pragma warning( once : 4820 )

#pragma warning( push )
// disable padding due to __declspec(align()) warnings.
#pragma warning( disable : 4324 )
#define HLSL_ALIGN __declspec( align( 16 ) )

namespace Graphics
{
    namespace HLSL
    {
        struct float2
        {
            HLSL_ALIGN Math::Float2 f;
            float2() = default;
            constexpr float2(Math::Float2 const & f):f(f){}
            constexpr float2(float a, float b):f(a,b){}
        };
        struct float3
        {
            HLSL_ALIGN Math::Float3 f;
            float3() = default;
            constexpr float3(Math::Float3 const & f):f(f){}
            constexpr float3(float a, float b, float c):f(a,b,c){}
        };
        struct float4
        {
            HLSL_ALIGN Math::Float4 f;
            float4() = default;
            constexpr float4(Math::Float4 const & f):f(f){}
            constexpr float4(Math::Float3 f3, float d):f(f3,d){}
            constexpr float4(float a, float b, float c, float d):f(a,b,c,d){}
        };
        struct float3x3
        {
            float3 row[3];

            float3x3() = default;
            constexpr float3x3(Math::Float3x3 const & m) : row{m.row[0], m.row[1], m.row[2]} {}
        };
        struct float3x4
        {
            float4 row[3];

            float3x4( ) = default;
            constexpr float3x4(Math::Float3x3 const & m) : row{float4(m.row[0], 0), float4(m.row[1], 0), float4(m.row[2], 0)} {}
            constexpr float3x4(Math::Float4x4 const & m) : row{m.row[0], m.row[1], m.row[2]} {}
        };
        struct float4x4
        {
            float4 row[4];

            float4x4( ) = default;
            constexpr float4x4(Math::Float4x4 const & m) : row{m.row[0], m.row[1], m.row[2], m.row[3]} {}
        };


        struct arrayfloat
        {
            HLSL_ALIGN float f;

            arrayfloat() = default;
            constexpr arrayfloat( float  v ) : f( v ) {};
        };


        struct uint2
        {
            HLSL_ALIGN Math::Unsigned2 u;
            uint2() = default;
            constexpr uint2(Math::Unsigned2 const & u):u(u){}
            constexpr uint2(uint32_t a, uint32_t b):u(a,b){}
        };
    }
}

#pragma warning( pop )
