#pragma once

#include <string>
#include <vector>
#include <array>
#include <sstream>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "FileStructs.h"

//forward declarations


class PlyFile
{
public:

    enum class PlyFormat: uint8_t { ASCII, BINARY_LITTLE_ENDIAN, BINARY_BIG_ENDIAN };
    enum class PlyType  : uint8_t { CHAR, UCHAR, SHORT, USHORT, INT, UINT, FLOAT, DOUBLE, SCALAR, UNKOWN };

    struct Element
    {
    public:
        struct Property
        {
            std::string  m_name;
            PlyType      m_length_type;
            PlyType      m_value_type;
            unsigned     m_offset;
        public:
            Property():
                m_name(),
                m_length_type( PlyType::UNKOWN ),
                m_value_type( PlyType::UNKOWN ),
                m_offset( 0 )
            {}
            Property( std::string name, PlyType length_type, PlyType value_type, unsigned offset ):
                m_name( name ),
                m_length_type( length_type ),
                m_value_type( value_type ),
                m_offset( offset )
            {}
        };

        std::string m_name;
        unsigned m_count;
        unsigned m_size;
        unsigned m_offset;
        std::vector<Property> m_properties;

    public:

        Element( std::string name, unsigned count, unsigned offset ):
            m_name( name ),
            m_count( count ),
            m_size( ),
            m_offset( offset ),
            m_properties( )
        {}
    };

    friend Element;

    // variables
private:
    std::stringstream           m_file_stream;
    PlyFormat                   m_format;
    std::streamoff m_header_start;
    std::streamoff m_data_start;
    std::vector<Element>        m_elements;
    std::vector<std::string>    m_comments;
    // functions
public:

    PlyFile( std::wstring filename );
    void Open( std::wstring filename ) ;
    void Close();
    bool IsOpen() const;
    FileReading::PositionsContainer GetVertexPositions();
    FileReading::NormalsContainer   GetVertexNormals();
    FileReading::UVsContainer       GetVertexUV();
    FileReading::ColorsContainer    GetVertexColors();
    FileReading::IndicesContainer   GetIndices();

    FileReading::BoneWeightsAndIndicesContainer GetBoneWeightsAndIndices() { return FileReading::BoneWeightsAndIndicesContainer(); }
    D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology();

    PlyFile& operator=(PlyFile other)
    {
        swap(*this, other);
        return *this;
    }

    PlyFile(PlyFile&& source):
    m_file_stream()
    {
        swap(*this, source);
    }

    friend void swap(PlyFile& first, PlyFile& second); // nothrow

private:

    bool IsPly();
    void CheckFileFormat();
    void ReadHeader();
    Element* GetElement( const std::string & element_name );
    void GoToElement( const Element& element );
    static PlyType ConvertToPlyType( std::string input );
    static unsigned GetSize( PlyType );


    template<typename T, size_t N> void StoreProperties( const PlyFile::Element& element, const std::array<Element::Property*,N> & properties, std::array<T*,N> const & store_locations, const std::array<int,N> & strides )
    {
        switch ( m_format )
        {
        case PlyFormat::ASCII:
            {
                ASCIIStoreProperties( element, properties, store_locations, strides );
                break;
            }
        case PlyFormat::BINARY_BIG_ENDIAN:
            {
                BinStorePropertiesOtherEndian( element, properties, store_locations, strides );
                break;
            }
        case PlyFormat::BINARY_LITTLE_ENDIAN:
            {
                BinStoreProperties( element, properties, store_locations, strides );
                break;
            }
        }
    }

    template<typename T, size_t N> void StoreList( const PlyFile::Element& element, const Element::Property& property, std::array<T*,N> const & store_locations, const std::array<int,N> & strides )
    {
        switch ( m_format )
        {
        case PlyFormat::ASCII:
            {
                ASCIIStoreList( element, property, store_locations, strides );
                break;
            }
        case PlyFormat::BINARY_BIG_ENDIAN:
            {
                BinStoreListOtherEndian( element, property, store_locations, strides );
                break;
            }
        case PlyFormat::BINARY_LITTLE_ENDIAN:
            {
                BinStoreList( element, property, store_locations, strides );
                break;
            }
        }
    }

    uint8_t GetMaxNumberOfListElements( const PlyFile::Element& element, const Element::Property& property )
    {
        switch ( m_format )
        {
        case PlyFormat::ASCII:
            {
                return ASCIIGetMaxNumberOfListElements( element );
            }
        case PlyFormat::BINARY_BIG_ENDIAN:
            {
                return BinOtherEndianGetMaxNumberOfListElements( element, property );
            }
        case PlyFormat::BINARY_LITTLE_ENDIAN:
            {
                return BinGetMaxNumberOfListElements( element, property );
            }
        }
        return 0;
    }

    template<typename T> std::vector<T> ReturnList( const PlyFile::Element& element, const Element::Property& property )
    {
        switch ( m_format )
        {
        case PlyFormat::ASCII:
            {
                return ASCIIReturnList<T>( element, property );
            }
        case PlyFormat::BINARY_BIG_ENDIAN:
            {
                return BinReturnListOtherEndian<T>( element, property );
            }
        case PlyFormat::BINARY_LITTLE_ENDIAN:
            {
                return BinReturnList<T>( element, property );
            }
        }
        return std::vector<T>();
    }

    template<typename T, size_t N> void BinStoreProperties( const PlyFile::Element& element, const std::array<Element::Property*,N> & properties, std::array<T*,N> store_locations, std::array<int,N> const & strides )
    {

        GoToElement( element );

        for( unsigned line = 0; line < element.m_count; ++line )
        {
            auto pos = m_file_stream.tellg();

            for( unsigned property_number = 0; property_number < element.m_properties.size(); ++property_number )
            {
                for( unsigned store_index = 0; store_index < properties.size(); ++store_index )
                {
                    if( &element.m_properties[property_number] == properties[store_index] )
                    {
                        m_file_stream.seekg( properties[store_index]->m_offset, ios_base::cur );
                        *store_locations[store_index] = BinRead<T>( properties[store_index]->m_value_type );
                        store_locations[store_index] += strides[store_index];
                        m_file_stream.seekg( pos );
                        break;
                    }
                }
            }

            m_file_stream.seekg( element.m_size, ios_base::cur );
        }
    }

    template<typename T, size_t N> void BinStoreList( const PlyFile::Element& element, const Element::Property& property, std::array<T*,N> & store_locations, const std::array<int,N> & strides )
    {
        GoToElement( element );

        for( unsigned line = 0; line < element.m_count; ++line )
        {
            auto number_of_list_elements = BinRead<unsigned>( property.m_length_type );
            for( auto element_index = 0u;  element_index < std::max( number_of_list_elements, static_cast<unsigned>( N ) ) ; ++element_index )
            {
                if( element_index < N && element_index < number_of_list_elements )
                {
                    *store_locations[element_index] = BinRead<T>( property.m_value_type );
                    store_locations[element_index] += strides[element_index];
                }
                else if( element_index < N )
                {
                    store_locations[element_index] += strides[element_index];
                }
                else //if( element_index < number_of_list_elements )
                {
                    m_file_stream.seekg( GetSize( property.m_value_type ), std::ios_base::cur );
                }
            }
        }
    }

    uint8_t BinGetMaxNumberOfListElements( const PlyFile::Element& element, const Element::Property& property )
    {
        GoToElement( element );

        uint8_t max_number_of_list_elements = 0;
        for( unsigned line = 0; line < element.m_count; ++line )
        {
            auto number_of_list_elements = BinRead<uint8_t>( property.m_length_type );
            max_number_of_list_elements = std::max( max_number_of_list_elements,  number_of_list_elements);
            m_file_stream.seekg( GetSize( property.m_value_type ) * number_of_list_elements, std::ios_base::cur );
        }

        return max_number_of_list_elements;
    }

    template<typename T> std::vector<T> BinReturnList( const PlyFile::Element& element, const Element::Property& property )
    {
        auto max_number_of_list_elements = BinGetMaxNumberOfListElements(element, property);

        auto list = std::vector<T>( max_number_of_list_elements * element.m_count, std::numeric_limits<T>::max() );

        GoToElement( element );

        auto list_position = list.begin();
        for( unsigned line = 0; line < element.m_count; ++line )
        {
            auto number_of_list_elements = BinRead<size_t>( property.m_length_type );
            for( auto element_index = 0u;  element_index < number_of_list_elements; ++element_index )
            {
                list_position[element_index] = BinRead<T>( property.m_value_type );
            }
            list_position += max_number_of_list_elements;
        }
        return list;
    }

    template<typename T> T BinRead( PlyType type )
    {
        switch ( type )
        {
        case PlyType::CHAR:
            {
                int8_t temp;
                m_file_stream.read( reinterpret_cast<char*>( &temp ), sizeof( temp ) );
                return static_cast<T>( temp );
            }
        case PlyType::UCHAR:
            {
                uint8_t temp;
                m_file_stream.read( reinterpret_cast<char*>( &temp ), sizeof( temp ) );
                return static_cast<T>( temp );
            }
        case PlyType::SHORT:
            {
                int16_t temp;
                m_file_stream.read( reinterpret_cast<char*>( &temp ), sizeof( temp ) );
                return static_cast<T>( temp );
            }
        case PlyType::USHORT:
            {
                uint16_t temp;
                m_file_stream.read( reinterpret_cast<char*>( &temp ), sizeof( temp ) );
                return static_cast<T>( temp );
            }
        case PlyType::INT:
            {
                int32_t temp;
                m_file_stream.read( reinterpret_cast<char*>( &temp ), sizeof( temp ) );
                return static_cast<T>( temp );
            }
        case PlyType::UINT:
            {
                uint32_t temp;
                m_file_stream.read( reinterpret_cast<char*>( &temp ), sizeof( temp ) );
                return static_cast<T>( temp );
            }
        case PlyType::FLOAT:
            {
                float temp;
                m_file_stream.read( reinterpret_cast<char*>( &temp ), sizeof( temp ) );
                return static_cast<T>( temp );
            }
        case PlyType::DOUBLE:
            {
                double temp;
                m_file_stream.read( reinterpret_cast<char*>( &temp ), sizeof( temp ) );
                return static_cast<T>( temp );
            }
        default:
            {
                return T();
            }
        }
    }
    template<typename T, size_t N> void BinStorePropertiesOtherEndian( const PlyFile::Element& element, std::array<Element::Property*,N> const & properties, std::array<T*,N> store_locations, const std::array<int,N> & strides )
    {

        GoToElement( element );

        for( auto line = 0u; line < element.m_count; ++line )
        {
            auto pos = m_file_stream.tellg();

            for( auto property_number = 0u; property_number < element.m_properties.size(); ++property_number )
            {
                for( auto store_index = 0u; store_index < properties.size(); ++store_index )
                {
                    if( &element.m_properties[property_number] == properties[store_index] )
                    {
                        m_file_stream.seekg( properties[store_index]->m_offset, ios_base::cur );
                        *store_locations[store_index] = BinReadOtherEndian<T>( properties[store_index]->m_value_type );
                        store_locations[store_index] += strides[store_index];
                        m_file_stream.seekg( pos );
                        break;
                    }
                }
            }

            m_file_stream.seekg( element.m_size, ios_base::cur );
        }
    }

    template<typename T, size_t N> void BinStoreListOtherEndian( const PlyFile::Element& element, const Element::Property& property, std::array<T*,N> const & store_locations, const std::array<int,N> & strides )
    {
        GoToElement( element );

        for( auto line = 0u; line < element.m_count; ++line )
        {
            auto number_of_list_elements = BinReadOtherEndian<unsigned>( property.m_length_type );
            for( auto element_index = 0u;  element_index < std::max<unsigned>( number_of_list_elements, N ) ; ++element_index )
            {
                if( element_index < N && element_index < number_of_list_elements )
                {
                    *store_locations[element_index] = BinReadOtherEndian<T>( property.m_value_type );
                    store_locations[element_index] += strides[element_index];
                }
                else if( element_index < N )
                {
                    store_locations[element_index] += strides[element_index];
                }
                else //if( element_index < number_of_list_elements )
                {
                    m_file_stream.seekg( GetSize( property.m_value_type ), std::ios_base::cur );
                }
            }
        }
    }

    uint8_t BinOtherEndianGetMaxNumberOfListElements( const PlyFile::Element& element, const Element::Property& property )
    {
        GoToElement( element );

        uint8_t max_number_of_list_elements = 0;
        for( unsigned line = 0; line < element.m_count; ++line )
        {
            auto number_of_list_elements = BinReadOtherEndian<uint8_t>( property.m_length_type );
            max_number_of_list_elements = std::max( max_number_of_list_elements,  number_of_list_elements);
            m_file_stream.seekg( GetSize( property.m_value_type ) * number_of_list_elements, std::ios_base::cur );
        }

        return max_number_of_list_elements;
    }

    template<typename T> std::vector<T> BinReturnListOtherEndian( const PlyFile::Element& element, const Element::Property& property )
    {
        auto max_number_of_list_elements = BinOtherEndianGetMaxNumberOfListElements(element, property);

        auto list = std::vector<T>( max_number_of_list_elements * element.m_count, std::numeric_limits<T>::max() );

        GoToElement( element );

        auto list_position = list.begin();
        for( unsigned line = 0; line < element.m_count; ++line )
        {
            auto number_of_list_elements = BinReadOtherEndian<size_t>( property.m_length_type );
            for( auto element_index = 0u;  element_index < number_of_list_elements; ++element_index )
            {
                list_position[element_index] = BinReadOtherEndian<T>( property.m_value_type );
            }
            list_position += max_number_of_list_elements;
        }
        return list;
    }

    template<typename T> T BinReadOtherEndian( PlyType type )
    {
        switch ( type )
        {
        case PlyType::CHAR:
            {
                int8_t temp;
                m_file_stream.read( reinterpret_cast<char*>( &temp ), sizeof( temp ) );
                return static_cast<T>( temp );
            }
        case PlyType::UCHAR:
            {
                uint8_t temp;
                m_file_stream.read( reinterpret_cast<char*>( &temp ), sizeof( temp ) );
                return static_cast<T>( temp );
            }
        case PlyType::SHORT:
            {
                int16_t temp;
                m_file_stream.read( reinterpret_cast<char*>( &temp ), sizeof( temp ) );
                auto reversed = _byteswap_ushort( *reinterpret_cast< uint16_t* > ( &temp ) );
                return static_cast<T>( *reinterpret_cast< int16_t* > ( &reversed ) );
            }
        case PlyType::USHORT:
            {
                uint16_t temp;
                m_file_stream.read( reinterpret_cast<char*>( &temp ), sizeof( temp ) );
                return static_cast<T>( _byteswap_ushort( temp ) );
            }
        case PlyType::INT:
            {
                int32_t temp;
                m_file_stream.read( reinterpret_cast<char*>( &temp ), sizeof( temp ) );
                auto reversed = _byteswap_ulong( *reinterpret_cast< uint32_t* > ( &temp ) );
                return static_cast<T>( *reinterpret_cast< int32_t*> ( &reversed ) );
            }
        case PlyType::UINT:
            {
                uint32_t temp;
                m_file_stream.read( reinterpret_cast<char*>( &temp ), sizeof( temp ) );
                return static_cast<T>( _byteswap_ulong( temp ) );
            }
        case PlyType::FLOAT:
            {
                float temp;
                m_file_stream.read( reinterpret_cast<char*>( &temp ), sizeof( temp ) );
                auto reversed = _byteswap_ulong( *reinterpret_cast< uint32_t* > ( &temp ) );
                return static_cast<T>( *reinterpret_cast<float*>( &reversed ) );
            }
        case PlyType::DOUBLE:
            {
                double temp;
                m_file_stream.read( reinterpret_cast<char*>( &temp ), sizeof( temp ) );
                auto reversed = _byteswap_uint64( *reinterpret_cast< uint64_t* > ( &temp ) );
                return static_cast<T>( *reinterpret_cast< double* >( &reversed ) );
            }
        default:
            {
                return T();
            }
        }
    }
    template<typename T, size_t N> void ASCIIStoreProperties( const PlyFile::Element& element, const std::array<Element::Property*,N> & properties, std::array<T*,N> store_locations, const std::array<int,N> & strides )
    {

        GoToElement( element );

        std::string temp;
        for( unsigned line = 0; line < element.m_count; ++line )
        {
            for( unsigned property_number = 0; property_number < element.m_properties.size(); ++property_number )
            {
                auto pos = m_file_stream.tellg();

                for( unsigned store_index = 0; store_index < properties.size(); ++store_index )
                {
                    if( &element.m_properties[property_number] == properties[store_index] )
                    {
                        switch ( properties[store_index]->m_value_type )
                        {
                        case PlyType::CHAR:
                        case PlyType::INT:
                        case PlyType::SHORT:
                            {
                                m_file_stream >> temp;
                                *store_locations[store_index] = static_cast<T>( std::stoi( temp ) );
                                break;
                            }
                        case PlyType::UCHAR:
                        case PlyType::UINT:
                        case PlyType::USHORT:
                            {
                                m_file_stream >> temp;
                                *store_locations[store_index] = static_cast<T>( std::stoul( temp ) );
                                break;
                            }
                        case PlyType::FLOAT:
                            {
                                m_file_stream >> temp;
                                *store_locations[store_index] = static_cast<T>( std::stof( temp ) );
                                break;
                            }
                        case PlyType::DOUBLE:
                            {
                                m_file_stream >> temp;
                                *store_locations[store_index] = static_cast<T>( std::stod( temp ) );
                                break;
                            }
                        default:
                            {
                                m_file_stream >> *store_locations[store_index];
                                break;
                            }
                        }

                        store_locations[store_index] += strides[store_index];
                        break;
                    }
                }

                m_file_stream.seekg( pos );
                // go to next property
                if( property_number == element.m_properties.size()-1)
                {
                    m_file_stream.ignore( 256, '\n' ); // ignore the last entry
                }
                else
                {
                    m_file_stream.ignore( 256, ' ' ); // ignore any other entry
                }
            }
        }
    }

    template<typename T, size_t N> void ASCIIStoreList( const PlyFile::Element& element, const Element::Property& property, std::array<T*,N> const & store_locations, const std::array<int,N> & strides )
    {
        GoToElement( element );
        std::string temp;

        for( auto line = 0u; line < element.m_count; ++line )
        {

            m_file_stream >> temp;
            auto number_of_list_elements = static_cast<unsigned>( std::stoul( temp ) );


            for( auto element_index = 0u;  element_index < N ; ++element_index )
            {

                switch ( property.m_value_type )
                {
                case PlyType::CHAR:
                case PlyType::INT:
                case PlyType::SHORT:
                    {
                        m_file_stream >> temp;
                        *store_locations[element_index] = static_cast<T>( std::stoi( temp ) );
                        break;
                    }
                case PlyType::UCHAR:
                case PlyType::UINT:
                case PlyType::USHORT:
                    {
                        m_file_stream >> temp;
                        *store_locations[element_index] = static_cast<T>( std::stoul( temp ) );
                        break;
                    }
                case PlyType::FLOAT:
                    {
                        m_file_stream >> temp;
                        *store_locations[element_index] = static_cast<T>( std::stof( temp ) );
                        break;
                    }
                case PlyType::DOUBLE:
                    {
                        m_file_stream >> temp;
                        *store_locations[element_index] = static_cast<T>( std::stod( temp ) );
                        break;
                    }
                default:
                    {
                        m_file_stream >> *store_locations[element_index];
                        break;
                    }
                }
                store_locations[element_index] += strides[element_index];
            }

            if( number_of_list_elements > N )
            {
                m_file_stream.ignore( 256, '\n' ); // ignore the rest of the line
            }

        }
    }

    uint8_t ASCIIGetMaxNumberOfListElements( const PlyFile::Element& element )
    {
        GoToElement( element );
        std::string temp;

        uint8_t max_number_of_list_elements = 0;
        for( auto line = 0u; line < element.m_count; ++line )
        {
            m_file_stream >> temp;
            auto number_of_list_elements = static_cast<uint8_t>( std::stoul( temp ) );
            max_number_of_list_elements = std::max( max_number_of_list_elements, number_of_list_elements );
            m_file_stream.ignore( 256, '\n' ); // ignore the rest of the line
        }
        return max_number_of_list_elements;
    }

    template<typename T> std::vector<T> ASCIIReturnList( const PlyFile::Element& element, const Element::Property& property )
    {
        auto max_number_of_list_elements = ASCIIGetMaxNumberOfListElements( element );

        auto list = std::vector<T>( max_number_of_list_elements * element.m_count, std::numeric_limits<T>::max() );

        std::string temp;
        auto list_position = list.begin();
        GoToElement( element );
        for( auto line = 0u; line < element.m_count; ++line )
        {
            m_file_stream >> temp;
            auto number_of_list_elements = static_cast<unsigned>( std::stoul( temp ) );
            for( auto element_index = 0u;  element_index < number_of_list_elements; ++element_index )
            {
                switch ( property.m_value_type )
                {
                case PlyType::CHAR:
                case PlyType::INT:
                case PlyType::SHORT:
                    {
                        m_file_stream >> temp;
                        list_position[element_index] = static_cast<T>( std::stoi( temp ) );
                        break;
                    }
                case PlyType::UCHAR:
                case PlyType::UINT:
                case PlyType::USHORT:
                    {
                        m_file_stream >> temp;
                        list_position[element_index] = static_cast<T>( std::stoul( temp ) );
                        break;
                    }
                case PlyType::FLOAT:
                    {
                        m_file_stream >> temp;
                        list_position[element_index] = static_cast<T>( std::stof( temp ) );
                        break;
                    }
                case PlyType::DOUBLE:
                    {
                        m_file_stream >> temp;
                        list_position[element_index] = static_cast<T>( std::stod( temp ) );
                        break;
                    }
                default:
                    {
                        m_file_stream >> list_position[element_index];
                        break;
                    }
                }
            }
            list_position += max_number_of_list_elements;
        }
        return list;
    }
};


inline void swap(PlyFile& first, PlyFile& second)
{
    using std::swap;
    // by swapping the members of two classes,
    // the two classes are effectively swapped
    swap(first.m_file_stream,  second.m_file_stream);
    swap(first.m_format,       second.m_format);
    swap(first.m_header_start, second.m_header_start);
    swap(first.m_data_start,   second.m_data_start);
    swap(first.m_elements,     second.m_elements);
    swap(first.m_comments,     second.m_comments);
}
