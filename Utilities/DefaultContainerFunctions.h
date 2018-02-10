#pragma once

#include <type_traits>
#include <cassert>
#include <iterator>

/// return the size
template<typename ContainerType>
auto constexpr Size( ContainerType && c )
{
    return std::make_unsigned_t < typename std::iterator_traits< decltype( begin( c ) ) >::difference_type >(std::distance( begin( c ), end( c ) ));
}


template<typename ContainerType>
bool constexpr IsEmpty( ContainerType && c )
{
    return begin( c ) == end( c );
}


template<typename ContainerType>
constexpr inline decltype(auto) First( ContainerType && c ) //Bas 14-12-2015: Added the inline, otherwise it would complain about unreferenced function
{
    assert( IsEmpty( c ) == false );
    return *begin( c );
}


template<typename ContainerType>
constexpr inline decltype(auto) Last(ContainerType && c) //Bas 14-12-2015: Added the inline, otherwise it would complain about unreferenced function
{
    assert( IsEmpty( c ) == false );
    return end(c)[-1];
}
