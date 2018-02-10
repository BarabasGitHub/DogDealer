#pragma once
#include "FloatTypes.h"
// Overloads on >> operator for istreams
#include <istream>
template<typename _Elem, class _Traits> std::basic_istream<_Elem,_Traits> &operator>>( std::basic_istream<_Elem,_Traits>& stream, Math::Float4& vector )
{
    stream >> vector.x;
    stream >> vector.y;
    stream >> vector.z;
    stream >> vector.w;
    return stream;
}

template<typename _Elem, class _Traits> std::basic_istream<_Elem,_Traits> &operator>>( std::basic_istream<_Elem,_Traits>& stream, Math::Float3& vector )
{
    stream >> vector.x;
    stream >> vector.y;
    stream >> vector.z;
    return stream;
}

template<typename _Elem, class _Traits> std::basic_istream<_Elem,_Traits> &operator>>( std::basic_istream<_Elem,_Traits>& stream, Math::Float2& vector )
{
    stream >> vector.x;
    stream >> vector.y;
    return stream;
}

// Overloads on << operator for ostreams
#include <ostream>
template<typename _Elem, class _Traits> std::basic_ostream<_Elem,_Traits> &operator<<( std::basic_ostream<_Elem,_Traits>& stream, Math::Float4 vector )
{
    stream << vector.x;
    stream << " ";
    stream << vector.y;
    stream << " ";
    stream << vector.z;
    stream << " ";
    stream << vector.w;
    return stream;
}

template<typename _Elem, class _Traits> std::basic_ostream<_Elem,_Traits> &operator<<( std::basic_ostream<_Elem,_Traits>& stream, Math::Float3 vector )
{
    stream << vector.x;
    stream << " ";
    stream << vector.y;
    stream << " ";
    stream << vector.z;
    return stream;
}

template<typename _Elem, class _Traits> std::basic_ostream<_Elem,_Traits> &operator<<( std::basic_ostream<_Elem,_Traits>& stream, Math::Float2 vector )
{
    stream << vector.x;
    stream << " ";
    stream << vector.y;
    return stream;
}

