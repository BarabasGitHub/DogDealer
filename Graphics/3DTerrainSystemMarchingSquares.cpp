#include "3DTerrainSystemMarchingSquares.h"

#include "3DTerrainSystemStructs.h"

#include <Math\FloatOperators.h>
#include <Math\MathFunctions.h>
#include <Math\Conversions.h>

#include <Utilities\IntegerRange.h>

namespace Graphics
{

    Math::Float2 MakeVertex( Math::Float2 a, Math::Float2 b, float value_a, float value_b, float iso_value )
    {
        auto factor = ( iso_value - value_a ) / ( value_b - value_a );

        return Lerp( a, b, factor );
    }

    
    std::pair<Math::Float2, Math::Float3> MakeVertex( Math::Float2 a, Math::Float2 b, DensityAndGradient value_a, DensityAndGradient value_b, float iso_value )
    {
        auto factor = ( iso_value - value_a.density ) / ( value_b.density - value_a.density );

        return{ Lerp( a, b, factor ), Nlerp( value_a.gradient, value_b.gradient, factor ) };
    }

    // using the convention:
    // 1--2 
    // :  :
    // 0--3
    // INPUT:
    // offset is the position of corner 0
    // scale is the size of the square sides
    // values are the values of corner 0, 1, 2, 3
    // OUPTUT:
    // vertices, the vertices form pairs which are the begin and end of line segments
    void CreateVerticesForCell( Math::Float2 const offset, std::array<Math::Unsigned2, 4> const & corner_coordinates, Math::Float2 const scale, std::array<float, 4> const values, float const iso_value, std::vector<Math::Float2> & vertices )
    {
        std::array<bool, 4> solid;
        for( auto i = 0u; i < 4; i++ )
        {
            solid[i] = values[i] > iso_value;
        }

        auto previous_vertex_count = vertices.size();

        // edge 01 and edge 12
        for( auto i = 0u; i < 2; i++ )
        {
            if( solid[i] != solid[i + 1] )
            {
                auto vertex = MakeVertex( Float2FromUnsigned2( corner_coordinates[i] ), Float2FromUnsigned2( corner_coordinates[i + 1] ), values[i], values[i + 1], iso_value );
                vertex *= scale;
                vertex += offset;
                vertices.emplace_back( vertex );
            }
        }
        // edge 23
        if( solid[2] != solid[3] )
        {
            // invert the order for consistency
            auto vertex = MakeVertex( Float2FromUnsigned2( corner_coordinates[3] ), Float2FromUnsigned2( corner_coordinates[2] ), values[3], values[2], iso_value );
            vertex *= scale;
            vertex += offset;
            vertices.emplace_back( vertex );
        }
        // edge 30
        if( solid[3] != solid[0] )
        {
            // invert the order for consistency
            auto vertex = MakeVertex( Float2FromUnsigned2( corner_coordinates[0] ) , Float2FromUnsigned2( corner_coordinates[3] ), values[0], values[3], iso_value );
            vertex *= scale;
            vertex += offset;
            vertices.emplace_back( vertex );
        }

        auto added_vertex_count = vertices.size() - previous_vertex_count;
        assert( added_vertex_count == 0 || added_vertex_count == 2 || added_vertex_count == 4 );
        if( added_vertex_count == 4 )
        {
            assert( solid[0] == solid[2] && solid[1] == solid[3] );
            // we have a sadle point, calculate the average value
            auto average = values[0] + values[1] + values[2] + values[3];
            average *= 0.25f;
            // by default we construct these lines / /
            // if the solidness of the center is the same as the upper left corner we need
            // the lines need to be \ \ instead of / / 
            // thus we swap the left and right vertex
            if( (average > iso_value) == solid[1] )
            {
                std::swap( vertices[previous_vertex_count], vertices[previous_vertex_count + 2] );
            }
        }
    }

    void CreateVerticesForCell( std::array<Math::Unsigned2, 4> const & corner_coordinates, std::array<DensityAndGradient, 4> const values, float const iso_value, std::vector<Math::Float2> & vertices, std::vector<Math::Float3> & normals )
    {
        std::array<bool, 4> solid;
        for( auto i = 0u; i < 4; i++ )
        {
            solid[i] = values[i].density > iso_value;
        }

        auto previous_vertex_count = vertices.size();

        // edge 01 and edge 12
        for( auto i = 0u; i < 2; i++ )
        {
            if( solid[i] != solid[i + 1] )
            {
                auto vertex = MakeVertex( Float2FromUnsigned2( corner_coordinates[i] ), Float2FromUnsigned2( corner_coordinates[i + 1] ), values[i], values[i + 1], iso_value );
                vertices.emplace_back( vertex.first );
                normals.emplace_back( vertex.second );
            }
        }
        // edge 23
        if( solid[2] != solid[3] )
        {
            // invert the order for consistency
            auto vertex = MakeVertex( Float2FromUnsigned2( corner_coordinates[3] ), Float2FromUnsigned2( corner_coordinates[2] ), values[3], values[2], iso_value );
            vertices.emplace_back( vertex.first );
            normals.emplace_back( vertex.second );
        }
        // edge 30
        if( solid[3] != solid[0] )
        {
            // invert the order for consistency
            auto vertex = MakeVertex( Float2FromUnsigned2( corner_coordinates[0] ), Float2FromUnsigned2( corner_coordinates[3] ), values[0], values[3], iso_value );
            vertices.emplace_back( vertex.first );
            normals.emplace_back( vertex.second );
        }

        auto added_vertex_count = vertices.size() - previous_vertex_count;
        assert( added_vertex_count == 0 || added_vertex_count == 2 || added_vertex_count == 4 );
        if( added_vertex_count == 4 )
        {
            assert( solid[0] == solid[2] && solid[1] == solid[3] );
            // we have a sadle point, calculate the average value
            auto average = values[0].density + values[1].density + values[2].density + values[3].density;
            average *= 0.25f;
            // by default we construct these lines / /
            // if the solidness of the center is the same as the upper left corner we need
            // the lines need to be \ \ instead of / / 
            // thus we swap the left and right vertex
            if( ( average > iso_value ) == solid[1] )
            {
                std::swap( vertices[previous_vertex_count], vertices[previous_vertex_count + 2] );
                std::swap( normals[previous_vertex_count], normals[previous_vertex_count + 2] );
            }
        }
    }

    
    void CreateVerticesForTile( Math::Float2 const offset, Math::Float2 const cell_scale, Math::Unsigned2 cell_count, std::vector<float> const & values, float const iso_value, std::vector<Math::Float2> &  vertices )
    {
        assert( values.size() == (cell_count.x + 1) * (cell_count.y + 1));
        
		auto row_cells = cell_count.x + 1;

		for( uint32_t y = 0; y < cell_count.y; y++ )
        {
            for( uint32_t x = 0; x < cell_count.x; x++ )
            {
                std::array<float, 4> cell_values;
                std::array<Math::Unsigned2, 4> corner_coordinates;
                corner_coordinates[0] = { x, y };
                corner_coordinates[1] = { x, y + 1 };
                corner_coordinates[2] = { x + 1, y + 1 };
                corner_coordinates[3] = { x + 1, y };
                for( auto i = 0u; i < 4; i++ )
                {
                    cell_values[i] = values[corner_coordinates[i].x + corner_coordinates[i].y * row_cells];
                }
                
                CreateVerticesForCell( offset, corner_coordinates,  cell_scale, cell_values, iso_value, vertices );
            }
        }
    }



    void CreateVerticesForTile( Math::Unsigned2 cell_count, std::vector<DensityAndGradient> const & values, float const iso_value, std::vector<Math::Float2> &  vertices, std::vector<Math::Float3> & normals )
    {
        assert( values.size() == ( cell_count.x + 1 ) * ( cell_count.y + 1 ) );

        auto row_cells = cell_count.x + 1;

        for( uint32_t y = 0; y < cell_count.y; y++ )
        {
            for( uint32_t x = 0; x < cell_count.x; x++ )
            {
                std::array<DensityAndGradient, 4> cell_values;
                std::array<Math::Unsigned2, 4> corner_coordinates;
                corner_coordinates[0] = { x, y };
                corner_coordinates[1] = { x, y + 1 };
                corner_coordinates[2] = { x + 1, y + 1 };
                corner_coordinates[3] = { x + 1, y };
                for( auto i = 0u; i < 4; i++ )
                {
                    cell_values[i] = values[corner_coordinates[i].x + corner_coordinates[i].y * row_cells];
                }

                CreateVerticesForCell( corner_coordinates, cell_values, iso_value, vertices, normals );
            }
        }
    }
    
    namespace
    {


        struct Comparator
        {
            Comparator( Math::Float2 value, std::vector<Math::Float2>::const_iterator vertices )
                : value( value ), vertices( vertices )
            {
            }

            Math::Float2 value;
            std::vector<Math::Float2>::const_iterator vertices;

            bool operator()( uint32_t i ) const
            {
                return vertices[i] == value;
            }
        };


        template<int Increment>
        std::vector<uint32_t>::iterator FindConnectedEdges( std::vector<uint32_t>& indices, std::vector<uint32_t>::iterator destination, std::vector<Math::Float2> const & input_vertices )
        {
            // find the edge that ends at the start of the first edge
            auto found = std::find_if( begin( indices ), end( indices ), Comparator( input_vertices[*destination], begin( input_vertices ) ) );
            while( found != end( indices ) )
            {
                // move the destination one elment further
                destination += Increment;
                // if it's a start vertex
                if( *found % 2 == 0 )
                {
                    *destination = found[1];
                    indices.erase( found, found + 2 );
                }
                // otherwise it is an end vertex
                else
                {
                    *destination = found[-1];
                    indices.erase( found - 1, found + 1 );
                }

                // find the edge that ends at the first vertex of the last found edge
                found = std::find_if( begin( indices ), end( indices ), Comparator( input_vertices[*destination], begin( input_vertices ) ) );
            }

            return destination;
        }
    }


    // input is vertices in pairs, as edges
    // output is vertices, no longer in pairs, but each vertex is an end & begin of an edge together with the vertex before/after it
    void SortVertices( std::vector<Math::Float2> const & input_vertices, std::vector<uint32_t> & output_indices )
    {
        assert( input_vertices.size() % 2 == 0 && input_vertices.size() >= 2 );
        auto input_vertex_count = input_vertices.size();
        auto edge_count = input_vertex_count / 2;
        // we add one later in the loop, to account for the startin index
        auto output_vertex_count = edge_count;
       
        
        std::vector<uint32_t> indices;
        {
            auto range = CreateIntegerRange( 0u, uint32_t( input_vertex_count ) );
            indices.assign( begin( range ), end( range ) );
        }

        auto filled_vertex_count = 0u;

        while( !indices.empty() )
        {
            assert( indices.size() >= 2 );
            assert( indices.size() % 2 == 0 );

            // account for the starting index we have to add extra
            ++output_vertex_count;

            output_indices.resize( output_vertex_count );

            auto destination = begin( output_indices );
            // store the end of the unfilled items
            auto unfilled_end = end( output_indices ) - filled_vertex_count;

            // moves all already filled entries to the back of the vector
            std::move_backward( unfilled_end - 2, end( output_indices ) - 2, end(output_indices) );

            // add the index of the new starting edge
            // we pretend this is the end of a previous edge
            *destination = indices[0];

            // find the connected edges in the fowwards direction
            destination = FindConnectedEdges<1>( indices, destination, input_vertices );

            // make the destination point past the last added element
            ++destination;

            // calculate the number of elements that we added
            auto added_elements_count = destination - begin( output_indices );

            // check if we have processed all vertices yet
            if( indices.empty() )
            {
                filled_vertex_count += uint32_t(added_elements_count);
                break;
            }

            // move all data to the end of the vector
            std::move_backward( begin( output_indices ), destination, unfilled_end );          

            // set the destination to point to the first valid element
            destination = unfilled_end - added_elements_count;

            // find the connected edges in the 'backwards' direction
            destination = FindConnectedEdges<-1>( indices, destination, input_vertices );
            
            // calculate the total number of indices (vertices), we filled in already
            filled_vertex_count = uint32_t(end(output_indices) - destination);
        }

        assert( filled_vertex_count == output_vertex_count );
    }
}