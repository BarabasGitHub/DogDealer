#include "TangentDirections.h"

#include <Math\FloatOperators.h>
#include <Math\MathFunctions.h>
#include <Utilities\StdVectorFunctions.h>

#include <array>
#include <cstdint>

using namespace std;
using namespace Math;

void CalculateTangents( const vector<Float3> & vertex_positions, const vector<Float3> & vertex_normals, const vector<Float2> & vertex_texture_coordinates, vector<uint32_t> & vertex_indices, vector<TangentDirections> & vertex_tangents, vector<uint32_t> & duplicate_indices /*indices to the original elements that need to be duplicated and appended to the original vectors*/ )
{
    assert( vertex_positions.size( ) != 0 );
    assert( vertex_positions.size( ) == vertex_normals.size( ) );
    assert( vertex_positions.size( ) == vertex_texture_coordinates.size( ) );
    assert( vertex_indices.size( ) != 0 );
    const unsigned indices_per_face = 3;

    ResetSize(vertex_tangents, vertex_positions.size() );

    for( auto indices = vertex_indices.begin( ); indices != vertex_indices.end( ); indices += indices_per_face )
    {
        array<Float3, 3> positions;
        array<Float2, 3> texture_coordinates;

        for( auto i = 0u; i < indices_per_face; ++i )
        {
            auto const index = indices[i];
            positions[i] = vertex_positions[index];
            texture_coordinates[i] = vertex_texture_coordinates[index];
        }

        auto edge01 = positions[1] - positions[0];
        auto edge02 = positions[2] - positions[0];

        // calculate tangent vectors ---------------------------
        auto texture01 = texture_coordinates[1] - texture_coordinates[0];
        auto texture02 = texture_coordinates[2] - texture_coordinates[0];


        Float3 U = 0;
        Float3 V = 0;

        auto div = Cross( texture01, texture02 );
        auto area = Area( texture01, texture02 );

        if( area > 0 )
        {

            U  = edge02 *  texture01.y;
            U -= edge01 *  texture02.y;
            V  = edge02 * -texture01.x;
            V += edge01 *  texture02.x;

            U /= div;
            V /= div;

        }
        U = ConditionalNormalize( U );
        V = ConditionalNormalize( V );

        //weight by triangle texture coverage       
        U *= area;
        V *= area;

        auto N = ConditionalNormalize( Cross( edge01, edge02 ) );
        auto handedness = signbit( Dot( N, Cross( U, V ) ) ) * -2 + 1;

        // DO NOT modify U or V in this loop!
        for( auto i = 0u; i < indices_per_face; ++i )
        {
            auto vertex_index = indices[i];
            auto tangent_directions = begin(vertex_tangents) + vertex_index;

            auto existing_handedness = tangent_directions->handedness;
            // Add a new vertex in case the UV are mirrored.
            if( existing_handedness != 0 && existing_handedness != handedness )
            {
                auto duplicate_index = std::find( begin( duplicate_indices ), end( duplicate_indices ), vertex_index );
                if( duplicate_index == end( duplicate_indices ) )
                {
                    auto new_vertex_index = uint32_t( Size( vertex_tangents ) );
                    indices[i] = new_vertex_index;
                    vertex_tangents.emplace_back();
                    tangent_directions = end( vertex_tangents ) - 1;
                    duplicate_indices.emplace_back( vertex_index );
                }
                else
                {
                    tangent_directions = end( vertex_tangents ) - (end(duplicate_indices) - duplicate_index);
                    assert( tangent_directions->handedness == handedness );
                }
            }

            // weight by angle to fix the L-Shape problem
            auto weight = GetAngle(RotationBetweenAxes( positions[( i + 2 ) % 3] - positions[i], positions[( i + 1 ) % 3] - positions[i] ));
            tangent_directions->tangent += U * weight;
            tangent_directions->bitangent += V * weight;
            tangent_directions->handedness = handedness;
        }
    }

    for( auto i = 0u; i < vertex_tangents.size( ); ++i )
    {
        auto& tangent_directions = vertex_tangents[i];
        auto normal_index = i;
        // point the normal index to the original data in case of an newly added vertex due to mirrored uv stuff.
        if( normal_index >= vertex_normals.size() )
        {
            normal_index = duplicate_indices[normal_index - vertex_normals.size()];
        }
        auto N = vertex_normals[normal_index];

        // project U and V to the N plane and store normalized
        auto U = tangent_directions.tangent;
        U = ComponentPerpendicularToNormal( U, N );
        U = ConditionalNormalize( U );
        tangent_directions.tangent = U;

        auto V = tangent_directions.bitangent;
        V = ComponentPerpendicularToNormal( V, N );
        V = ConditionalNormalize( V );
        tangent_directions.bitangent = V;
    }
}