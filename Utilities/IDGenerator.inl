#include <algorithm>

template <typename IDType>
IDType IDGenerator<IDType>::NewID()
{
    IDType id;
    if(m_unused.empty())
    {
        auto index = static_cast<IDType::index_t>(m_generation.size());
        m_generation.emplace_back(uint8_t(0));
        id.index = index;
        id.generation = 0;
    }
    else
    {
        auto index = m_unused.front();
        m_unused.pop_front();
        id.index = index;
        id.generation = m_generation[index];
    }
    return id;
}


template <typename IDType>
bool IDGenerator<IDType>::IsValid(IDType id) const
{
    return id.index < m_generation.size() &&
        m_generation[id.index] == id.generation;
}

template <typename IDType>
void IDGenerator<IDType>::Remove(IDType id)
{
    if(IsValid(id))
    {
        m_unused.push_back(id.index);
        ++m_generation[id.index];
    }
}

template <typename IDType>
std::vector<IDType> IDGenerator<IDType>::Remove(std::vector<IDType> ids)
{
    // remove invalid ids
    ids.erase(std::remove_if(begin(ids), end(ids), [this](IDType const & id)
    {
        return !this->IsValid(id);
    }), end(ids));

    // increase generations
    for(auto const & id : ids)
    {
        ++m_generation[id.index];
    }

    // store unused ids
    m_unused.resize(m_unused.size() + ids.size());
    auto unused_iterator = m_unused.end() - ptrdiff_t(ids.size());
    for(auto const & id : ids)
    {
        *unused_iterator = id.index;
        ++unused_iterator;
    }

    return ids;
}


template <typename IDType>
void IDGenerator<IDType>::Remove(Range<IDType const *> ids)
{
    Remove(std::vector<IDType>{begin(ids), end(ids)});
}
