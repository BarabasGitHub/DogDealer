#pragma once
#include "IntegerIterator.h"
#include "Range.h"

template<typename IntegerType, ptrdiff_t Increment = 1>
using IntegerRange = Range<IntegerIterator<IntegerType, Increment>>;


// create a range of numbers from 0 to end
template<typename Type>
IntegerRange<Type> CreateIntegerRange( Type end )
{
    assert( end >= 0 );
    typedef IntegerIterator<Type> IteratorType;
    return{ IteratorType( 0 ), IteratorType( end ) };
}

// create a range of number from begin to end
template<ptrdiff_t Increment = 1, typename Type1, typename Type2>
auto CreateIntegerRange( Type1 begin, Type2 end )
{
    typedef decltype( begin + end ) IntegerType;
    assert( Increment > 0 ? IntegerType(begin) <= IntegerType(end) : IntegerType(begin) >= IntegerType(end) );    
    return IntegerRange<IntegerType, Increment>{IntegerType(begin), IntegerType(end)};
}