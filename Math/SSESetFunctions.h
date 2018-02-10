#pragma once

#include "SSETypes.h"

namespace Math
{
    namespace SSE
    {
        IntegerVector Set( int32_t x, int32_t y, int32_t z, int32_t w );
        IntegerVector SetAll(int8_t x);
        IntegerVector SetAll(int16_t x);
        IntegerVector SetAll( int32_t x );
        IntegerVector SetSingle( int32_t x );
        IntegerVector Set( int64_t x, int64_t y );
        IntegerVector SetAll( int64_t x );
        IntegerVector Set(
            int8_t x, int8_t y, int8_t z, int8_t w, int8_t i, int8_t j, int8_t k, int8_t l,
            int8_t x2, int8_t y2, int8_t z2, int8_t w2, int8_t i2, int8_t j2, int8_t k2, int8_t l2);
        IntegerVector Set(
            uint8_t x, uint8_t y, uint8_t z, uint8_t w, uint8_t i, uint8_t j, uint8_t k, uint8_t l,
            uint8_t x2, uint8_t y2, uint8_t z2, uint8_t w2, uint8_t i2, uint8_t j2, uint8_t k2, uint8_t l2);
        IntegerVector Set(int16_t x, int16_t y, int16_t z, int16_t w, int16_t i, int16_t j, int16_t k, int16_t l);
        IntegerVector Set( uint32_t x, uint32_t y, uint32_t z, uint32_t w );
        IntegerVector SetAll(uint8_t x);
        IntegerVector SetAll(uint16_t x);
        IntegerVector SetAll( uint32_t x );
        IntegerVector SetSingle( uint32_t x );
        IntegerVector Set( uint64_t x, uint64_t y );
        IntegerVector SetAll( uint64_t x );
        Float32Vector Set( float const x, float const y, float const z, float const w );
        Float32Vector SetAll( const float value );
        // sets only the first value of the vector, the rest will be set to zero
        Float32Vector SetSingle( float value );
        // returns the first value in the vector
        float GetSingle( Float32Vector in );
        int32_t GetSingle32BitSigned( IntegerVector a);
        uint32_t GetSingle32BitUnsigned( IntegerVector a);
        int64_t GetSingle64BitSigned( IntegerVector a );
        uint64_t GetSingle64BitUnsigned( IntegerVector a );
        // probably inefficient
        template<uint8_t Index> float Extract( Float32Vector in );
        // get a specific element (these are not straightforward)
        // template<uint8_t Index> int8_t Extract8BitSigned( IntegerVector in );
        // template<uint8_t Index> uint8_t Extract8BitUnsigned( IntegerVector in );
        // get a specific element (these are not straightforward)
        // template<uint8_t Index> int16_t Extract16BitSigned( IntegerVector in );
        // template<uint8_t Index> uint16_t Extract16BitUnsigned( IntegerVector in );
        // get a specific element
        template<uint8_t Index> int32_t Extract32BitSigned( IntegerVector in );
        template<uint8_t Index> uint32_t Extract32BitUnsigned( IntegerVector in );
        // get a specific element
        template<uint8_t Index> int64_t Extract64BitSigned( IntegerVector in );
        template<uint8_t Index> uint64_t Extract64BitUnsigned( IntegerVector in );
        // returns zero
        Float32Vector ZeroFloat32Vector();


        // implementations

        inline IntegerVector SetAll(int8_t x)
        {
            return _mm_set1_epi8(x);
        }


        inline IntegerVector SetAll(uint8_t x)
        {
            return _mm_set1_epi8(x);
        }


        inline IntegerVector Set(
            int8_t x, int8_t y, int8_t z, int8_t w, int8_t i, int8_t j, int8_t k, int8_t l,
            int8_t x2, int8_t y2, int8_t z2, int8_t w2, int8_t i2, int8_t j2, int8_t k2, int8_t l2)
        {
            return _mm_set_epi8(l2, k2, j2, i2, w2, z2, y2, x2, l, k, j, i, w, z, y, x);
        }


        inline IntegerVector Set(
            uint8_t x, uint8_t y, uint8_t z, uint8_t w, uint8_t i, uint8_t j, uint8_t k, uint8_t l,
            uint8_t x2, uint8_t y2, uint8_t z2, uint8_t w2, uint8_t i2, uint8_t j2, uint8_t k2, uint8_t l2)
        {
            return Set(int8_t(x), int8_t(y), int8_t(z), int8_t(w), int8_t(i), int8_t(j), int8_t(k), int8_t(l),
                int8_t(x2), int8_t(y2), int8_t(z2), int8_t(w2), int8_t(i2), int8_t(j2), int8_t(k2), int8_t(l2));
        }

        inline IntegerVector Set(int16_t x, int16_t y, int16_t z, int16_t w, int16_t i, int16_t j, int16_t k, int16_t l)
        {
            return _mm_set_epi16(l, k, j, i, w, z, y, x);
        }


        inline IntegerVector Set(uint16_t x, uint16_t y, uint16_t z, uint16_t w, uint16_t i, uint16_t j, uint16_t k, uint16_t l)
        {
            return Set(int16_t(x), int16_t(y), int16_t(z), int16_t(w), int16_t(i), int16_t(j), int16_t(k), int16_t(l));
        }


        inline IntegerVector SetAll(int16_t x)
        {
            return _mm_set1_epi16(x);
        }


        inline IntegerVector SetAll(uint16_t x)
        {
            return SetAll(int16_t(x));
        }


        inline IntegerVector Set( int32_t x, int32_t y, int32_t z, int32_t w )
        {
            return _mm_set_epi32(w, z, y, x);
        }


        inline IntegerVector SetAll( int32_t x )
        {
            return _mm_set1_epi32( x );
        }


        inline IntegerVector SetSingle( int32_t x )
        {
            return _mm_cvtsi32_si128(x);
        }


        inline IntegerVector Set( int64_t x, int64_t y )
        {
            return _mm_set_epi64x( x, y );
        }


        inline IntegerVector SetAll( int64_t x )
        {
            return _mm_set1_epi64x( x );
        }


        inline IntegerVector Set( uint32_t x, uint32_t y, uint32_t z, uint32_t w )
        {
            return Set( int32_t(x), int32_t(y), int32_t(z), int32_t(w) );
        }


        inline IntegerVector SetAll( uint32_t x )
        {
            return SetAll( int32_t(x) );
        }


        inline IntegerVector SetSingle( uint32_t x )
        {
            return SetSingle( int32_t(x) );
        }


        inline IntegerVector Set( uint64_t x, uint64_t y )
        {
            return Set( int64_t(x), int64_t(y) );
        }


        inline IntegerVector SetAll( uint64_t x )
        {
            return SetAll( int64_t(x) );
        }


        inline Float32Vector Set( float const x, float const y, float const z, float const w )
        {
            return _mm_setr_ps( x, y, z, w );
        }


        inline Float32Vector SetAll( const float value )
        {
            return _mm_set_ps1( value );
        }


        // sets only the first value of the vector, the rest will be set to zero
        inline Float32Vector SetSingle( float value )
        {
            return _mm_set_ss( value );
        }


        // returns the first value in the vector
        inline float GetSingle( Float32Vector in )
        {
            return _mm_cvtss_f32( in );
        }


        inline int32_t GetSingle32BitSigned( IntegerVector a )
        {
            return _mm_cvtsi128_si32(a);
        }


        inline uint32_t GetSingle32BitUnsigned( IntegerVector a )
        {
            return uint32_t(_mm_cvtsi128_si32(a));
        }


        inline int64_t GetSingle64BitSigned( IntegerVector a )
        {
            return _mm_cvtsi128_si64( a );
        }


        inline uint64_t GetSingle64BitUnsigned( IntegerVector a )
        {
            return uint64_t(_mm_cvtsi128_si64( a ));
        }


        template<uint8_t Index>
        inline float Extract( Float32Vector in )
        {
            auto integer = _mm_extract_ps( in, Index );
            return reinterpret_cast<float&>( integer );
        }


        template<uint8_t Index> int32_t Extract32BitSigned( IntegerVector in )
        {
            return _mm_extract_epi32(in, Index);
        }


        template<uint8_t Index> uint32_t Extract32BitUnsigned( IntegerVector in )
        {
            int32_t result = Extract32BitSigned<Index>(in);
            return reinterpret_cast<uint32_t&>(result);
        }


        template<uint8_t Index> int64_t Extract64BitSigned( IntegerVector in )
        {
            return _mm_extract_epi64(in, Index);
        }


        template<uint8_t Index> uint64_t Extract64BitUnsigned( IntegerVector in )
        {
            int64_t result = Extract64BitSigned<Index>(in);
            return reinterpret_cast<uint64_t&>(result);
        }


        // returns zero
        inline Float32Vector ZeroFloat32Vector()
        {
            return _mm_setzero_ps();
        }
    }
}
