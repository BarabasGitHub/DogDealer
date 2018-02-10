
#include "SphereContainer.h"

#include <Utilities\IndexedHelp.h>
#include <Utilities\VectorHelper.h>
#include <Utilities\ContainerHelpers.h>

using namespace Physics;
using namespace BoundingShapes;

Physics::SphereContainer::SphereContainer()
{
    Append(offsets, 0u);
}


void Physics::AddSphere(BodyID body, Sphere sphere, SphereContainer & self)
{
    AddSpheres(body, CreateRange(&sphere, 1), self);
}


void Physics::AddSpheres(BodyID body, Range<BoundingShapes::Sphere const*> spheres, SphereContainer & self)
{
    assert(!Contains(body, self));
    Append(self.spheres, spheres);
    Append(self.bodies, body, Size(spheres));
    Append(self.offsets, uint32_t(Size(self.spheres)));
    AddIndexToIndices( self.body_to_offset, body.index, uint32_t(Size(self.offsets) - 2));
}


void Physics::Remove(Range<BodyID const*> bodies, SphereContainer & self)
{
    auto indices = RemoveIndices( self.body_to_offset, bodies );
    std::sort( begin( indices ), end( indices ) );

    RemoveEntries( self.spheres, self.offsets, indices );
    RemoveEntries( self.bodies, self.offsets, indices );
    RemoveOffsets( self.offsets, indices );
}


bool Physics::Contains( BodyID id, SphereContainer const & self )
{
    return GetOptional(self.body_to_offset, id.index) != c_invalid_index;
}
