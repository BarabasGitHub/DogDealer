#pragma once

#include "Memory.h"
#include "Range.h"
#include <cassert>
#include <cstdint>
#include <initializer_list>

template<typename DataType>
struct Vector;

template<typename DataType>
void swap( Vector<DataType> & a, Vector<DataType> & b );

template<typename DataType>
DataType const * begin( Vector<DataType> const & v );

template<typename DataType>
DataType * begin( Vector<DataType> & v );

template<typename DataType>
DataType const * end( Vector<DataType> const & v );

template<typename DataType>
DataType * end( Vector<DataType> & v );


template<typename DataType>
struct Vector
{
    DataType * data = nullptr;
    uint32_t size = 0;
    uint32_t capacity = 0;


    Vector() = default;


    Vector( Vector const & other ) :
        Vector( other.size )
    {
        Copy( other.data, size, this->data );
    }


    Vector( Vector && other )
    {
        swap( *this, other );
    }


    explicit Vector( uint32_t new_size ) :
        data( new DataType[new_size] ),
        size( new_size ),
        capacity( new_size )
    {
    }


    Vector(std::initializer_list<DataType> list) :
        Vector(Size(list))
    {
        Copy(begin(list), size, this->data);
    }


    explicit Vector(Range<DataType const *> range) :
        Vector(Size(range))
    {
        Copy(begin(range), size, this->data);
    }


    ~Vector()
    {
        delete[] data;
    }


    Vector& operator=( Vector const & other )
    {
        if( this->capacity < other.size )
        {
            this->~Vector();
            new (this) Vector( other );
        }
        else
        {
            this->size = other.size;
            Copy( other.data, other.size, this->data );
        }
        return *this;
    }


    Vector& operator=( Vector&& other )
    {
        swap( *this, other );
        return *this;
    }


    DataType& operator[]( uint32_t i )
    {
        assert( i < size );
        return this->data[i];
    }


    DataType const & operator[]( uint32_t i ) const
    {
        assert( i < size );
        return this->data[i];
    }


    operator Range<DataType*>()
    {
        return {begin(*this), end(*this)};
    }


    operator Range<DataType const *>()  const
    {
        return {begin(*this), end(*this)};
    }
};


template<typename DataType>
void swap( Vector<DataType> & a, Vector<DataType> & b )
{
    auto data = a.data;
    auto size = a.size;
    auto capacity = a.capacity;
    a.data = b.data;
    a.size = b.size;
    a.capacity = b.capacity;
    b.data = data;
    b.size = size;
    b.capacity = capacity;
}


template<typename DataType>
DataType const * begin( Vector<DataType> const & v )
{
    return v.data;
}


template<typename DataType>
DataType * begin( Vector<DataType> & v )
{
    return v.data;
}


template<typename DataType>
DataType const * end( Vector<DataType> const & v )
{
    return v.data + v.size;
}


template<typename DataType>
DataType * end( Vector<DataType> & v )
{
    return v.data + v.size;
}
