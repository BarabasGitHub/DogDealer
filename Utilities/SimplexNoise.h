#pragma once
#include <Math\ForwardDeclarations.h>
#include "Range.h"



// parameters for the noise
// offset can be used to change the starting point
// frequency determines the frequency of change in the noise
// amplitude determines the maximum and minimum value the noise can return, ranges from -amplitude to +amplitude
template<typename FloatType>
struct NoiseParameters
{
    FloatType offset;
    FloatType frequency;
    float amplitude;
};

// 1d Simplex Noise
float SimplexNoise( float x, float* gradient = nullptr ) noexcept;

// 2d Simplex Noise
float SimplexNoise( Math::Float2 position, Math::Float2* gradient = nullptr ) noexcept;

// 3d Simplex Noise
float SimplexNoise( Math::Float3 position, Math::Float3* gradient = nullptr ) noexcept;

// 4d Simplex Noise
float SimplexNoise( Math::Float4 position, Math::Float4* gradient = nullptr ) noexcept;

// single level of simplex noise, without gradient
template<typename FloatType>
float SimplexNoise( NoiseParameters<FloatType> const & parameters, FloatType position) noexcept;

// single level of simplex noise, with gradient
template<typename FloatType>
float SimplexNoise( NoiseParameters<FloatType> const & parameters, FloatType position, FloatType& gradient ) noexcept;

// multiple levels of simplex noise summed together, without gradient
template<typename FloatType>
float SimplexNoise( Range<NoiseParameters<FloatType> const *> levels, FloatType input ) noexcept;

// multiple levels of simplex noise summed together, with gradient
template<typename FloatType>
float SimplexNoise( Range<NoiseParameters<FloatType> const *> levels, FloatType input, FloatType& gradient ) noexcept;


// implementations
template<typename FloatType>
float SimplexNoise( NoiseParameters<FloatType> const & parameters, FloatType position ) noexcept
{
    position += parameters.offset;
    position *= parameters.frequency;
    auto value = SimplexNoise( position, nullptr );
    value *= parameters.amplitude;
    return value;
}


template<typename FloatType>
float SimplexNoise( NoiseParameters<FloatType> const & parameters, FloatType position, FloatType& gradient ) noexcept
{
    position += parameters.offset;
    position *= parameters.frequency;
    auto value = SimplexNoise( position, &gradient );
    gradient *= parameters.frequency * parameters.amplitude;
    return value * parameters.amplitude;
}


template<typename FloatType>
inline float SimplexNoise( Range<NoiseParameters<FloatType> const *> levels, FloatType input ) noexcept
{
    float value = 0;
    for( auto const & p : levels )
    {
        value += SimplexNoise( p, input );
    }
}


template<typename FloatType>
inline float SimplexNoise( Range<NoiseParameters<FloatType> const *> levels, FloatType input, FloatType& gradient ) noexcept
{
    float value = 0;
    gradient = 0;
    for( auto const & p : levels )
    {
        FloatType new_gradient;
        value += SimplexNoise( p, input, new_gradient );
        gradient += new_gradient;
    }
    return value;
}