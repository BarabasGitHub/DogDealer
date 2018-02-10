
#include "OrientedBoxContainer.h"

#include <Utilities\IndexedHelp.h>
#include <Utilities\VectorHelper.h>
#include <Utilities\ContainerHelpers.h>

using namespace Physics;
using namespace BoundingShapes;

Physics::OrientedBoxContainer::OrientedBoxContainer()
{
    Append(offsets, 0u);
}


void Physics::AddBox(BodyID body, OrientedBox box, OrientedBoxContainer & self)
{
    AddBoxes(body, CreateRange(&box, 1), self);
}


void Physics::AddBoxes(BodyID body, Range<BoundingShapes::OrientedBox const*> boxes, OrientedBoxContainer & self)
{
    assert(!Contains(body, self));
    Append(self.boxes, boxes);
    Append(self.bodies, body, Size(boxes));
    Append(self.offsets, uint32_t(Size(self.boxes)));
    AddIndexToIndices(self.body_to_offset, body.index, uint32_t(Size(self.offsets) - 2));
}


void Physics::Remove(Range<BodyID const*> bodies, OrientedBoxContainer & self)
{
    auto indices = RemoveIndices( self.body_to_offset, bodies );
    std::sort( begin( indices ), end( indices ) );

    RemoveEntries( self.boxes, self.offsets, indices );
    RemoveEntries( self.bodies, self.offsets, indices );
    RemoveOffsets( self.offsets, indices );
}


bool Physics::Contains( BodyID id, OrientedBoxContainer const & self )
{
    return GetOptional(self.body_to_offset, id.index) != c_invalid_index;
}
