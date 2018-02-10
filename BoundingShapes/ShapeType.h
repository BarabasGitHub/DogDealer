#pragma once
#include <cstdint>

namespace BoundingShapes
{
    enum struct ShapeType : uint8_t
    {
        AxisAlignedBox,
        AxisAlignedBoxHierarchyMesh,
        OrientedBox,
        Sphere,
        SphereHierarchyMesh,
        Triangle
    };
}