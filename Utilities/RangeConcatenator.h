#pragma once

#include "Range.h"
#include <tuple>

// NOTE: this class is not well-tested!
template<typename IteratorType>
struct RangeConcatenation : std::pair<Range<IteratorType>, Range<IteratorType>>
{
    struct iterator
    {
        typedef typename IteratorType::value_type value_type;
        typedef typename IteratorType::pointer pointer;
        typedef typename IteratorType::difference_type difference_type;
        typedef typename IteratorType::reference reference;
        typedef std::random_access_iterator_tag iterator_category;

        IteratorType internal_iterator;
        bool first;
        RangeConcatenation const * parent;

        bool operator==( iterator other ) { return std::tie( parent, first, internal_iterator ) == std::tie( other.parent, other.first, other.internal_iterator ); }
        bool operator!=( iterator other ) { return !( *this == other ); }
        auto operator*( ) const ->decltype( *internal_iterator ) & { return *internal_iterator; }
        auto operator*( ) ->decltype( *internal_iterator ) & { return *internal_iterator; }
        iterator& operator++( )
        {
            ++internal_iterator;
            if(first && internal_iterator == end(parent->first) )
            {
                first = false;
                internal_iterator = begin(parent->second);
            }
            return *this;
        }
        iterator operator++( int ) { auto out = *this; operator++( ); return out; }
        iterator& operator--( )
        {
            if(!first && internal_iterator == begin(parent->second) )
            {
                first = true;
                internal_iterator = end(parent->first);
            }
            --internal_iterator;
            return *this;
        }
        iterator operator--( int ) { auto out = *this; operator--( ); return out; }

        iterator& operator+=( difference_type offset ) 
        {            
            if( first )
            {
                auto first_distance = std::distance( internal_iterator, end( parent->first ) );
                assert( first_distance > 0 );
                if( offset > first_distance )
                {
                    assert( offset > 0 );
                    first = false;
                    internal_iterator = begin( parent->second );
                    offset -= first_distance + 1;
                }
            }
            else
            {
                auto second_distance = std::distance( begin( parent->second ), internal_iterator );
                assert( second_distance < 0 );
                if( offset < second_distance )
                {
                    assert( offset < 0 );
                    first = true;
                    internal_iterator = end( parent->first );
                    offset -= second_distance + 1;
                }
            }
            internal_iterator += offset;
            return *this; 
        }

        iterator& operator-=( difference_type offset )
        {
            return operator+=( -offset );
        }

        iterator operator+( difference_type offset ) { auto out = *this; return out += offset; }
        iterator operator-( difference_type offset ) { auto out = *this; return out -= offset; }

        reference const operator[]( difference_type offset ) const { return *( operator+( offset ) ); }
        reference operator[]( difference_type offset ) { return *( operator+( offset ) ); }

        friend void swap( iterator& first, iterator& second )
        {
            using std::swap;
            swap( std::tie( first.internal_iterator, first.first, first.parent ), std::tie( second.internal_iterator, second.first, second.parent ) );
        }
    };

    typedef const iterator const_iterator;

    RangeConcatenation( std::pair<Range<IteratorType>, Range<IteratorType>> pair ) :
        std::pair<Range<IteratorType>, Range<IteratorType>>( pair )
    {}
};

template<typename Type> RangeConcatenation<Type>::const_iterator begin( RangeConcatenation<Type> const & r ) 
{ 
    using std::begin;
    return { begin( r.first ), true, &r }; 
}
template<typename Type> RangeConcatenation<Type>::const_iterator end(RangeConcatenation<Type> const & r)
{ 
    using std::end;
    return{ end( r.second ), false, &r };
}
template<typename Type> RangeConcatenation<Type>::iterator begin(RangeConcatenation<Type> & r ) 
{ 
    using std::begin;
    return { begin(r.first), true, &r }; 
}
template<typename Type> RangeConcatenation<Type>::iterator end(RangeConcatenation<Type> & r) 
{ 
    using std::end;
    return{ end( r.second ), false, &r };
}

template<typename Type>
RangeConcatenation<Type> ConcatenateRanges( Range<Type> first, Range<Type> second ) 
{
    return std::make_pair( first, second );
}