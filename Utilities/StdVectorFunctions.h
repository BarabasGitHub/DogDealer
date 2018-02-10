#pragma once
#include "DefaultContainerFunctions.h"
#include "Range.h"

#include <vector>
#include <cstdint>

// increase the size by one, allocating new storage if necessary
template<typename DataType> void Grow(std::vector<DataType> & self);

// increase the size by count, allocating new storage if necessary, returns the newly added range of data
template<typename DataType> Range<DataType *> Grow( std::vector<DataType> & self, size_t count );

// decrease the size by count, does not reallocate
template<typename DataType> void Shrink( std::vector<DataType> & self, size_t count );

// Clears the vector and make it the specified size, allocating more storage if necessary. Does not decrease the allocated storage.
template<typename DataType> void ResetSize(std::vector<DataType> & self, size_t size );

// explicitly set the capacity, assuming that the size is smaller.
template<typename DataType> void SetCapacity(std::vector<DataType> & self, size_t capacity);

// clears and deallocates all storage
template<typename DataType> void Clear(std::vector<DataType> & self);

// sets the capacity equal to the size
template<typename DataType> void ShrinkToFit(std::vector<DataType> & self);

// append one value to the vector
template<typename DataType> void Append(std::vector<DataType> & self, DataType value);

// append value a number of times
template<typename DataType> Range<DataType*> Append(std::vector<DataType> & self, DataType value, size_t count);

// append the range of values to the vector
template<typename DataType, typename Iterator> Range<DataType*> Append(std::vector<DataType> & self, Range<Iterator> range);

// append a vector to a vector
template<typename DataType> Range<DataType*> Append(std::vector<DataType> & self, std::vector<DataType> const & other);

// insert an element before the element currently at 'index' (that is between the current elements at 'index - 1' and 'index')
template<typename DataType> void Insert(DataType value, uint32_t index, std::vector<DataType> & self);


template<typename DataType> void Append(std::vector<DataType> & self, DataType value)
{
    self.push_back(value);
}


template<typename DataType> Range<DataType*> Append(std::vector<DataType> & self, DataType value, size_t count)
{
    auto old_size = Size(self);
    auto new_size = count + old_size;
    self.resize(new_size, value);
    return CreateRange(self, old_size, new_size);
}


template<typename DataType, typename Iterator> Range<DataType*> Append(std::vector<DataType> & self, Range<Iterator> range)
{
    auto old_size = Size(self);
    self.insert(end(self), begin(range), end(range));
    auto new_size = Size(self);
    return CreateRange(self, old_size, new_size);
}


template<typename DataType> Range<DataType*> Append(std::vector<DataType> & self, std::vector<DataType> const & other)
{
    return Append(self, CreateRange(other));
}


template<typename DataType> void Insert(DataType value, uint32_t index, std::vector<DataType> & self)
{
    self.insert(begin(self) + index, value);
}


template<typename DataType> void Clear(std::vector<DataType> & self)
{
    self.clear();
    self.shrink_to_fit();
}


template<typename DataType> void ShrinkToFit(std::vector<DataType> & self)
{
    self.shrink_to_fit();
}


template<typename DataType>
void SetCapacity(std::vector<DataType> & self, size_t capacity)
{
    assert(Size(self) <= capacity);
    self.reserve(capacity);
}


template<typename DataType>
void Grow(std::vector<DataType>& self)
{
    self.emplace_back();
}


template<typename DataType>
Range<DataType *> Grow( std::vector<DataType>& self, size_t count )
{
    auto old_size = Size(self);
    auto new_size = old_size + count;
    self.resize(new_size);
    return CreateRange(self, old_size, new_size);
}


template<typename DataType> void Shrink( std::vector<DataType> & self, size_t count )
{
    auto old_size = Size( self );
    assert( old_size >= count );
    self.resize( old_size - count );
}


template<typename DataType>
void ResetSize( std::vector<DataType>& self, size_t size )
{
    self.clear();
    self.resize(size);
}
