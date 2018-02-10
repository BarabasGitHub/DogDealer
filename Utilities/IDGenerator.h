#pragma once

#include "Range.h"
#include <vector>
#include <deque>

template<typename IDType>
class IDGenerator
{
    std::vector<typename IDType::generation_t> m_generation;
    std::deque<typename IDType::index_t> m_unused;

public:

    bool IsValid(IDType id ) const;

    IDType NewID();

    void Remove(IDType id);

    // removes all valid ids in the vector
    // returns a vector with the valid and thus removed entity ids
    std::vector<IDType> Remove( std::vector<IDType> ids );
    // removes all valid ids in the range
    void Remove(Range<IDType const *> ids);
};


#include "IDGenerator.inl"
