#pragma once
#include "CppUnitTest.h"
#include <Math\FixedPoint.h>
#include <Math\FloatTypes.h>
#include <Math\FloatMatrixTypes.h>
#include <Math\SparseMatrix.h>
#include <Math\Conversions.h>

#include <Utilities\UnitTest\ToString.h>

#include <string>
#include <vector>

namespace Math
{
    std::wstring const c_delim = L", ";
    std::wstring const c_start = L"[ ";
    std::wstring const c_end = L"] ";


    inline std::wstring ToString( const FixedPoint fp )
    {
        return std::to_wstring(DoubleFromFixedPoint(fp));
    }


    inline std::wstring ToString( const Math::Float2 & f )
    {
        return c_start + std::to_wstring( f[0] ) + c_delim + std::to_wstring( f[1] ) + c_end;
    }

    inline std::wstring ToString( const Math::Float3 & f )
    {
        return c_start + std::to_wstring( f[0] ) + c_delim + std::to_wstring( f[1] ) + c_delim + std::to_wstring( f[2] ) + c_end;
    }


    inline std::wstring ToString( const Math::Float4 & f )
    {
        return c_start + std::to_wstring( f[0] ) + c_delim + std::to_wstring( f[1] ) + c_delim + std::to_wstring( f[2] ) + c_delim + std::to_wstring( f[3] ) + c_end;
    }


    inline std::wstring ToString(const Math::Quaternion & f)
    {
        return ToString(Math::Float4(f));
    }


    inline std::wstring ToString( const Math::Float4x4 & m )
    {
        using namespace Microsoft::VisualStudio::CppUnitTestFramework;
        return c_start + ToString( m[0] ) + c_delim + ToString( m[1] ) + c_delim + ToString( m[2] ) + c_delim + ToString( m[3] ) + c_end;
    }
    

    inline std::wstring ToString( const SparseMatrix & m )
    {
        using Microsoft::VisualStudio::CppUnitTestFramework::ToString;
        std::wstring s;
        s += c_start + ToString(GetNumberOfRows(m)) + c_delim + ToString(GetNumberOfColumns(m)) + c_delim + ToString(GetNumberOfElements(m)) + c_end + c_delim;
        s += c_start + ToString( CreateRange(m.row_offsets) ) + c_end + c_delim;
        s += c_start + ToString( CreateRange(m.column_indices) ) + c_end + c_delim;
        s += c_start + ToString( CreateRange(m.values) ) + c_end;
        return s;
    }
}