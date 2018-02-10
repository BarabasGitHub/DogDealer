#pragma once

#include "ShapeType.h"

#include "AxisAlignedBox.h"
#include "OrientedBox.h"

struct CollisionMeshFileHeader
{
    // box around all other shapes
    BoundingShapes::AxisAlignedBox axis_aligned_box;
    uint8_t number_of_shapes;
    // shape types ...
    // possible shapes:
    // - sphere
    // - oriented box
    // - mesh
};

// Mesh:
// node_count
// mesh.nodes
// vertex_position_count
// mesh.vertex_positions
