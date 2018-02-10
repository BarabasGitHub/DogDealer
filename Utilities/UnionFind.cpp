#include "UnionFind.h"


void InitializeUnionFind(Range<uint32_t *> parents)
{
    for( auto i = 0u; i < Size(parents); ++i )
    {
        parents[i] = i;
    }
}


uint32_t GetRoot(Range<uint32_t const *> parents, uint32_t index)
{
    auto parent = parents[index];
    while(index != parent)
    {
        auto grandparent = parents[parent];
        index = parent;
        parent = grandparent;
    }
    return index;
}


uint32_t GetRoot(Range<uint32_t *> parents, uint32_t index)
{
    auto parent = parents[index];
    while(index != parent)
    {
        index = parent;
        parent = parents[index];
    }
    return index;
}


bool Find(Range<uint32_t const*> parents, uint32_t index_a, uint32_t index_b)
{
    return GetRoot(parents, index_a) == GetRoot(parents, index_b);
}


void Unite(uint32_t index_a, uint32_t index_b, Range<uint32_t *> parents)
{
    auto root_a = GetRoot(parents, index_a);
    auto root_b = GetRoot(parents, index_b);
    parents[root_a] = root_b;
}