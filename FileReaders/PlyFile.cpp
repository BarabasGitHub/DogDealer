
#include "PlyFile.h"
#include <array>
#include <cctype>
#include <fstream>

//#include <Graphics\Direct3D11.h>

using namespace std;
using namespace DirectX;
using namespace PackedVector;

using namespace FileReading;

namespace
{

    namespace keywords
    {
        const string end_header( "end_header" );
        const string comment( "comment" );
        const string element( "element" );
        const string property( "property" );
        const string list( "list" );
    }

    template<size_t N>
    array<PlyFile::Element::Property *,N> GetProperties( PlyFile::Element & element, array<string,N> const & property_names )
    {
        auto properties = array<PlyFile::Element::Property *,N>();
        for( auto& element_property : element.m_properties )
        {
            for( auto n = 0u; n < N; ++n )
            {
                if( element_property.m_name == property_names[n] )
                {
                    properties[n] = &element_property;
                }
            }
        }
        return properties;
    }


}

PlyFile::PlyFile( wstring filename )
{
    Open( move(filename) ); 
    assert(IsPly());
    ReadHeader();
}


void PlyFile::Open( wstring filename )
{
    if( IsOpen() ) Close();

    // read file in binary format, otherwise the line-endings aren't read correctly somehow.
    ifstream file( move(filename), ios::binary | ios::in );
    assert( file.good() );
    m_file_stream << file.rdbuf();
}

void PlyFile::Close()
{
    m_file_stream.clear();
}

bool PlyFile::IsOpen() const
{
    return !m_file_stream.str().empty();
}

bool PlyFile::IsPly()
{
    m_file_stream.seekg( 0, ios::beg );
    string file_type;
    m_file_stream >> file_type;

    if( file_type != "ply" )
        return false;

    string file_format;
    m_file_stream >> file_format;
    if( file_format != "format" )
        return false;

    m_file_stream >> file_format;
    if( file_format == "ascii" )
    {
        m_format = PlyFormat::ASCII;
    }
    else if ( file_format == "binary_little_endian" )
    {
        m_format = PlyFormat::BINARY_LITTLE_ENDIAN;
    }
    else if ( file_format == "binary_big_endian" )
    {
        m_format = PlyFormat::BINARY_BIG_ENDIAN;
    }
    else
    {
        return false;
    }

    float file_version;
    m_file_stream >> file_version;
    if( file_version == 1.0f )
    {
        // 1.0 is the only version available at the moment.
    }
    else
    {
        return false;
    }

    m_file_stream.ignore( numeric_limits<size_t>::max(), '\n' );
    m_header_start = m_file_stream.tellg();

    return true;
}

void PlyFile::ReadHeader()
{
    m_file_stream.seekg( m_header_start );

    string line;
    unsigned element_offset = 0;
    unsigned property_offset = 0;
    while( getline( m_file_stream, line ) && line.compare( 0, keywords::end_header.size(), keywords::end_header ) != 0 )
    {
        if( line.compare( 0, keywords::comment.size(), keywords::comment ) == 0 )
        {
            m_comments.emplace_back( line.substr( keywords::comment.size()+1 ) );
        }
        else if( line.compare( 0, keywords::element.size(), keywords::element ) == 0 )
        {
            stringstream line_stream( line.substr( keywords::element.size() ) );
            string text_buffer;
            line_stream >> text_buffer;
            unsigned count;
            line_stream >> count;
            m_elements.emplace_back( text_buffer, count, element_offset );

            property_offset = 0;
            if( m_format == PlyFormat::ASCII )
            {
                element_offset += count;
            }
        }
        else if( line.compare( 0, keywords::property.size(), keywords::property ) == 0 )
        {
            stringstream line_stream( line.substr( keywords::property.size() ) );
            PlyType length_type;
            PlyType value_type;

            string text_buffer;
            line_stream >> text_buffer;
            if( text_buffer == keywords::list )
            {
                line_stream >> text_buffer;
                length_type = ConvertToPlyType( text_buffer );
                line_stream >> text_buffer;
                value_type = ConvertToPlyType( text_buffer );
            }
            else // proprty is scalar
            {
                length_type = PlyType::SCALAR;
                value_type = ConvertToPlyType( text_buffer );
            }

            line_stream >> text_buffer;
            m_elements.back().m_properties.emplace_back( text_buffer , length_type, value_type, property_offset );

            if( m_format == PlyFormat::ASCII )
            {
                if( length_type == PlyType::SCALAR ) ++property_offset;
            }
            else // assuming binary format
            {
                auto previous_offset = property_offset;

                if( length_type == PlyType::SCALAR  ) property_offset += GetSize( value_type );

                element_offset += m_elements.back().m_count * ( property_offset - previous_offset ); // is this correct? The property_offset should be the total size of all properties together in bytes.
            }
            m_elements.back().m_size = property_offset;
        }
    }

    m_file_stream.ignore( numeric_limits<size_t>::max(), '\n' );
    m_data_start = m_file_stream.tellg();
}

PlyFile::PlyType PlyFile::ConvertToPlyType( string input )
{
    //    name        type        number of bytes
    //---------------------------------------
    //char       character                 1
    //uchar      unsigned character        1
    //short      short integer             2
    //ushort     unsigned short integer    2
    //int        integer                   4
    //uint       unsigned integer          4
    //float      single-precision float    4
    //double     double-precision float    8

    if ( input == "char" )
    {
        return PlyType::CHAR;
    }
    else if ( input == "uchar" )
    {
        return PlyType::UCHAR;
    }
    else if ( input == "short" )
    {
        return PlyType::SHORT;
    }
    else if ( input == "ushort" )
    {
        return PlyType::USHORT;
    }
    else if ( input == "int" )
    {
        return PlyType::INT;
    }
    else if ( input == "uint" )
    {
        return PlyType::UINT;
    }
    else if ( input == "float" )
    {
        return PlyType::FLOAT;
    }
    else if ( input == "double" )
    {
        return PlyType::DOUBLE;
    }
    else
    {
        return PlyType::UNKOWN;
    }
}

unsigned PlyFile::GetSize( PlyType type )
{
    //    name        type        number of bytes
    //---------------------------------------
    //char       character                 1
    //uchar      unsigned character        1
    //short      short integer             2
    //ushort     unsigned short integer    2
    //int        integer                   4
    //uint       unsigned integer          4
    //float      single-precision float    4
    //double     double-precision float    8
    switch ( type )
    {
    case PlyType::CHAR:
    case PlyType::UCHAR:
        {
            return 1;
        }
    case PlyType::SHORT:
    case PlyType::USHORT:
        {
            return 2;
        }
    case PlyType::INT:
    case PlyType::UINT:
    case PlyType::FLOAT:
        {
            return 4;
        }
    case PlyType::DOUBLE:
        {
            return 8;
        }
    default:
        {
            return 0;
        }
    }

}

PlyFile::Element* PlyFile::GetElement( const string & element_name )
{
    for( auto& element : m_elements )
    {
        if( element.m_name == element_name )
        {
           return &element;
        }
    }
    return nullptr;
}

void PlyFile::GoToElement( const PlyFile::Element& element )
{
    m_file_stream.seekg( m_data_start, ios_base::beg );

    if( m_format == PlyFormat::ASCII )
    {
        for ( unsigned i = 0; i < element.m_offset; ++i )
        {
            m_file_stream.ignore( 1024, '\n' ); //ignore line
        }
    }
    else // binary
    {
        m_file_stream.seekg( element.m_offset, ios_base::cur );
    }
}

PositionsContainer PlyFile::GetVertexPositions()
{
    auto vertex_element = GetElement( "vertex" );

    if( vertex_element == nullptr ||
        vertex_element->m_properties.empty() ||
        vertex_element->m_properties[0].m_length_type != PlyType::SCALAR ) // check for the element having a list (assuming it will only contain a list if it does.
    {
        return PositionsContainer();
    }

    static const unsigned char n_properties = 3;
    const array<string,n_properties> property_names = {"x","y","z"};

    auto vertex_properties = GetProperties<n_properties>( *vertex_element, property_names );

    if( count(vertex_properties.begin(), vertex_properties.end(), nullptr ) != 0)
    {
        return PositionsContainer();
    }

    PositionsContainer vertex_positions( vertex_element->m_count );

    array<float*,n_properties> store_positions = { &vertex_positions.begin()->x, &vertex_positions.begin()->y, &vertex_positions.begin()->z };
    array<int,n_properties> strides  = { 3, 3, 3 };

    StoreProperties<float,n_properties>( *vertex_element, vertex_properties, store_positions, strides );

    return vertex_positions;
}

NormalsContainer PlyFile::GetVertexNormals()
{
    auto vertex_element = GetElement( "vertex" );

    if( vertex_element == nullptr ||
        vertex_element->m_properties.empty() ||
        vertex_element->m_properties[0].m_length_type != PlyType::SCALAR ) // check for the element having a list (assuming it will only contain a list if it does.
    {
        return NormalsContainer();
    }

    static const unsigned char n_properties = 3;
    const array<string,n_properties> property_names = {"nx","ny","nz"};

    auto vertex_properties = GetProperties<n_properties>( *vertex_element, property_names );

    if( count(vertex_properties.begin(), vertex_properties.end(), nullptr ) != 0)
    {
        return NormalsContainer();
    }

    NormalsContainer vertex_normals( vertex_element->m_count );

    array<float*,n_properties> store_positions = { &vertex_normals.begin()->x, &vertex_normals.begin()->y, &vertex_normals.begin()->z };
    array<int,n_properties> strides  = { 3, 3, 3 };

    StoreProperties<float,n_properties>( *vertex_element, vertex_properties, store_positions, strides );

    return vertex_normals;
}

UVsContainer PlyFile::GetVertexUV()
{
    auto vertex_element = GetElement( "vertex" );

    if( vertex_element == nullptr ||
        vertex_element->m_properties.empty() ||
        vertex_element->m_properties[0].m_length_type != PlyType::SCALAR ) // check for the element having a list (assuming it will only contain a list if it does.
    {
        return UVsContainer();
    }

    static const unsigned char n_properties = 2;
    const array<string,n_properties> property_names = {"s","t"};

    auto vertex_properties = GetProperties<n_properties>( *vertex_element, property_names );

    if( count(vertex_properties.begin(), vertex_properties.end(), nullptr ) != 0)
    {
        return UVsContainer();
    }

    UVsContainer vertex_UV( vertex_element->m_count );

    array<float*,n_properties> store_positions = { &vertex_UV.begin()->x, &vertex_UV.begin()->y };
    array<int,n_properties> strides  = { 2, 2 };

    StoreProperties<float,n_properties>( *vertex_element, vertex_properties, store_positions, strides );

    return vertex_UV;
}

ColorsContainer PlyFile::GetVertexColors()
{
    auto vertex_element = GetElement( "vertex" );

    if( vertex_element == nullptr ||
        vertex_element->m_properties.empty() ||
        vertex_element->m_properties[0].m_length_type != PlyType::SCALAR ) // check for the element having a list (assuming it will only contain a list if it does.
    {
        return ColorsContainer();
    }

    static const unsigned char n_properties = 4;
    const array<string,n_properties> property_names = {"r","g", "b", "a"};

    auto vertex_properties = GetProperties<n_properties>( *vertex_element, property_names );

    if( count(vertex_properties.begin(), vertex_properties.end(), nullptr ) != 0 )
    {
        return ColorsContainer();
    }

    for( auto property : vertex_properties )
    {
        if (property == nullptr) return ColorsContainer(vertex_element->m_count,1);
    }


    vector<Math::Float4> vertex_colors( vertex_element->m_count, Math::Float4(0,0,0,1) );

    array<float*,n_properties> store_positions = { &vertex_colors.begin()->x, &vertex_colors.begin()->y, &vertex_colors.begin()->z, &vertex_colors.begin()->w };
    array<int,n_properties> strides  = { 4, 4, 4, 4 };

    StoreProperties<float,n_properties>( *vertex_element, vertex_properties, store_positions, strides );

    return vertex_colors;

    //switch ( vertex_properties[0]->m_value_type ) // assuming they all have the same type
    //{
    //case PlyType::UCHAR:
    //    {
    //        ColorsContainer vertex_colors( vertex_element->m_count );

    //        array<uint8_t*,n_properties> store_positions = { &vertex_colors.begin()->r, &vertex_colors.begin()->g, &vertex_colors.begin()->b, &vertex_colors.begin()->a };
    //        array<int,n_properties> strides  = { 4, 4, 4, 4 };

    //        StoreProperties<uint8_t,n_properties>( *vertex_element, vertex_properties, store_positions, strides );

    //        return vertex_colors;
    //    }
    //case PlyType::FLOAT:
    //    {
    //        vector<XMFLOAT4> vertex_colors_temp( vertex_element->m_count );

    //        array<float*,n_properties> store_positions = { &vertex_colors_temp.begin()->x, &vertex_colors_temp.begin()->y, &vertex_colors_temp.begin()->z, &vertex_colors_temp.begin()->w };
    //        array<int,n_properties> strides  = { 4, 4, 4, 4 };

    //        StoreProperties<float,n_properties>( *vertex_element, vertex_properties, store_positions, strides );
    //
    //        vector<XMCOLOR> vertex_colors;
    //        vertex_colors.reserve( vertex_element->m_count );
    //        for( auto& float_color : vertex_colors_temp )
    //        {
    //            vertex_colors.push_back( XMCOLOR( float_color.x, float_color.y, float_color.z, float_color.w ) );
    //        }
    //        return vertex_colors;
    //    }
    //default:
    //    return vector<XMCOLOR>();
    //}
}

IndicesContainer PlyFile::GetIndices()
{
   auto vertex_element = GetElement( "face" );

    if( vertex_element == nullptr ||
        vertex_element->m_properties.empty() ||
        vertex_element->m_properties[0].m_length_type == PlyType::SCALAR ) // check if the element has a list
    {
        return IndicesContainer();
    }

    const array<string,1> property_name = { "vertex_indices" };

    auto vertex_property = GetProperties<1>( *vertex_element, property_name )[0];

    if( vertex_property == nullptr )
    {
        return IndicesContainer();
    }

    // assuming all goes well...
    return ReturnList<unsigned>( *vertex_element, *vertex_property );
}

D3D_PRIMITIVE_TOPOLOGY PlyFile::GetPrimitiveTopology()
{
   auto vertex_element = GetElement( "face" );

    if( vertex_element == nullptr ||
        vertex_element->m_properties.empty() ||
        vertex_element->m_properties[0].m_length_type == PlyType::SCALAR ) // check if the element has a list
    {
        return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    }

    const array<string,1> property_name = { "vertex_indices" };

    auto vertex_property = GetProperties<1>( *vertex_element, property_name )[0];

    if( vertex_property == nullptr )
    {
        return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    }

    auto number = GetMaxNumberOfListElements( *vertex_element, *vertex_property );

    switch (number)
    {
    case 2:
        return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    case 3:
        return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    default:
        return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    }

}