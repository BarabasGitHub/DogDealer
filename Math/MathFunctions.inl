#pragma once
#include "MathFunctions.h"
#include "FloatMatrixOperators.h"
#include "IntegerOperators.h"

#include <algorithm>
#include <numeric>
#include <cmath>
#include <functional>

// template function implementations
namespace Math
{

    template<> float Clamp<float>( const float min, const float max, const float input );

    template<typename Type>
    inline Type Clamp( const Type min_value, const Type max_value, const Type input )
    {
        return Max( min_value, Min( max_value, input ) );
    }

    template<typename Type>
    auto Sum(Type const & input)
    {
        return std::accumulate(std::next(begin(input)), end(input), *begin(input));
    }


    template<typename Type>
    auto Product(Type const & input)
    {
        return std::accumulate(std::next(begin(input)), end(input), *begin(input), std::multiplies<>());
    }


    template<typename Type>
    inline float Norm( const Type first )
    {
        auto dot = SquaredNorm(first);
        return Sqrt(dot);
    }


    template<typename Type>
    inline float SquaredNorm( const Type first )
    {
        return Dot( first, first );
    }


    template<typename Type>
    inline float Distance( const Type a, const Type b )
    {
        return Norm(a - b);
    }


    template<typename Type>
    inline float SquaredDistance( const Type a, const Type b )
    {
        return SquaredNorm(a - b);
    }


    inline float SquaredDistance( const float a, const float b )
    {
        return (a - b)*(a - b);
    }


    template<typename Type>
    inline Type Normalize( const Type first )
    {
        auto norm = Norm( first );
        assert(norm != 0);
        return first / norm;
    }


    template<typename Type>
    inline Type ConditionalNormalize( const Type first )
    {
        auto norm = Norm( first );
        return norm == 0 ? 0 : first / norm;
    }

    // special implementation for quaternions
    Quaternion Normalize( Quaternion const & first );

    template<typename Type>
    inline Type Lerp( Type first, Type second, float blend_factor )
    {
        return first + ( second - first ) * blend_factor;
    }

    // make a separate implementation for matrices
    Float4x4 Lerp( Float4x4 first, Float4x4 second, float blend_factor );


    template<typename Type>
    inline Type Nlerp( Type first, Type second, float blend_factor )
    {
        return Normalize( Lerp(first, second, blend_factor) );
    }

    // specialize for quaternions
    Quaternion Nlerp( Quaternion const & first, Quaternion const & second, float blend_factor );

    template<typename Type>
    inline Type cot( Type input )
    {
        return std::tan( static_cast<Type>(c_PI_div_2) - input );
    }


    template<typename Type> inline Type Sign( Type const & input )
    {
        return CopySign( Type(1), input );
    }



    namespace Details
    {

        template<typename Type, bool = std::is_arithmetic<std::decay_t<Type>>::value >
        struct Helper;

        template<typename Type>
        struct Helper<Type, true>
        {
            typedef std::decay_t<Type> ValueType;

            static bool Equal( ValueType const first, ValueType const second, float const tolerance )
            {
                return Math::AbsoluteDifference( first, second ) <= tolerance;
            }


            static bool Equal( ValueType const first, ValueType const second)
            {
                return first == second;
            }


            static ValueType Min(ValueType const first, ValueType const second)
            {
                return std::min(first, second);
            }


            static ValueType Max(ValueType const first, ValueType const second)
            {
                return std::max(first, second);
            }


            static ValueType Abs( ValueType const first )
            {
                return std::abs( first );
            }


            static ValueType Sqrt( ValueType const input )
            {
                return std::sqrt( input );
            }


            static ValueType Pow( ValueType input, ValueType power)
            {
                return std::pow(input, power);
            }


            static ValueType Floor( ValueType const input )
            {
                return std::floor( input );
            }


            static ValueType Ceil( ValueType const input )
            {
                return std::ceil( input );
            }


            static ValueType Round( ValueType const input )
            {
                return std::round( input );
            }


            static ValueType CopySign( ValueType const first, ValueType const second )
            {
                return std::copysign( first, second );
            }

            static ValueType Modulo( ValueType const number, ValueType const denominator )
            {
                return Modulo<std::is_floating_point<ValueType>::value, std::is_signed<ValueType>::value>( number, denominator );
            }

            template< bool floating, bool signed_integer  >
            static ValueType Modulo( ValueType const number, ValueType const denominator );

            template<>
            static ValueType Modulo<true, false>( ValueType const number, ValueType const denominator )
            {
                return number - denominator * Floor( number / denominator );
            }

            template<>
            static ValueType Modulo<true, true>( ValueType const number, ValueType const denominator )
            {
                return Modulo<true, false>( number, denominator );
            }

            template<>
            static ValueType Modulo< false, true >( ValueType const number, ValueType const denominator )
            {
                auto result = number % denominator;
                return result + ( number < 0 ? denominator : 0 );
            }

            template<>
            static ValueType Modulo< false, false >( ValueType const number, ValueType const denominator )
            {
                return number % denominator;
            }

        };

        template<typename Type>
        struct Helper<Type, false>
        {
        private:
            typedef std::decay_t<Type> DecayedType;

            template<typename CompareFuncionType>
            static bool CompareEqual( Type const & first, Type const & second, CompareFuncionType function )
            {
                using std::begin;
                using std::end;
                auto element_count = end( first ) - begin( first );
                if( end( second ) - begin( second ) != element_count ) return false;
                if(element_count == 0) return true;
                return std::equal( begin( first ), end( first ), begin( second ), function );
            }


            template<typename FunctionType>
            static DecayedType ForAll( Type const & input, FunctionType function )
            {
                auto out = DecayedType{};
                using std::begin;
                using std::end;
                std::transform( begin( input ), end( input ), begin( out ), function );
                return out;
            }


            template<typename FunctionType>
            static DecayedType ForAll( Type const & first, Type const & second, FunctionType const & function )
            {
                auto out = DecayedType{};
                using std::begin;
                using std::end;
                assert( end( second ) - begin( second ) == end( first ) - begin( first ) );
                std::transform( begin( first ), end( first ), begin( second ), begin( out ), function );
                return out;
            }



        public:
            static bool Equal( Type const & first, Type const & second, float const tolerance )
            {
                typedef decltype( first[0] ) SubType;
                return CompareEqual( first, second, [tolerance]( SubType const & first, SubType const & second )
                {
                    return Math::Equal( first, second, tolerance );
                } );
            }


            template<typename ElementComparatorType>
            static bool Equal( Type const & first, Type const & second, ElementComparatorType const & element_comparator )
            {
                return CompareEqual( first, second, element_comparator );
            }


            static bool Equal( Type const & first, Type const & second )
            {
                typedef decltype( first[0] ) SubType;
                return CompareEqual( first, second, []( SubType const & first, SubType const & second )
                {
                    return Math::Equal( first, second );
                } );
            }


            static DecayedType Min(Type const & first, Type const & second)
            {
                return ForAll( first, second, Helper<decltype( first[0] )>::Min );
            }


            static DecayedType Max(Type const & first, Type const & second)
            {
                return ForAll( first, second, Helper<decltype( first[0] )>::Max );
            }


            static DecayedType Abs( Type const & input )
            {
                return ForAll( input, Helper<decltype( input[0] )>::Abs );
            }


            static DecayedType Sqrt( Type const & input )
            {
                return ForAll( input, Helper<decltype( input[0] )>::Sqrt );
            }


            static DecayedType Pow( Type const & input, Type const & power)
            {
                return ForAll( input, power, Helper<decltype(input[0])>::Pow );
            }


            static DecayedType Floor( Type const & input )
            {
                return ForAll( input, Helper<decltype( input[0] )>::Floor );
            }


            static DecayedType Ceil( Type const & input )
            {
                return ForAll( input, Helper<decltype( input[0] )>::Ceil );
            }


            static DecayedType Round( Type const & input )
            {
                return ForAll( input, Helper<decltype( input[0] )>::Round );
            }


            static DecayedType CopySign( Type const & first, Type const & second )
            {
                return ForAll( first, second, Helper<decltype( first[0] )>::CopySign );
            }


            static DecayedType Modulo( Type const & first, Type const & second )
            {
                return ForAll( first, second, Helper<decltype( first[0] )>::Modulo );
            }
        };
    }

    template<typename Type>
    bool Equal(Type const & first, Type const & second )
    {
        return Details::Helper<Type>::Equal(first, second);
    }


    template<typename Type, typename ElementComparatorType>
    bool Equal( Type const & first, Type const & second, ElementComparatorType const & element_comparator)
    {
        return Details::Helper<Type>::Equal(first, second, element_comparator);
    }


    template<typename Type>
    bool Equal(Type const & first, Type const & second, float const tolerance )
    {
        return Details::Helper<Type>::Equal(first, second, tolerance);
    }


    template<> bool Equal<Quaternion>(Quaternion const & first, Quaternion const & second, float const tolerance);


    template<typename Type>
    Type Max(Type const & first, Type const & second)
    {
        return Details::Helper<Type>::Max(first, second);
    }


    template<> float Max<float>(float const& first, float const& second);
    template<> Float3 Max<Float3>(Float3 const & first, Float3 const & second);
    template<> Float4 Max<Float4>(Float4 const & first, Float4 const & second);


    template<typename Type>
    Type Min(Type const & first, Type const & second)
    {
        return Details::Helper<Type>::Min(first, second);
    }


    float Min(float const first, float const second);
    Float3 Min(Float3 const & first, Float3 const & second);
    Float4 Min(Float4 const & first, Float4 const & second);


    template<typename Type>
    Type Abs( Type const & input )
    {
        return Details::Helper<Type>::Abs(input);
    }


    float Abs( float const first );
    Float3 Abs( Float3 const & m );
    Float4 Abs( Float4 const & m );
    Float3x3 Abs( Float3x3 const & m );

    template<typename Type>
    Type AbsoluteDifference(Type a, Type b)
    {
        static_assert(std::is_signed<Type>::value, "Using AbsoluteDifference with unsigned types is not supported.");
        return Abs(a - b);
    }

    template<typename Type>
    Type Sqrt( Type const & input )
    {
        return Details::Helper<Type>::Sqrt( input );
    }


    template<typename Type> Type Pow( Type const & input, Type const & power)
    {
        return Details::Helper<Type>::Pow( input , power );
    }


    template<typename Type>
    Type Floor( Type const & input )
    {
        return Details::Helper<Type>::Floor( input );
    }


    template<typename Type>
    Type Ceil( Type const & input )
    {
        return Details::Helper<Type>::Ceil( input );
    }


    template<typename Type>
    Type Round( Type const & input )
    {
        return Details::Helper<Type>::Round( input );
    }


    template<typename Type>
    Type CopySign( Type const & magnitude, Type const & sign )
    {
        return Details::Helper<Type>::CopySign( magnitude, sign );
    }

    Float3 CopySign( Float3 const & magnitude, Float3 const & sign );
    Float4 CopySign( Float4 const & magnitude, Float4 const & sign );
    Quaternion CopySign( Quaternion const & magnitude, Quaternion const & sign );


    template<typename Type> Type Modulo( Type number, Type const & denominator )
    {
        return Details::Helper<Type>::Modulo( number, denominator );
    }
}
