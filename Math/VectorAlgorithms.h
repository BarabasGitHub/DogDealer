#pragma once

#include <Utilities\Range.h>
#include <Math\ForwardDeclarations.h>

#include <cstdint>

namespace Math
{
    // x = x ÷ inverse_scale
    void Divide( Range<float *> x, float inverse_scale );
    // y = x ÷ inverse_scale
    void Divide( Range<float const * __restrict> x, float inverse_scale, Range<float * __restrict> y );
    // z = x ÷ y
    void Divide( Range<float const * __restrict> x, Range<float const * __restrict> y, Range<float * __restrict> z);

    // y = scalar + y
    void Add( float const scalar, Range<float * __restrict> y);
    // y = scalar + y
    void Add( uint32_t const scalar, Range<uint32_t * __restrict> y);
    // y = x + y
    void Add( Range<float const * __restrict> x, Range< float * __restrict> y);
    // y = x × x_scale + y
    void Add( Range<float const * __restrict> x, float x_scale, Range< float * __restrict> y );
    // y = x × x_scale + y
    void Add(float const * __restrict x, float x_scale, float * __restrict y, size_t size);
    // y = x + y × y_scale
    void Add( Range<float const * __restrict> x, Range< float * __restrict> y, float y_scale );
    // y = x × x_scale + y × y_scale
    void Add(Range<float const * __restrict> x, float x_scale, Range< float * __restrict> y, float y_scale);
    // z = x + y
    void Add(Range<float const * __restrict> x, Range< float const * __restrict> y, Range< float * __restrict> z);
    // z = x + y
    void Add(float const * __restrict x, float const * __restrict y, float * __restrict z, size_t size);
    // z = x × x_scale + y
    void Add(Range<float const * __restrict> x, float x_scale, Range< float const * __restrict> y, Range< float * __restrict> z);
    // z = x × x_scale + y
    void Add(float const * __restrict x, float x_scale, float const * __restrict y, float * __restrict z, size_t size);

    // y = scalar × y
    void Multiply(float const scalar, Range<float * __restrict> y);
    // y = scalar × y
    void Multiply(float const scalar, float * __restrict y, size_t size);
    // multiples two ranges, Note: the ranges may not overlap!
    // y = x × y
    void Multiply( Range<float const * __restrict > const x, Range<float * __restrict > y );
    // z = x × y
    void Multiply(Range<float const * __restrict > const x, Range<float const * __restrict > y, Range<float * __restrict > z);
    // z = x × scalar
    void Multiply(Range<float const * __restrict > const x, float scalar, Range<float * __restrict > z);
    // z = x × scalar
    void Multiply(float const * __restrict const x, float scalar, float * __restrict z, size_t size);

    float Dot( Range<float const * __restrict > const x, Range<float const * __restrict > y );
    float Dot( float const * __restrict const x, float const * __restrict y, size_t size );

    // for float3
    // y = constant + y
    void Add(Math::Float3 const constant, Range<Math::Float3 * __restrict> y);

    // y = y || x
    void Or(Range<bool const * __restrict> x, Range<bool * __restrict> y);

    // y = max(x, y)
    void Maximum(Range<uint32_t const * __restrict> x, Range<uint32_t * __restrict> y);
}
