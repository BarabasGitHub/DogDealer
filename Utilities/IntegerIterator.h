#pragma once
#include <type_traits>

/// iterator class that iterates over a consecutive range of numbers
template<typename Type, ptrdiff_t Increment = 1, typename = std::enable_if_t<std::is_integral<Type>::value>>
class IntegerIterator
{
private:
    Type m_count;
public:

    typedef typename std::make_signed_t<Type> SignedType;
    typedef typename std::make_unsigned_t<Type> UnSignedType;

    typedef Type value_type;
    typedef Type* pointer;
    //typedef SignedType difference_type;
    typedef std::make_signed_t<size_t> difference_type;
    typedef Type& reference;
    typedef std::random_access_iterator_tag iterator_category;

    IntegerIterator() = default;
    IntegerIterator( Type initial_count ) : m_count( initial_count ) {}
    bool operator==( IntegerIterator other ) const { return m_count == other.m_count; }
    bool operator!=( IntegerIterator other ) const { return m_count != other.m_count; }
    Type operator*( ) const { return m_count; }
    IntegerIterator& operator++( ) { m_count += Increment; return *this; }
    IntegerIterator operator++( int ) { auto out = *this; operator++( ); return out; }
    IntegerIterator& operator--( ) { m_count -= Increment; return *this; }
    IntegerIterator operator--( int ) { auto out = *this; operator--( ); return out; }
    // IntegerIterator& operator+=( UnSignedType count ) { m_count += count * Increment; return *this; }
    // IntegerIterator& operator-=( UnSignedType count ) { m_count -= count * Increment; return *this; }
    // IntegerIterator& operator+=( SignedType count ) { m_count += count * Increment; return *this; }
    // IntegerIterator& operator-=( SignedType count ) { m_count -= count * Increment; return *this; }
    IntegerIterator& operator+=( difference_type count ) { m_count += SignedType(count) * Increment; return *this; }
    IntegerIterator& operator-=( difference_type count ) { m_count -= SignedType(count) * Increment; return *this; }
    difference_type operator-( IntegerIterator other ) const { return difference_type(m_count - other.m_count) / Increment; }
    // IntegerIterator operator+( SignedType count ) const { return{ m_count + count * Increment }; }
    // IntegerIterator operator-( SignedType count ) const { return{ m_count - count * Increment }; }
    // IntegerIterator operator+( UnSignedType count ) const { return{ m_count + count * Increment }; }
    // IntegerIterator operator-( UnSignedType count ) const { return{ m_count - count * Increment }; }
    IntegerIterator operator+( difference_type count ) const { return{ m_count + SignedType(count) * Increment }; }
    IntegerIterator operator-( difference_type count ) const { return{ m_count - SignedType(count) * Increment }; }
    bool operator<( IntegerIterator other ) const { return Increment > 0 ? m_count < other.m_count : m_count > other.m_count; }
    bool operator>( IntegerIterator other ) const { return Increment > 0 ? m_count > other.m_count : m_count < other.m_count; }
    bool operator<=( IntegerIterator other ) const { return Increment > 0 ? m_count <= other.m_count : m_count >= other.m_count; }
    bool operator>=( IntegerIterator other ) const { return Increment > 0 ? m_count >= other.m_count : m_count <= other.m_count; }
    //Type operator[]( SignedType offset ) const { return m_count + offset * Increment; }
    //Type operator[]( UnSignedType offset ) const { return m_count + offset * Increment; }
    Type operator[]( difference_type offset ) const { return m_count + SignedType(offset) * Increment; }
};
