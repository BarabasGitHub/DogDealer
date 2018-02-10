#pragma once
namespace Math
{
    /* Definitions of useful mathematical constants
    * M_E        - e
    * M_LOG2E    - log2(e)
    * M_LOG10E   - log10(e)
    * M_LN2      - ln(2)
    * M_LN10     - ln(10)
    * M_PI       - pi
    * M_PI_2     - pi/2
    * M_PI_4     - pi/4
    * M_1_PI     - 1/pi
    * M_2_PI     - 2/pi
    * M_2_SQRTPI - 2/sqrt(pi)
    * M_SQRT2    - sqrt(2)
    * M_SQRT1_2  - 1/sqrt(2)
    */
    struct RealConstant
    {
        RealConstant(double value): d(value), f(static_cast<float>(value)){};
        const double d;
        const float f;
        operator float() const { return f; }
        operator double() const { return d; }
        RealConstant( RealConstant const & ) = delete;
        RealConstant& operator=( RealConstant const & ) = delete;
    };

    /// e
    const RealConstant c_E  { 2.7182818284590452353602874713527 };
    /// log2(e)
    const RealConstant c_LOG2E  { 1.44269504088896340736  };
    /// log10(e)
    const RealConstant c_LOG10E  { 0.43429448190325182765112891891661 };
    /// ln(2)
    const RealConstant c_LN2  { 0.69314718055994530941723212145818 };
    /// ln(10)
    const RealConstant c_LN10  { 2.3025850929940456840179914546844 };
    /// 2pi
    const RealConstant c_2PI  { 6.283185307179586476925286766559 };
    /// pi
    const RealConstant c_PI  { 3.1415926535897932384626433832795 };
    /// pi/2
    const RealConstant c_PI_div_2  { 1.5707963267948966192313216916398 };
    /// pi/4
    const RealConstant c_PI_div_4  { 0.78539816339744830961566084581988 };
    /// 1 / pi
    const RealConstant c_1_div_PI  { 0.31830988618379067153776752674503 };
    /// 2 / pi
    const RealConstant c_2_div_PI  { 0.63661977236758134307553505349006 };
    /// 2 / sqrt(pi)
    const RealConstant c_2_div_SQRTPI  { 1.1283791670955125738961589031215 };
    /// sqrt(2)
    const RealConstant c_SQRT2  { 1.4142135623730950488016887242097 };
    /// 1/sqrt(2)
    const RealConstant c_SQRT1_2  { 0.70710678118654752440084436210485 };
}