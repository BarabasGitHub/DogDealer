#include "VertexReordering.h"

std::vector<uint32_t> CalculateNewIndices( std::vector<uint32_t> const & indices, uint32_t const vertex_count )
{
    if( indices.empty() ) return indices;
    std::vector<uint32_t> new_indices( indices.size() );
    std::vector<uint32_t> index_mapping( vertex_count, uint32_t(-1) );
    for( uint32_t i = 0u, index = 0u; i < indices.size(); ++i)
    {
        auto const original_index = indices[i];
        auto const mapped_index = index_mapping[original_index];
        // check if the index is already used before
        if( mapped_index == uint32_t(-1) )
        {
            index_mapping[original_index] = index;
            new_indices[i] = index;
            ++index;
        }
        else
        {
            new_indices[i] = mapped_index;
        }
    }

    return new_indices;
}

