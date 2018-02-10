#include "VectorAlgorithms.h"

#include <Utilities\Range.h>

#include <Utilities\IntegerRange.h>
#include "FloatTypes.h"

#include "SSETypes.h"
#include "SSEFloatFunctions.h"
#include "SSELoadStore.h"
#include "SSESetFunctions.h"

namespace Math
{
    namespace
    {
        auto const floats_in_sse = sizeof( SSE::Float32Vector ) / sizeof( float );
    }


    void Divide( Range<float *> x, float inverse_scale )
    {
        auto size = Size( x );
        float * xi = begin( x );
        for( auto i = 0u; i < size; ++i )
        {
            xi[i] /= inverse_scale;
        }
    }


    void Divide( Range<float const * __restrict> x, float inverse_scale, Range<float * __restrict> y )
    {
        assert( Size( x ) == Size( y ) );
        // make the bound local and use a raw pointers to help the compiler to vectorize the loop
        auto size = Size( x );
        float * __restrict yi = begin( y );
        float const * __restrict xi = begin( x );
        for( auto i = 0u; i < size; ++i )
        {
            yi[i] = xi[i] / inverse_scale;
        }
    }


    void Divide( Range<float const * __restrict> x, Range<float const * __restrict> y, Range<float * __restrict> z)
    {
        assert(Size(x) == Size(y));
        assert(Size(z) == Size(y));
        auto size = Size(x);
        auto * __restrict xi = begin(x);
        auto * __restrict yi = begin(y);
        auto * __restrict zi = begin(z);
        for( auto i = 0u; i < size; ++i )
        {
            zi[i] = xi[i] / yi[i];
        }
    }


    namespace
    {
        template<typename DataType>
        void AddImpl( DataType const x, Range< DataType * __restrict> y )
        {
            auto size = Size( y );
            auto * __restrict yi = begin( y );
            for( auto i = 0u; i < size; ++i )
            {
                yi[i] += x;
            }
        }
    }


    void Add( float const x, Range< float * __restrict> y )
    {
        AddImpl(x, y);
    }


    void Add( uint32_t const scalar, Range<uint32_t * __restrict> y)
    {
        AddImpl(scalar, y);
    }


    void Add( Range<float const * __restrict> x, float x_scale, Range< float * __restrict> y, float y_scale )
    {
        assert( Size( x ) == Size( y ) );
        auto size = Size( x );
        float * __restrict yi = begin( y );
        float const * __restrict xi = begin( x );
        for( auto i = 0u; i < size; ++i )
        {
            yi[i] = x_scale * xi[i] + y_scale * yi[i];
        }
    }


    void Add( Range<float const * __restrict> x, Range< float * __restrict> y, float y_scale )
    {
        assert( Size( x ) == Size( y ) );
        auto size = Size( x );
        float * __restrict yi = begin( y );
        float const * __restrict xi = begin( x );
        for( auto i = 0u; i < size; ++i )
        {
            yi[i] = xi[i] + y_scale * yi[i];
        }
    }


    void Add(float const * __restrict x, float x_scale, float * __restrict y, size_t size)
    {
        Add(x, x_scale, y, y, size);
    }


    void Add( Range<float const * __restrict> x, float x_scale, Range< float * __restrict> y )
    {
        assert( Size( x ) == Size( y ) );
        auto size = Size( x );
        float * __restrict yi = begin( y );
        float const * __restrict xi = begin( x );
        Add(xi, x_scale, yi, size);
    }


    void Add( Range<float const * __restrict> x, Range< float * __restrict> y )
    {
        assert( Size( x ) == Size( y ) );
        auto size = Size( x );
        float * __restrict yi = begin( y );
        float const * __restrict xi = begin( x );
        for( auto i = 0u; i < size; ++i )
        {
            yi[i] += xi[i];
        }
    }


    void Add(float const * __restrict x, float const * __restrict y, float * __restrict z, size_t size)
    {
        for(auto i = 0u; i < size; ++i)
        {
            z[i] = y[i] + x[i];
        }
    }


    void Add(Range<float const * __restrict> x, Range< float const * __restrict> y, Range< float * __restrict> z)
    {
        assert(Size(x) == Size(y));
        assert(Size(x) == Size(z));
        Add(begin(x), begin(y), begin(z), Size(x));
    }


    void Add(float const * __restrict x, float x_scale, float const * __restrict y, float * __restrict z, size_t size)
    {
        // for(auto i = 0u; i < size; ++i)
        // {
        //     z[i] = x_scale * x[i] + y[i];
        // }

        auto const sse_x_scale = SSE::SetAll( x_scale );

        auto kernel = [=](auto offset)
        {
            auto x_value = SSE::Load(x + offset);
            auto y_value = SSE::Load(y + offset);
            auto value0 = SSE::Multiply( x_value, sse_x_scale );
            auto value1 = SSE::Add(value0, y_value);
            SSE::Store( value1, z + offset );
        };

        size_t i = 0u;

        switch((size / floats_in_sse) % 4)
        {
            case 1:
                i -= 3 * floats_in_sse;
                goto one;
            case 2:
                i -= 2 * floats_in_sse;
                goto two;
            case 3:
                i -= floats_in_sse;
                goto three;
            case 0:
                goto four;
            default:
                break;
        }

        while( i + 4 * floats_in_sse <= size)
        {
        four:
            kernel(i);
        three:
            kernel(i + floats_in_sse);
        two:
            kernel(i + 2 * floats_in_sse);
        one:
            kernel(i + 3 * floats_in_sse);

            i += 4 * floats_in_sse;
        }

        auto extra = size % floats_in_sse;
        if( extra & 2 )
        {
            auto x_value = SSE::Load2(x + size - extra);
            auto y_value = SSE::Load2(y + size - extra);
            auto value0 = SSE::Multiply(x_value, sse_x_scale);
            auto value1 = SSE::Add(value0, y_value);
            SSE::Store2(value1, z + size - extra);
        }

        if(extra & 1)
        {
            auto x_value = SSE::LoadSingle(x + size - 1);
            auto y_value = SSE::LoadSingle(y + size - 1);
            auto value0 = SSE::MultiplySingle(x_value, sse_x_scale);
            auto value1 = SSE::AddSingle(value0, y_value);
            SSE::StoreSingle(value1, z + size - 1);
        }
    }


    void Add(Range<float const * __restrict> x, float x_scale, Range< float const * __restrict> y, Range< float * __restrict> z)
    {
        assert(Size(x) == Size(y));
        assert(Size(x) == Size(z));
        Add(begin(x), x_scale, begin(y), begin(z), Size(x));
    }


    void Multiply(float const scalar, float * __restrict y, size_t size)
    {
        for(auto i = 0u; i < size; ++i)
        {
            y[i] *= scalar;
        }
    }


    void Multiply(float const scalar, Range<float * __restrict> y)
    {
        Multiply(scalar, begin(y), Size(y));
    }



    void Multiply( Range<float const * __restrict> const x, Range<float * __restrict> y )
    {
        assert( Size( x ) == Size( y ) );

        //auto left = Size( x ) % floats_in_sse;
        //auto stop = end( x ) - left;
        //float const * __restrict xi = begin( x );
        //float* __restrict yi = begin( y );
        //while( xi != stop )
        //{
        //    auto value_x = SSE::LoadFloat32Vector( xi );
        //    auto value_y = SSE::LoadFloat32Vector( yi );
        //    auto value = SSE::Multiply( value_x, value_y );
        //    SSE::UnalignedStore( value, yi );

        //    xi += floats_in_sse;
        //    yi += floats_in_sse;
        //}

        //if( left & 2 )
        //{
        //    auto value_x = SSE::Load2( end(x) - left );
        //    auto value_y = SSE::Load2( end(y) - left );
        //    auto value = SSE::Multiply( value_x, value_y );
        //    SSE::Store2( value, end(y) - left );
        //}
        //if( left & 1)
        //{
        //    Last(y) *= Last(x);
        //}

        // make the bound local and use a raw pointers to help the compiler to vectorize the loop
        auto size = Size( x );
        float * __restrict yi = begin( y );
        float const * __restrict xi = begin( x );
        for( auto i = 0u; i < size; ++i )
        {
            yi[i] *= xi[i];
        }
    }


    void Multiply(Range<float const * __restrict > const x, Range<float const * __restrict > y, Range<float * __restrict > z)
    {
        assert(Size(x) == Size(y));
        assert(Size(x) == Size(z));
        // make the bound local and use a raw pointers to help the compiler to vectorize the loop
        auto size = Size(x);
        auto * __restrict yi = begin(y);
        auto * __restrict xi = begin(x);
        auto * __restrict zi = begin(z);
        for(auto i = 0u; i < size; ++i)
        {
            zi[i] = yi[i] * xi[i];
        }
    }


    void Multiply(float const * __restrict const x, float const y, float * __restrict z, size_t size)
    {
        for(auto i = 0u; i < size; ++i)
        {
            z[i] = y * x[i];
        }
    }


    void Multiply(Range<float const * __restrict > const x, float const y, Range<float * __restrict > z)
    {
        assert(Size(x) == Size(z));
        Multiply(begin(x), y, begin(z), Size(x));
    }


    float Dot( float const * __restrict const x, float const * __restrict y, size_t size )
    {
        // for (auto i = 0u; i < size; ++i)
        // {
        //     result += i[x] * i[y];
        // }
        // return result;

        auto result4 = SSE::ZeroFloat32Vector();
        if(size >= floats_in_sse)
        {
            for(size_t i = 0u; i + floats_in_sse <= size; i += floats_in_sse)
            {
                auto x_value = SSE::Load(x + i);
                auto y_value = SSE::Load(y + i);
                auto value0 = SSE::Multiply( x_value, y_value );
                result4 = SSE::Add(value0, result4);
            }

            result4 = SSE::HorizontalAdd(result4, result4);
            result4 = SSE::HorizontalAdd(result4, result4);
        }
        auto result = SSE::GetSingle(result4);

        auto extra = size % floats_in_sse;
        switch(extra)
        {
            case 3:
                result += x[size - 3] * y[size - 3];
            case 2:
                result += x[size - 2] * y[size - 2];
            case 1:
                result += x[size - 1] * y[size - 1];
            case 0:
                return result;
        }
        return result;
    }


    float Dot( Range<float const * __restrict > const x, Range<float const * __restrict > y )
    {
        assert(Size(x) == Size(y));
        return Dot(begin(x), begin(y), Size(x));
    }


    void Add(Math::Float3 const constant, Range<Math::Float3 * __restrict> const y_in)
    {
        float constants[12];
        constants[0] = constants[3] = constants[6] = constants[ 9] = constant.x;
        constants[1] = constants[4] = constants[7] = constants[10] = constant.y;
        constants[2] = constants[5] = constants[8] = constants[11] = constant.z;
        // constants[0] = Swizzle<0,1,2,0>(LoadFloat32Vector(&constant));
        // constants[1] = Swizzle<1,2,0,1>(constants[0]);
        // constants[2] = Swizzle<2,0,1,2>(constants[0]);

        auto * __restrict y_data = reinterpret_cast<float*>(begin(y_in));
        auto * __restrict y_data_end = reinterpret_cast<float*>(end(y_in));
        auto constants_range = CreateRange<float * __restrict>(constants, constants + 12);

        while( y_data < (y_data_end - 12) )
        {
            Add(constants_range, CreateRange(y_data, y_data + 12));
            y_data += 12;
        }

        Add(CreateRange(constants, y_data_end - y_data), CreateRange(y_data, y_data_end));
    }


    void Or(Range<bool const * __restrict> x, Range<bool * __restrict> y)
    {
        assert(Size(x) == Size(y));
        auto size = Size(x);
        auto * __restrict x_data = begin(x);
        auto * __restrict y_data = begin(y);
        for( auto i = 0u; i < size; ++i )
        {
            y_data[i] = y_data[i] || x_data[i];
        }
    }


    void Maximum(Range<unsigned const * __restrict> x, Range<unsigned * __restrict> y)
    {
        assert(Size(x) == Size(y));
        auto size = Size(x);
        auto * __restrict x_data = begin(x);
        auto * __restrict y_data = begin(y);
        for( auto i = 0u; i < size; ++i )
        {
            y_data[i] = y_data[i] > x_data[i] ? y_data[i] : x_data[i];
        }
    }
}
