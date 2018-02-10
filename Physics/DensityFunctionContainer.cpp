
#include "DensityFunctionContainer.h"

#include <Utilities\IndexedHelp.h>
#include <Utilities\VectorHelper.h>
#include <Utilities\ContainerHelpers.h>

using namespace Physics;

void Physics::AddFunction(DensityFunctionType function, BodyID body, DensityFunctionContainer & self)
{
    self.functions.emplace_back(move(function));
    self.bodies.push_back(body);

    AddIndexToIndices(self.body_to_data, body.index, uint32_t(self.functions.size() - 1));
}


void Physics::Remove(Range<BodyID const*> entities, DensityFunctionContainer & self)
{
    RemoveIDsWithSwapAndPrune(entities, self.body_to_data, self.bodies, self.functions);
}


bool Physics::Contains(BodyID body, DensityFunctionContainer const & self)
{
    return GetOptional(self.body_to_data, body.index) != c_invalid_index;
}
