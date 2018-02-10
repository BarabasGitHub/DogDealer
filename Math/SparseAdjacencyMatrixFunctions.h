#pragma once

#include <Utilities\Range.h>

namespace Math
{
    struct SparseAdjacencyMatrix;
    struct Edge;

    void AddEdge(Edge edge, SparseAdjacencyMatrix& self);
    void AddSymmetricEdges(Range<Edge const *> edges, SparseAdjacencyMatrix& self);
    void AddEdges(Range<Edge const *> edges, SparseAdjacencyMatrix& self);
    void AddEdgesUnsorted(Range<Edge const *> edges, SparseAdjacencyMatrix& self);

    void RemoveEdge(Edge edge, SparseAdjacencyMatrix& self);
    void RemoveSymmetricEdges(Range<Edge const *> edges, SparseAdjacencyMatrix& self);
    void RemoveEdges(Range<Edge const *> edges, SparseAdjacencyMatrix& self);
    void RemoveEdgesUnsorted(Range<Edge const *> edges, SparseAdjacencyMatrix& self);
    
    void RemoveVertex(uint32_t vertex, SparseAdjacencyMatrix& self);
    void RemoveVertices(Range<uint32_t const*> vertices, SparseAdjacencyMatrix& self);

    bool Exists(Edge edge, SparseAdjacencyMatrix const & self);
}
