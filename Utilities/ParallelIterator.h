#pragma once

#include <tuple>
#include <iterator>

#include <cassert>

template<typename IteratorType1, typename IteratorType2>
class ParallelIterator : public std::tuple<IteratorType1, IteratorType2>
{
public:
    typedef std::ptrdiff_t difference_type;
    typedef std::tuple<typename std::iterator_traits<IteratorType1>::value_type, typename std::iterator_traits<IteratorType2>::value_type> value_type;
    typedef std::tuple<typename std::iterator_traits<IteratorType1>::reference, typename std::iterator_traits<IteratorType2>::reference> reference;
    typedef value_type* pointer;
    typedef std::random_access_iterator_tag iterator_category;

    ParallelIterator( std::tuple<IteratorType1, IteratorType2> const & in ) : std::tuple<IteratorType1, IteratorType2>( in )
    {
    }
    reference operator*( ) const { return std::tie( *std::get<0>(*this ), *std::get<1>(*this ) ); }
    ParallelIterator& operator++( ) { ++std::get<0>( *this  ); ++std::get<1>(*this); return *this; }
    ParallelIterator operator++( int ) { auto out = *this; ++*this; return out; }
    ParallelIterator& operator--( ) { --std::get<0>(*this); --std::get<1>(*this); return *this; }
    ParallelIterator operator--( int ) { auto out = *this; --*this; return out; }
    difference_type operator - ( ParallelIterator const & other ) const
    {
        auto diff0 = std::get<0>(*this) - std::get<0>( other );
        assert( diff0 == std::get<1>( *this ) - std::get<1>( other ) );
        return diff0;
    }
    ParallelIterator& operator+=( difference_type offset ) { std::get<0>(*this) += offset; std::get<1>(*this) += offset; return *this; }
    ParallelIterator& operator-=( difference_type offset ) { std::get<0>(*this) -= offset; std::get<1>(*this) -= offset; return *this; }
    ParallelIterator operator+( difference_type offset ) const { auto out = *this; return out += offset; }
    ParallelIterator operator-( difference_type offset ) const { auto out = *this; return out -= offset; }
    reference operator[]( difference_type offset ) const { return *( operator+( offset ) ); }

private:

};

template<typename IteratorType1, typename IteratorType2>
ParallelIterator<IteratorType1, IteratorType2> CreateParallelIterator( IteratorType1 it1, IteratorType2 it2 ) { return std::make_tuple( it1, it2 ); }

namespace std
{
    template<typename IteratorType1, typename IteratorType2> inline
        void iter_swap(ParallelIterator<IteratorType1, IteratorType2> left, ParallelIterator<IteratorType1, IteratorType2> right)
    {	
        iter_swap(std::get<0>(left), std::get<0>(right));
        iter_swap(std::get<1>(left), std::get<1>(right));
    }
}