#pragma once

#include "SSETypes.h"

#include <Utilities\CompilerHints.h>

#include <type_traits>

namespace Math
{
    namespace SSE
    {

        Float32Vector Load(float const * input);
        Float32Vector LoadFloat32Vector( float const * input );
        Float32Vector Load2( float const * input, Float32Vector upper_values = { { 0, 0, 0, 0 } } );
        Float32Vector Load2Upper( float const * input, Float32Vector lower_values = { { 0, 0, 0, 0 } } );
        Float32Vector LoadSingle(float const * input);
        Float32Vector LoadAll( float * input );

        template<typename IntegerType, typename = std::enable_if_t<std::is_integral<IntegerType>::value>>
        IntegerVector Load( IntegerType const * input );
        template<typename IntegerType, typename = std::enable_if_t<std::is_integral<IntegerType>::value>>
        IntegerVector Load2(IntegerType const * input);
        template<typename IntegerType, typename = std::enable_if_t<std::is_integral<IntegerType>::value>>
        IntegerVector Load2(IntegerType const * input, IntegerVector upper_values);
        template<typename IntegerType, typename = std::enable_if_t<std::is_integral<IntegerType>::value>>
        IntegerVector Load2Upper(IntegerType const * input);
        template<typename IntegerType, typename = std::enable_if_t<std::is_integral<IntegerType>::value>>
        IntegerVector Load2Upper( IntegerType const * input, IntegerVector lower_values );
        template<typename IntegerType, typename = std::enable_if_t<std::is_integral<IntegerType>::value>>
        IntegerVector LoadAll( IntegerType * input );

        void Store( Float32Vector val, float * output );
        void Store2( Float32Vector val, float* output );
        void Store2Upper( Float32Vector val, float* output );
        void StoreSingle(Float32Vector val, float * output);

        template<typename IntegerType, typename = std::enable_if_t<std::is_integral<IntegerType>::value> >
        void Store( IntegerVector val, IntegerType * output );
        template<typename IntegerType, typename = std::enable_if_t<std::is_integral<IntegerType>::value> >
        void Store64( IntegerVector val, IntegerType * output );

        void Store( IntegerVector val, int64_t & output );
        void Store( IntegerVector val, uint64_t & output );
    }
}


// implementations
namespace Math
{
    namespace SSE
    {

        inline Float32Vector Load(float const * input)
        {
            return LoadFloat32Vector(input);
        }


        inline Float32Vector LoadFloat32Vector( float const * input )
        {
            return _mm_loadu_ps( input );
        }


        inline Float32Vector Load2( float const * input, Float32Vector upper_values )
        {
            return _mm_loadl_pi( upper_values, reinterpret_cast<__m64 const *>(input) );
        }


        inline Float32Vector Load2Upper( float const * input, Float32Vector lower_values )
        {
            return _mm_loadh_pi( lower_values, reinterpret_cast<__m64 const *>( input ) );
        }


        inline Float32Vector LoadSingle(float const * input)
        {
            return _mm_load_ss(input);
        }


        inline void Store( Float32Vector val, float * output )
        {
            _mm_storeu_ps( output, val );
        }


        inline void Store2(Float32Vector val, float* output)
        {
            _mm_storel_pi( reinterpret_cast<__m64*>(output), val );
        }


        inline void Store2Upper(Float32Vector val, float* output)
        {
            _mm_storeh_pi( reinterpret_cast<__m64*>(output), val );
        }


        inline void StoreSingle(Float32Vector val, float * output)
        {
            _mm_store_ss(output, val);
        }

        inline Float32Vector LoadAll( float * in )
        {
            return _mm_load_ps1( in );
        }


        template<typename IntegerType, typename>
        IntegerVector Load( IntegerType const * input )
        {
            return _mm_loadu_si128(reinterpret_cast<IntegerVector const *>(input));
        }


        template<typename IntegerType, typename>
        IntegerVector Load2( IntegerType const * input)
        {
            return _mm_loadl_epi64(reinterpret_cast<IntegerVector const *>(input));
        }


        template<typename IntegerType, typename>
        IntegerVector Load2( IntegerType const * input, IntegerVector upper_values)
        {
            // trick to put the values in the upper half (hopefully it doesn't cause problems)
            auto lower_values = Load(reinterpret_cast<uint64_t const*>(input) - 1);
            return _mm_unpackhi_epi64(lower_values, upper_values);
        }


        template<typename IntegerType, typename>
        IntegerVector Load2Upper(IntegerType const * input)
        {
            auto upper_values = Load2(input);
            return _mm_unpacklo_epi64(_mm_setzero_si128(), upper_values);
        }


        template<typename IntegerType, typename>
        IntegerVector Load2Upper( IntegerType const * input, IntegerVector lower_values)
        {
            auto upper_values = Load(input);
            return _mm_unpacklo_epi64(lower_values, upper_values);
        }


        template<typename IntegerType, typename>
        IntegerVector LoadAll( IntegerType * in )
        {
            switch(sizeof(IntegerType))
            {
                case sizeof(char):
                    return _mm_set1_epi8(reinterpret_cast<char&>(*in));
                case sizeof(short):
                    return _mm_set1_epi16(reinterpret_cast<short&>(*in));
                case sizeof(int):
                    return _mm_set1_epi32(reinterpret_cast<int&>(*in));
                case sizeof(__m64):
                    return _mm_set1_epi64x(reinterpret_cast<__m64&>(*in));
                default:
                    UNREACHABLE();
            }
        }


        template<typename IntegerType, typename>
        inline void Store64<IntegerType, void>( IntegerVector val, IntegerType * output )
        {
            _mm_storel_epi64( reinterpret_cast<IntegerVector *>( output ), val );
        }


        template<typename IntegerType, typename>
        inline void Store<IntegerType, void>( IntegerVector val, IntegerType * output )
        {
            _mm_storeu_si128( reinterpret_cast<__m128i*>( output ), val );
        }


        inline void Store( IntegerVector val, int64_t & output )
        {
            Store64( val, &output );
        }


        inline void Store( IntegerVector val, uint64_t & output )
        {
            Store64( val, &output );
        }
    }
}
