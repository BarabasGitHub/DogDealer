#include "FaceReordering.h"

#include <Utilities\Memory.h>

#include <cassert>
#include <cstdint>
#include <limits>
#include <cmath>

#include <algorithm>
#include <array>
#include <numeric>
#include <vector>
#include <tuple>

// algorithm by Tom Forsyth
// http://home.comcast.net/~tom_forsyth/papers/fast_vert_cache_opt.html

namespace
{
    const uint8_t c_vertices_per_face = 3;
    const uint8_t c_max_vertex_cache_size = 32;

    const float c_invalid_score = 0; //-10.f * c_vertices_per_face;

    const float c_cache_decay_power = 1.5f;
    const float c_last_triangle_score = 0.75f;
    const float c_valence_boost_scale = 2.0f;
    const float c_valence_boost_power = 0.5f;

    float CalculateVertexScore( uint8_t const cache_position, uint8_t const active_triangle_count )
    {
        float score = 0.0f;

        if( active_triangle_count > 0 )
        {
            if( cache_position < c_vertices_per_face )
            {
                // This vertex was used in the last triangle,
                // so it has a fixed score, whichever of the three
                // it's in. Otherwise, you can get very different
                // answers depending on whether you add
                // the triangle 1,2,3 or 3,1,2 - which is silly.
                score = c_last_triangle_score;
            }
            else if( cache_position < c_max_vertex_cache_size )
            {
                // Points for being high in the cache.
                // range (0, 1]
                auto const scaler = 1.0f / ( c_max_vertex_cache_size - c_vertices_per_face );
                score = 1 - ( cache_position - c_vertices_per_face ) * scaler;
                score = std::pow( score, c_cache_decay_power );
            }
            //else // Vertex is not in FIFO cache - no score.


            // Bonus points for having a low number of tris still to
            // use the vert, so we get rid of lone verts quickly.
            // range (0, 1]
            auto const valance_boost = std::pow( float( active_triangle_count ), -c_valence_boost_power );
            score += c_valence_boost_scale * valance_boost;
        }
        else
        {
            // No tri needs this vertex!
            score = c_invalid_score;
        }
        // total range, not including the 'invalid score' (0, 1 + c_valence_boost_scale]
        return score;
    }

    std::array<std::array<float, c_max_vertex_cache_size + 1>, 32> PreCalculateScores( )
    {
        std::array<std::array<float, c_max_vertex_cache_size + 1>, 32> precalculated_scores;

        for( uint8_t v = 0u; v < 32u; ++v )
        {
            for( uint8_t c = 0u; c < precalculated_scores.front().size(); ++c )
            {
                precalculated_scores[v][c] = CalculateVertexScore(c, v);
            }
        }
        return precalculated_scores;
    }

    //std::array<float, c_max_vertex_cache_size + 1> PreCalculateCacheScores( )
    //{
    //    std::array<float, c_max_vertex_cache_size + 1> precalculated_scores;

    //    auto c = 0u;
    //    for( ; c < c_vertices_per_face; ++c )
    //    {
    //        precalculated_scores[c] = c_last_triangle_score;
    //    }
    //    for( ; c < precalculated_scores.size( ); ++c )
    //    {
    //        auto const scaler = 1.0f / ( c_max_vertex_cache_size - c_vertices_per_face );
    //        auto score = 1 - ( c - c_vertices_per_face ) * scaler;
    //        score = std::pow( score, c_cache_decay_power );
    //        precalculated_scores[c] = score;
    //    }
    //    precalculated_scores.back() = 0;
    //    return precalculated_scores;
    //}

    //std::array<float, 32> PreCalculateValenceScores( )
    //{
    //    std::array<float, 32> precalculated_scores;
    //    precalculated_scores[0] = c_invalid_score;
    //    for( auto v = 1u ; v < precalculated_scores.size(); ++v )
    //    {
    //        auto const valance_boost = std::pow( float( v ), -c_valence_boost_power );
    //        precalculated_scores[v] = c_valence_boost_scale * valance_boost;;
    //    }
    //    return precalculated_scores;
    //}



    const auto c_precalculated_scores = PreCalculateScores( );
    //const auto c_precalculated_cache_scores = PreCalculateCacheScores();
    //const auto c_precalculated_valence_scores = PreCalculateValenceScores();

    inline float GetScore( uint8_t cache_position, uint8_t active_triangle_count )
    {
        cache_position = std::min( c_max_vertex_cache_size, cache_position );
        active_triangle_count = std::min( active_triangle_count, uint8_t( 31 ) );
        return c_precalculated_scores[active_triangle_count][cache_position];
    }

    //inline float GetScore( uint8_t cache_position, uint8_t active_triangle_count )
    //{
    //    cache_position = std::min( c_max_vertex_cache_size, cache_position );
    //    active_triangle_count = std::min( active_triangle_count, uint8_t( 31 ) );
    //    return c_precalculated_cache_scores[cache_position] + c_precalculated_valence_scores[active_triangle_count];
    //}

    inline std::vector<unsigned> CalculateOffsets( std::vector<uint8_t> const & active_triangle_counts, unsigned const vertex_count )
    {
        std::vector<unsigned> offsets;
        offsets.reserve( vertex_count );
        // custom partial sum, because otherwise the uint8_t overflows
        unsigned previous_offset = 0;
        for( auto i = 0u; i < vertex_count; ++i )
        {
            offsets.emplace_back( previous_offset );
            previous_offset += active_triangle_counts[i];
        }
        return offsets;
    }

    inline std::vector<uint8_t> CountVertexDataUse( std::vector<uint32_t> const & indices, unsigned const vertex_count_plus_one )
    {
        std::vector<uint8_t> active_triangle_counts( vertex_count_plus_one, 0 );

        // First scan over the vertex data, count the total number of occurrences of each vertex
        for( auto index : indices )
        {
            assert( active_triangle_counts[index] < uint8_t( -1 ) );
            ++active_triangle_counts[index];
        }
        return active_triangle_counts;
    }

    // reuses the active_triangle_counts for some internal bookkeeping, should come out the same though
    inline std::vector<unsigned> CalculateTriangleIndices( std::vector<uint32_t> const & indices, std::vector<unsigned> const & offsets, unsigned const face_count, std::vector<uint8_t>& active_triangle_counts )
    {
        std::fill( active_triangle_counts.begin( ), active_triangle_counts.end( ), uint8_t(0) );

        std::vector<unsigned> triangle_indices( face_count * c_vertices_per_face );
        // Fill the vertex data structures with indices to the triangles
        // using each vertex
        for( auto i = 0u; i < face_count; ++i )
        {
            for( auto j = 0u; j < c_vertices_per_face; ++j )
            {
                auto const vertex_index = indices[c_vertices_per_face* i + j];
                auto const index = offsets[vertex_index] + active_triangle_counts[vertex_index];
                triangle_indices[index] = i;
                ++active_triangle_counts[vertex_index];
            }
        }
        return triangle_indices;
    }

    inline std::vector<float> CalculateInitialVertexScores( std::vector<uint8_t> const & active_triangle_counts, const unsigned vertex_count_plus_one )
    {
        std::vector<float> vertex_scores( vertex_count_plus_one );
        // calculate the initial scores based on the active triangle count
        // the max vertex cache size is used so the cache position won't contribute to the score
        for( auto i = 0u; i < vertex_count_plus_one; ++i )
        {
            vertex_scores[i] = GetScore( c_max_vertex_cache_size, active_triangle_counts[i] );
        }
        return vertex_scores;
    }


    inline std::vector<float> CalculateInitialFaceScores( std::vector<float> const & vertex_scores, std::vector<uint32_t> const & indices, unsigned face_count )
    {
        std::vector<float> face_scores( face_count );
        // calculate the face scores from the vertex scores
        for( auto i = 0u; i < face_count; ++i )
        {
            float score = 0;
            for( auto j = 0u; j < c_vertices_per_face; ++j )
            {
                score += vertex_scores[indices[i * c_vertices_per_face + j]];
            }
            face_scores[i] = score;
        }
        return face_scores;
    }

    inline unsigned FindBestStartingFace( std::vector<float> const & face_scores, unsigned const face_count )
    {
        // find the best starting face
        auto best_face_index = face_count;
        float best_face_score = c_invalid_score;
        for( auto i = 0u; i < face_count; ++i )
        {
            float score = face_scores[i];
            if( best_face_score < score )
            {
                best_face_score = score;
                best_face_index = i;
            }
        }
        return best_face_index;
    }

    template<typename Type>
    inline std::pair<Type, Type> CompareAndSwap( Type const a, Type const b )
    {
        //return std::minmax(a, b);
        // this is faster
        return b < a ? std::pair<Type, Type>{ b, a } : std::pair<Type, Type>{ a, b };
    }

    // sort in descending order assuming the last element is always 0
    inline void SortCachePositions( std::array<uint8_t, c_vertices_per_face + 1> & cache_positions )
    {
        static_assert( c_vertices_per_face == 3, "Sorting here assumes we use 3 vertices per face" );
        for( auto i = 1u; i < c_vertices_per_face; ++i )
        {
            for( auto j = 0u; j < c_vertices_per_face - i; ++j )
            {
                std::tie( cache_positions[j + 1], cache_positions[j] ) = CompareAndSwap( cache_positions[j + 1], cache_positions[j] );
            }
        }
    }
}


std::vector<uint32_t> ReorderFaces( std::vector<uint32_t> const & indices, unsigned const vertex_count )
{
    assert( indices.size( ) % c_vertices_per_face == 0 );
    const auto face_count = unsigned( indices.size( ) / c_vertices_per_face );
    const auto vertex_count_plus_one = vertex_count + 1;

    auto active_triangle_counts = CountVertexDataUse( indices, vertex_count_plus_one );

    auto const offsets = CalculateOffsets( active_triangle_counts, vertex_count );

    auto triangle_indices = CalculateTriangleIndices( indices, offsets, face_count, active_triangle_counts );

    auto vertex_scores = CalculateInitialVertexScores( active_triangle_counts, vertex_count_plus_one );

    auto face_scores = CalculateInitialFaceScores( vertex_scores, indices, face_count );

    // initalize cache positions
    std::array<unsigned, c_max_vertex_cache_size> cache;
    cache.fill( vertex_count );
    std::vector<uint8_t> cache_positions( vertex_count_plus_one, c_max_vertex_cache_size );

    // make our output indices vector with enough space allocated for all indices
    std::vector<uint32_t> output_indices;
    output_indices.reserve( indices.size( ) );

    auto best_face_index = FindBestStartingFace( face_scores, face_count );

    while( best_face_index < face_count )
    {
        // find the old cache positions, vertex indices and output the indices
        std::array<uint8_t, c_vertices_per_face + 1> old_cache_positions = { { 0 } };
        std::array<unsigned, c_vertices_per_face> new_vertices;
        for( uint8_t i = 0u; i < c_vertices_per_face; ++i )
        {
            auto const vertex_index = indices[best_face_index * c_vertices_per_face + i];
            output_indices.emplace_back( vertex_index );
            if( output_indices.size( ) == indices.size( ) )
            {
                return output_indices;
            }

            auto const cache_position = cache_positions[vertex_index];
            // limit the cache positions so we don't run outside the cache
            old_cache_positions[i] = (cache_position < c_max_vertex_cache_size ) ? cache_position : c_max_vertex_cache_size; // faster than using std::min ... =S
            new_vertices[i] = vertex_index;
        }

        SortCachePositions( old_cache_positions );

        // move elements back and update their cache position
        for( uint8_t amount = 0u; amount < c_vertices_per_face; ++amount )
        {
            auto const begin = old_cache_positions[amount + 1];
            auto const end = old_cache_positions[amount];
            auto destination = uint8_t(end + amount);
            for( auto cache_index = end; cache_index > begin; --destination )
            {
                --cache_index;
                auto const vertex_index = cache[cache_index];
                cache_positions[vertex_index] = destination;
                if( destination < c_max_vertex_cache_size )
                {
                    cache[destination] = vertex_index;
                }

                // update the cache positions of the other vertices and calculate their new scores
                auto const active_triangle_count = active_triangle_counts[vertex_index];
                auto const score = GetScore( destination, active_triangle_count );
                auto const score_difference = score - vertex_scores[vertex_index];
                vertex_scores[vertex_index] = score;

                if( active_triangle_count > 0 )
                {
                    auto offset = offsets[vertex_index];
                    // update the face scores
                    for( uint8_t j = 0u; j < active_triangle_count; ++j )
                    {
                        auto const index = triangle_indices[offset + j];
                        face_scores[index] += score_difference;
                    }
                }
            }
        }

        // insert the new elements at the start and update their cache positions
        for( uint8_t i = 0u; i < c_vertices_per_face; ++i )
        {
            auto const vertex_index = new_vertices[i];
            cache[i] = vertex_index;
            cache_positions[vertex_index] = i;
            // update the triangle indices and the active triangle count
            auto const active_triangle_count = active_triangle_counts[vertex_index];
            assert( active_triangle_count > 0 );
            auto const new_triangle_count = uint8_t(active_triangle_count - 1);
            active_triangle_counts[vertex_index] = new_triangle_count;

            // update the cache positions of the other vertices and calculate their new scores
            auto const score = GetScore( i, new_triangle_count );
            auto const score_difference = score - vertex_scores[vertex_index];
            vertex_scores[vertex_index] = score;

            auto vertex_offset = offsets[vertex_index];
            auto best_triangle_index = vertex_offset + new_triangle_count;
            // update the face scores
            for( auto j = 0u; j <= new_triangle_count; ++j )
            {
                auto const offset = vertex_offset + j;
                auto const index = triangle_indices[offset];
                face_scores[index] += score_difference;
                if( index == best_face_index )
                {
                    triangle_indices[offset] = triangle_indices[best_triangle_index];
                }
            }
        }

        face_scores[best_face_index] = c_invalid_score;

        float best_face_score = c_invalid_score;
        auto last_face_index = 0u;
        // look for a face in the neighbouring faces
        for( auto const vertex_index : cache )
        {
            for( auto i = 0u; i < active_triangle_counts[vertex_index]; ++i )
            {
                last_face_index = triangle_indices[offsets[vertex_index] + i];
                auto const score = face_scores[last_face_index];
                if( best_face_score < score )
                {
                    best_face_score = score;
                    best_face_index = last_face_index;
                }
            }
        }

        // if we found no good score in the cache neighbours find the best in complete list
        if( best_face_score == c_invalid_score )
        {
            best_face_index = FindBestStartingFace( face_scores, face_count );
        }
    }

    return output_indices;

}