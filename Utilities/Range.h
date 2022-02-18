#pragma once
#include <iterator>
#include <type_traits>
#include <cassert>

#include "DefaultContainerFunctions.h"

// fix iterator traits for restricted pointers
template<class _Ty>
struct std::iterator_traits < _Ty * __restrict > : public std::iterator_traits < _Ty * >{};

// holds a begin and end pointer or iterator
// functions from DefaultContainerFunctions that work on the range: Size, IsEmpty, First, Last
template<typename IteratorType>
struct Range
{
    IteratorType start;
    IteratorType stop;
    // default constructor
    Range() = default;
    // default copy constructor
    Range( Range const & other ) = default;
    // construct a range from a begin and end
    Range( IteratorType begin, IteratorType end ) :
        start( begin ),
        stop( end )
    {}
    // construct a range from another range that has pointers/iterators convertible to this type
    // for example: int * to int const *
    template<typename OtherIteratorType, typename = std::enable_if_t<std::is_convertible<OtherIteratorType, IteratorType>::value>>
    Range( Range<OtherIteratorType> const & other ) :
        start( other.start ),
        stop( other.stop )
    {}
    // construct a range from a container that has the data() and size() member functions.
    // will create a range of the whole container
    template < typename ContainerType,
        // get pointer type from the data() member function on the container
        typename PointerType = decltype( std::declval<ContainerType>().data() ),
        // so we can also construct a pointer type to a const element
        typename = std::enable_if_t<std::is_convertible<PointerType, IteratorType>::value>>
        Range( ContainerType & container ) :
        start( container.data() ),
        stop( start + Size(container) )
    {}
    // access the range through the underlying operator[] of the pointer/iterator
    template <typename = void> // just so it won't compile unless you call it, so the size of the type doesn't have to be known
    auto operator[]( std::make_unsigned_t<typename std::iterator_traits<IteratorType>::difference_type> offset ) const ->decltype(start[offset])
    {
        assert( offset < Size(*this) );
        return start[offset];
    }
};

// return the begin
template<typename IteratorType>
IteratorType begin(Range<IteratorType> const & r) { return r.start; }
// return the end
template<typename IteratorType>
IteratorType end(Range<IteratorType> const & r) { return r.stop; }


// create a range form begin to end
template<typename Type>
Range<Type> CreateRange( Type begin, Type end ) { return{ begin, end }; }

// create a range from begin to begin + size
template<typename Type, typename = std::enable_if_t<std::is_pointer<Type>::value>>
Range<Type> CreateRange( Type begin, size_t size )
{
    return{ begin, begin + size };
}

// create a range from data() + begin to data() + end
template < typename IteratorType >
Range<IteratorType> CreateRange( Range<IteratorType> range, size_t begin, size_t end )
{
        assert( begin <= end );
        assert( end <= Size( range ) );
        auto pointer = ::begin(range);
        return{ pointer + begin, pointer + end };
}

// create a range from data() + begin to data() + end
template < typename IteratorType >
Range<IteratorType> CreateRange( Range<IteratorType> range, size_t begin )
{
    return CreateRange(range, begin, Size(range));
}

// create a range from data() + begin to data() + end
template < typename ContainerType,
    typename = std::enable_if_t<std::is_class<ContainerType>::value>,
    // get pointer type from the data() member function on the container
    typename PointerType = decltype( std::declval<ContainerType>().data() ),
    typename = std::enable_if_t < std::is_pointer<PointerType>::value >>
Range<PointerType> CreateRange( ContainerType & container, size_t begin, size_t end )
{
    assert( begin <= end );
    assert( end <= Size(container) );
    auto pointer = container.data();
    return{ pointer + begin, pointer + end };
}


// create a range over the whole container or from data() to data() + size()
template < typename ContainerType,
    typename = std::enable_if_t<std::is_class<ContainerType>::value>,
    // get pointer type from the data() member function on the container
    typename PointerType = decltype( std::declval<ContainerType>( ).data( ) ),
    typename = std::enable_if_t < std::is_pointer<PointerType>::value >>
Range<PointerType> CreateRange( ContainerType & container )
{
    auto pointer = container.data();
    return{ pointer, pointer + Size(container) };
}



// Create a range of a different type that points to the same data.
// Only allowed for ranges of pointers and this can only work correctly if the underlying data is of the same type.
// So for example a float3 range can be reinterpreted as a float range (or the other way around).
template<typename OutputValueType, typename InputValueType >
Range<OutputValueType *> ReinterpretRange( Range<InputValueType *> input )
{
    const bool input_is_bigger = sizeof( InputValueType ) > sizeof( OutputValueType );
    static_assert(
        ( input_is_bigger && ( sizeof( InputValueType ) % sizeof( OutputValueType ) == 0 ) ) ||
        ( sizeof( OutputValueType ) % sizeof( InputValueType ) == 0 ), "Reinterpet cast won't work." );

    auto const number =
        input_is_bigger ?
        sizeof( InputValueType ) / sizeof( OutputValueType ) :
        sizeof( OutputValueType ) / sizeof( InputValueType );

    auto size = Size( input );
    input_is_bigger ? assert( true ) : assert( size % number == 0 );
    input_is_bigger ? size *= number : size /= number ;

    return CreateRange( reinterpret_cast<OutputValueType *>( begin( input ) ), size );
}

// exclude the first element from the range.
template<typename IteratorType>
void PopFirst(Range<IteratorType> & self)
{
    assert(!IsEmpty(self));
    ++self.start;
}
