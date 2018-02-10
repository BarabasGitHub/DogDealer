#include "SparseAdjacencyMatrix.h"
#include "SparseAdjacencyMatrixFunctions.h"
#include "Edge.h"
#include "VectorAlgorithms.h"

#include <Utilities\StdVectorFunctions.h>
#include <Utilities\VectorHelper.h>

#include <algorithm>

using namespace Math;


void Math::AddEdge(Edge edge, SparseAdjacencyMatrix& self)
{
    EnsureElementExists(edge.source + 1, Last(self.offsets), self.offsets);
    auto start = self.offsets[edge.source];
    auto stop = self.offsets[edge.source + 1];
    auto found = std::lower_bound(begin(self.indices) + start, begin(self.indices) + stop, edge.destination);
    if(found == begin(self.indices) + stop || *found != edge.destination)
    {
        self.indices.insert(found, edge.destination);
        Add(1, CreateRange(self.offsets, edge.source + 1, Size(self.offsets)));
    }
}


void Math::RemoveEdge(Edge edge, SparseAdjacencyMatrix& self)
{
    if(edge.source < Size(self.offsets))
    {
        auto start = self.offsets[edge.source];
        auto stop = self.offsets[edge.source + 1];
        auto found = std::lower_bound(begin(self.indices) + start, begin(self.indices) + stop, edge.destination);
        if(found != begin(self.indices) + stop && *found == edge.destination)
        {
            self.indices.erase(found);
            Add(uint32_t(-1), CreateRange(self.offsets, edge.source + 1, Size(self.offsets)));
        }

    }
}


void Math::AddSymmetricEdges(Range<Edge const *> edges, SparseAdjacencyMatrix& self)
{
    for( auto e : edges )
    {
        AddEdge(e, self);
        AddEdge({e.destination, e.source}, self);
    }
}


void Math::RemoveSymmetricEdges(Range<Edge const *> edges, SparseAdjacencyMatrix& self)
{
    for(auto e : edges)
    {
        RemoveEdge(e, self);
        RemoveEdge({e.destination, e.source}, self);
    }
}


void Math::AddEdges(Range<Edge const *> edges, SparseAdjacencyMatrix& self)
{
    // this can be done more efficiently if they vertex pairs are sorted
    AddEdgesUnsorted(edges, self);
}


void Math::RemoveEdges(Range<Edge const *> edges, SparseAdjacencyMatrix& self)
{
    // this can be done more efficiently if they vertex pairs are sorted
    RemoveEdgesUnsorted(edges, self);
}

// void Math::AddEdgesSorted(Range<Edge const *> edges, SparseAdjacencyMatrix& self)
// {
//     Grow(self.indices, Size(edges));
//     auto indices_destination = end(self.indices);
//     auto indices_source = end(self.indices) - Size(edges);
//     for (auto i = count; i > 0; )
//     {
//         i -= 1;

//         auto edge = edges[i];
//         if(i > 0 && edge.source == edges[i - 1].source)
//         {
//             auto next_edge_destination = edges[i - 1].destination;
//             std::lower_bound(begin(self.indices) +
//         }
//         else
//         {
//         }
//     }
// }


void Math::AddEdgesUnsorted(Range<Edge const *> edges, SparseAdjacencyMatrix& self)
{
    for( auto v : edges )
    {
        AddEdge(v, self);
    }
}


void Math::RemoveEdgesUnsorted(Range<Edge const *> edges, SparseAdjacencyMatrix& self)
{
    for(auto v : edges)
    {
        RemoveEdge(v, self);
    }
}


void Math::RemoveVertex(uint32_t vertex, SparseAdjacencyMatrix & self)
{
    auto vertices_count = GetNumberOfVertices(self);
    uint32_t offset = 0;
    for (auto v = 0u; v < vertices_count; ++v)
    {
        auto edge_begin = self.offsets[v];
        self.offsets[v] = edge_begin - offset;
        auto edge_end = self.offsets[v + 1];
        if(v != vertex)
        {
            for (auto e = edge_begin; e < edge_end; ++e)
            {
                auto current_vertex_index = self.indices[e];
                self.indices[e - offset] = current_vertex_index;
                offset += (current_vertex_index == vertex);
            }
        }
        else
        {
            offset += edge_end - edge_begin;
        }
    }
    self.offsets[vertices_count] -= offset;
    Shrink(self.indices, offset);
}


void Math::RemoveVertices(Range<uint32_t const*> vertices, SparseAdjacencyMatrix & self)
{
    for(auto v : vertices)
    {
        RemoveVertex(v, self);
    }
}


bool Math::Exists(Edge edge, SparseAdjacencyMatrix const & self)
{
    if((edge.source + 1) < Size(self.offsets))
    {
        auto start = self.offsets[edge.source];
        auto stop = self.offsets[edge.source + 1];
        auto found = std::lower_bound(begin(self.indices) + start, begin(self.indices) + stop, edge.destination);
        if(found != begin(self.indices) + stop)
        {
            return *found == edge.destination;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}
