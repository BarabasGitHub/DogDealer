#pragma once

#include <vector>
#include <cstdint>
#include <algorithm> // for std::move

template<typename DataType>
struct DoublyLinkedList
{
    struct Element
    {
        uint32_t next, previous;
        DataType data;
        Element() = default;
        Element(Element const &) = default;
        Element(Element &&);
        Element(uint32_t next, uint32_t previous);
        template<typename InputDataType> Element(uint32_t next, uint32_t previous, InputDataType && data);
    };

    // the first element keeps track of where to begin/end
    std::vector<Element> elements;

    DataType & operator[](size_t i);
    DataType const & operator[](size_t i) const;

    DoublyLinkedList();
    DoublyLinkedList(DoublyLinkedList const &) = delete;
    DoublyLinkedList(DoublyLinkedList &&) = default;
};


template <typename DataType>
size_t Size(DoublyLinkedList<DataType> const & self)
{
    return Size(self.elements) - 1;
}


template <typename DataType>
size_t Capacity(DoublyLinkedList<DataType> const & self)
{
    return self.elements.capacity() - 1;
}


template <typename DataType>
void SetCapacity(size_t capacity, DoublyLinkedList<DataType> & self)
{
    self.elements.reserve(capacity + 1);
}


template <typename DataType>
bool IsEmpty(DoublyLinkedList<DataType> const & self)
{
    return First(self.elements).next == 0;
}


template <typename DataType>
void Empty(DoublyLinkedList<DataType> & self)
{
    self.elements.resize(1);
    First(self.elements).next = 0;
    First(self.elements).previous = 0;
}


template <typename DataType>
void Clear(DoublyLinkedList<DataType> & self)
{
    // clear
    self.~DoublyLinkedList<DataType>();
    // make new
    new (&self) DoublyLinkedList<DataType>();
}


template <typename InputDataType, typename ListDataType>
void AppendElement(InputDataType && data, DoublyLinkedList<ListDataType> & self)
{
    auto new_element_index = uint32_t(Size(self.elements));
    auto last_index = First(self.elements).previous;
    First(self.elements).previous = new_element_index;
    self.elements[last_index].next = new_element_index;
    self.elements.emplace_back(0, last_index, std::forward<InputDataType>(data));
}


// returns the index of the next element
template <typename DataType>
uint32_t RemoveElement(uint32_t index, DoublyLinkedList<DataType> & self)
{
    index += 1;
    auto & element = self.elements[index];
    auto next_index = element.next;
    // update previous and next elements, effectively removing this element from the list
    self.elements[element.previous].next = element.next;
    self.elements[element.next].previous = element.previous;
    // move the last element to the empty position
    auto & last = Last(self.elements);
    // first update the indices of the previous and next elements of the last element
    self.elements[last.previous].next = index;
    self.elements[last.next].previous = index;
    // move the last element
    element = std::move(last);
    // remove the last element
    self.elements.pop_back();
    return next_index - 1;
}


template <typename DataType> DataType & DoublyLinkedList<DataType>::operator[](size_t i)
{
    return this->elements[i + 1].data;
}


template <typename DataType> DataType const & DoublyLinkedList<DataType>::operator[](size_t i) const
{
    return this->elements[i + 1].data;
}


template <typename DataType>
uint32_t GetFirstIndex(DoublyLinkedList<DataType> const & self)
{
    return First(self.elements).next - 1;
}


template <typename DataType>
uint32_t GetNextIndex(DoublyLinkedList<DataType> const & self, uint32_t current_index)
{
    return self.elements[current_index + 1].next - 1;
}


// remove unused elements
template <typename DataType>
void Compact(DoublyLinkedList<DataType> & self)
{
    auto i = First(self.elements).next;
    // find the first gap
    auto next = self.elements[i].next;
    while(next == i + 1)
    {
        i += 1;
        next = self.elements[i].next;
    }
    auto j = i;
    while( j < next )
    {
        auto next_element = self.elements[next];
        next = next_element.next;
        auto destination_element = self.elements[j];
        destination_element = std::move(next_element);
        ++j;
        destination_element.next = j;
    }
    self.elements.resize(j);
    Last(self.elements).next = 0;
    First(self.elements).previous = j - 1;
}


template <typename DataType, typename FunctionType>
void IterateAndRemove(DoublyLinkedList<DataType> & self, FunctionType const & function)
{
    for(auto i = First(self.elements).next; i != 0; )
    {
        auto remove_element = function(self.elements[i].data);
        if(remove_element)
        {
            i = RemoveElement(i - 1, self) + 1;
        }
        else
        {
            i = self.elements[i].next;
        }
    }
}


template <typename DataType, typename FunctionType>
void Iterate(DoublyLinkedList<DataType> const & self, FunctionType const & function)
{
    for(auto i = First(self.elements).next; i != 0; i = self.elements[i].next)
    {
        function(self.elements[i].data);
    }
}


template <typename DataType> DoublyLinkedList<DataType>::Element::Element(Element && other) :
    Element(other.next, other.previous, std::move(other.data))
{}


template <typename DataType> DoublyLinkedList<DataType>::Element::Element(uint32_t next, uint32_t previous) :
    next(next),
    previous(previous)
{
}


template <typename DataType> template <typename InputDataType> DoublyLinkedList<DataType>::Element::Element(uint32_t next, uint32_t previous, InputDataType && data) :
    next(next),
    previous(previous),
    data(std::forward<InputDataType>(data))
{
}


template <typename DataType> DoublyLinkedList<DataType>::DoublyLinkedList()
{
    this->elements.emplace_back(0, 0);
}
