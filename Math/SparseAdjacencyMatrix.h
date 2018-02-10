#pragma once

#include <vector>

namespace Math
{
    struct SparseAdjacencyMatrix;

    void swap( SparseAdjacencyMatrix & a, SparseAdjacencyMatrix & b );
    SparseAdjacencyMatrix MakeSize( SparseAdjacencyMatrix m, uint32_t vertices, uint32_t edges );
    SparseAdjacencyMatrix CreateSparseAdjacencyMatrix( uint32_t vertices, uint32_t edges );
    uint32_t GetNumberOfVertices( SparseAdjacencyMatrix const & m );
    uint32_t GetNumberOfEdges( SparseAdjacencyMatrix const & m );

    struct SparseAdjacencyMatrix
    {
        std::vector<uint32_t> indices;
        // offsets into the indices, start and end
        std::vector<uint32_t> offsets = { { { 0 } } };
    };


    inline void swap( SparseAdjacencyMatrix & a, SparseAdjacencyMatrix & b )
    {
        using std::swap;
        swap( a.indices, b.indices );
        swap( a.offsets, b.offsets );
    }

    inline SparseAdjacencyMatrix MakeSize( SparseAdjacencyMatrix m, uint32_t vertices, uint32_t edges)
    {
        m.indices.clear();
        m.indices.resize( edges );
        m.offsets.clear();
        m.offsets.resize( vertices );
        return m;
    }


    inline SparseAdjacencyMatrix CreateSparseAdjacencyMatrix( uint32_t vertices, uint32_t edges )
    {
        return MakeSize( {}, vertices, edges );
    }


    inline uint32_t GetNumberOfVertices( SparseAdjacencyMatrix const & m )
    {
        return uint32_t(m.offsets.size() - 1);
    }


    inline uint32_t GetNumberOfEdges( SparseAdjacencyMatrix const & m )
    {
        return m.offsets.back();
    }
}
