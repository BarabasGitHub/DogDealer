#pragma once

template<typename Type>
void Zero( Type * memory, size_t size );

template<typename Type>
void Copy( Type const * __restrict source, size_t size, Type * __restrict destination );

// mainly used to hide the implementation
void VoidZero( void * memory, size_t size );
// mainly to hide the implementation
void VoidCopy( void const * __restrict source, size_t size, void * __restrict destination );

// fill the memory with a uniform value
template<typename Type>
void Fill(Type value, Type * data, size_t size);

// template implementations


template<typename Type>
void Zero( Type * memory, size_t size )
{
    VoidZero( static_cast<void*>(memory), size * sizeof( Type ) );
}


template<typename Type>
void Copy( Type const * __restrict source, size_t size, Type * __restrict destination )
{
    VoidCopy( static_cast<void const* __restrict>( source ), size * sizeof( Type ), static_cast<void* __restrict>( destination ) );
}


template<typename Type>
void Fill(Type value, Type * data, size_t size)
{
    for( auto i = 0u; i < size; ++i )
    {
        data[i] = value;
    }
}