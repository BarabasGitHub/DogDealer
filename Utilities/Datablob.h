#pragma once

#include <vector>
#include <cstdint>
#include <istream>

typedef std::vector<std::uint8_t> DataBlob;

inline DataBlob CreateDataBlobTillEndOfStream( std::istream& data_stream )
{
    auto begin = data_stream.tellg( );
    data_stream.seekg( 0, std::ios::end );
    auto size = data_stream.tellg( ) - begin;
    auto blob = DataBlob( size );
    data_stream.seekg( begin );
    data_stream.read( reinterpret_cast<char*>( blob.data( ) ), size );
    return blob;
}