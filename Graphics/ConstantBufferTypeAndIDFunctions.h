#pragma once

#include "ConstantBufferTypeAndID.h"

#include <Utilities\Range.h>

namespace Graphics
{
    struct ConstantBufferContainer;
    ConstantBufferID GetBufferID( Range<ConstantBufferTypeAndID const *> type_and_id, ConstantBufferType type );

    void ReleaseBuffers(Range<ConstantBufferTypeAndID const *> type_and_ids, ConstantBufferContainer & constant_buffer_container);
}
