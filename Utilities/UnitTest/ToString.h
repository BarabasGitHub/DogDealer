#pragma once

#include <string>

#include "../Range.h"

#include "CppUnitTest.h"

namespace ToStringDetails
{
    std::wstring const c_delim = L", ";
    std::wstring const c_start = L"[ ";
    std::wstring const c_end = L"] ";
}

template<typename Type>
std::wstring ToString( const Range<Type> & v )
{
    using Microsoft::VisualStudio::CppUnitTestFramework::ToString;
    std::wstring o = ToStringDetails::c_start;
    for( auto & e : v )
    {
        o += ToString( e ) + ToStringDetails::c_delim;
    }
    return o + ToStringDetails::c_end;
}



template<typename Type>
std::wstring ToString(const std::vector<Type> & v)
{
    return ToString(CreateRange(v));
}