#pragma once

#include <iostream>
#include <vector>


template<typename Type> void WriteVector( std::ostream& stream, std::vector<Type> const & vector );
template<typename Type> void WriteObject( std::ostream& stream, Type type );
template<typename Type> Type ReadObject( std::istream& stream );
template<typename Type> void ReadObject( std::istream& stream, Type & object );
template<typename Type> void ReadVector( std::istream& stream, std::vector<Type>& vector );
template<typename Type> void Read( std::istream& stream, Type* data_pointer, size_t count );
void Read( std::istream& stream, void* data_pointer, size_t byte_size );


template<typename Type>
void WriteVector(std::ostream& stream, std::vector<Type> const & vector)
{
    auto data = vector.data();
    auto number_of_elements = vector.size();

    stream.write( reinterpret_cast<char const *>(data), sizeof(Type) * number_of_elements);
}


template<typename Type>
void WriteObject(std::ostream& stream, Type type)
{
    stream.write( reinterpret_cast<char*>(&type), sizeof(type));
}


template<typename Type>
Type ReadObject(std::istream& stream)
{
    Type output;
    ReadObject(stream, output);
    return output;
}


template<typename Type>
void ReadObject( std::istream& stream, Type & object )
{
    stream.read( reinterpret_cast<char*>(&object), sizeof(object));
}


template<typename Type>
void ReadVector(std::istream& stream, std::vector<Type>& vector)
{
    Read( stream, vector.data(), vector.size() );
}


template<typename Type>
void Read( std::istream& stream, Type* data_pointer, size_t count )
{
    Read( stream, static_cast<void*>( data_pointer ), count * sizeof( Type ) );
}


inline void Read(std::istream& stream, void* data_pointer, size_t byte_size)
{
    stream.read( static_cast<char*>(data_pointer), byte_size );
}