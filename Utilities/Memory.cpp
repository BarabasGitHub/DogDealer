
#include "Memory.h"

#include <cstring>

void VoidZero( void * memory, size_t size )
{
    memset( memory, 0, size );
}


void VoidCopy( void const * __restrict source, size_t size, void * __restrict destination )
{
    memcpy( destination, source, size );
}
