#include "ConstantBufferTypeAndIDFunctions.h"
#include "ConstantBufferContainer.h"

using namespace Graphics;

ConstantBufferID Graphics::GetBufferID( Range<ConstantBufferTypeAndID const *> type_and_id, ConstantBufferType type )
{
    for( auto i = 0u; i < Size(type_and_id); ++i )
    {
        if(type_and_id[i].type == type)
        {
            return type_and_id[i].id;
        }
    }
    return c_invalid_constant_buffer_id;
}


void Graphics::ReleaseBuffers(Range<ConstantBufferTypeAndID const *> type_and_ids, ConstantBufferContainer & constant_buffer_container)
{
    for( auto & type_and_id : type_and_ids)
    {
        constant_buffer_container.Remove(type_and_id.id);
    }
}
