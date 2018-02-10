#pragma once

#include "BodyID.h"
#include "DensityFunction.h"
#include <Utilities\Range.h>

#include <vector>

namespace Physics
{
    struct DensityFunctionContainer
    {
        std::vector<DensityFunctionType> functions;
        std::vector<BodyID> bodies;

        std::vector<uint32_t> body_to_data;
    };


    void AddFunction(DensityFunctionType function, BodyID entity, DensityFunctionContainer & self);
    void Remove(Range<BodyID const*> bodies, DensityFunctionContainer & self);
    bool Contains(BodyID entity, DensityFunctionContainer const & self);
}
