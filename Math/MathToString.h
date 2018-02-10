#pragma once
#include <Math\FloatTypes.h>
#include <Math\FloatMatrixTypes.h>
#include <Math\SparseMatrix.h>

#include <Utilities\Range.h>

#include <string>


namespace Math
{
    std::string const c_delim = ", ";
    std::string const c_start = "{ ";
    std::string const c_end = "} ";

    inline std::string ToString( const float f )
    {
        return std::to_string( f );
    }

    inline std::string ToString( const Math::Float2 & f )
    {
        return c_start + std::to_string( f[0] ) + c_delim + std::to_string( f[1] ) + c_end;
    }

    inline std::string ToString( const Math::Float3 & f )
    {
        return c_start + std::to_string( f[0] ) + c_delim + std::to_string( f[1] ) + c_delim + std::to_string( f[2] ) + c_end;
    }


    inline std::string ToString( const Math::Float4 & f )
    {
        return c_start + std::to_string( f[0] ) + c_delim + std::to_string( f[1] ) + c_delim + std::to_string( f[2] ) + c_delim + std::to_string( f[3] ) + c_end;
    }

    inline std::string ToString( const Math::Float4x4 & m )
    {
        return c_start + ToString( m[0] ) + c_delim + ToString( m[1] ) + c_delim + ToString( m[2] ) + c_delim + ToString( m[3] ) + c_end;
    }

    template<typename Type>
    std::string ToString( const Type & a )
    {
        return std::to_string( a );
    }

    template<typename Type>
    std::string ToString( Range<Type const *> v )
    {
        std::string o = c_start;
        for( auto & e : v )
        {
            o += ToString( e ) + c_delim;
        }
        o += c_end;
        return o;
    }


    inline std::string ToString( const SparseMatrix & m )
    {
        std::string s;
        s += c_start + ToString( GetNumberOfRows(m) ) + c_delim + ToString( GetNumberOfColumns(m) ) + c_delim + ToString( GetNumberOfElements(m) ) + c_end + c_delim;
        s += c_start + ToString( CreateRange(m.row_offsets) ) + c_end + c_delim;
        s += c_start + ToString( CreateRange(m.column_indices) ) + c_end + c_delim;
        s += c_start + ToString( CreateRange(m.values) ) + c_end;
        return s;
    }
}