#pragma once

#include "AxisAlignedBox.h"
#include "AxisAlignedBoxHierarchyMesh.h"
#include "OrientedBox.h"
#include "Plane.h"
#include "Ray.h"
#include "Sphere.h"
#include "SphereHierarchyMesh.h"
#include "Triangle.h"

#include <Math\FloatMatrixTypes.h>
#include <Utilities\Range.h>
#include <vector>
#include <cstdint>

namespace BoundingShapes
{
    // two 2d line segments, AB and CD
    bool Intersect( Math::Float2 a, Math::Float2 b, Math::Float2 c, Math::Float2 d );
    // two 2d line segments, AB and CD, assuming that Dot(AB, CD) >= 0
    bool IntersectSameOrientation( Math::Float2 a, Math::Float2 b, Math::Float2 c, Math::Float2 d );

    bool Contains( Sphere const & sphere, Math::Float3 point );
    bool Intersect( Sphere sphere1, Sphere const & sphere2 );

    bool Contains( AxisAlignedBox const & box, Math::Float3 point );
    bool Intersect( AxisAlignedBox const & box1, AxisAlignedBox const & box2 );
    bool Intersect( AxisAlignedBox const & box, Ray const & ray);

    bool Contains( OrientedBox const & box, Math::Float3 point );
    bool Intersect( OrientedBox box1, OrientedBox const & box2 );

    bool Intersect( AxisAlignedBox box, Sphere const & sphere );
    bool Intersect( OrientedBox box, Sphere const & sphere );
    bool Intersect( OrientedBox const & box1, AxisAlignedBox const & box2 );
    bool Intersect( Triangle const & triangle, Sphere const & sphere );
    bool Intersect( Triangle const & triangle, AxisAlignedBox const & box );
    bool Intersect( Triangle const & triangle, OrientedBox const & box );
    bool Intersect( SphereHierarchyMesh const & mesh, OrientedBox const & box );
    bool Intersect( AxisAlignedBoxHierarchyMesh const & mesh, OrientedBox const & box );

    // does an intersection test for a box and a frustum defined by a projection matrix
    // -w <= x <= w
    // -w <= y <= w
    //  0 <= z <= w
    bool IntersectFrustum( AxisAlignedBox const & box, Math::Float4x4 const projection_matrix );
    // returns the indices of the intersecting boxes
    void IntersectFrustum( Range<AxisAlignedBox const *> boxes, Math::Float4x4 const & projection_matrix, std::vector<uint32_t> & intersecting_indices );
    // returns the indices of the intersecting boxes after transforming them
    void IntersectFrustum( Range<AxisAlignedBox const *> boxes, Range<Math::Float4x4 const*> transforms, Math::Float4x4 const & projection_matrix, std::vector<uint32_t> & intersecting_indices );

    // tests whether the box is (partly) in front of the plane
    bool InFront( AxisAlignedBox const & box, Plane plane_equation );

    // tests whether the point is in front of the plane
    bool InFront( Math::Float3 point, Plane plane_equation );

    // tests whether the box intersects with the area contained by the planes
    bool Intersect(AxisAlignedBox const & box, Range<Plane const*> planes);
    // appends the indices of the boxes that intersect with the area contained by the planes
    void Intersect( Range<AxisAlignedBox const *> boxes, Range<Plane const*> planes, std::vector<uint32_t> & contained_indices);
}
