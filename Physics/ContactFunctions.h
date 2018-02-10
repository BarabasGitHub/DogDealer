#pragma once

#include <BoundingShapes\OrientedBox.h>
#include <BoundingShapes\Triangle.h>
#include <BoundingShapes\AxisAlignedBox.h>
#include <BoundingShapes\Plane.h>

#include <Math\FloatTypes.h>

#include <Utilities\Range.h>

#include <vector>
#include <array>
#include <cstdint>

// input:
// - plane_normals & plane_origins,
// - vertices, each vertex forms an edge with the subsequent vertex
std::vector<Math::Float3> SutherlandHodgman( Range<BoundingShapes::Plane const *> planes, Range<Math::Float3 const*> vertices );

// returns the number of contact points and outputs the contact points
uint8_t FindContactPoints( Math::Float3 extent, BoundingShapes::Triangle triangle, Math::Float3 separation_axis, Range<Math::Float3*> output_contact_points );

void MakePlaneEquations( Math::Float3 extent, uint8_t reference_face_index, std::array<BoundingShapes::Plane, 4> & planes );
void MakePlaneEquations( BoundingShapes::OrientedBox box, uint8_t reference_face_index, std::array<BoundingShapes::Plane, 4> & planes );
void MakePlaneEquations( BoundingShapes::Triangle const & triangle, std::array<BoundingShapes::Plane, 3> & planes );

size_t DiscardVertices( BoundingShapes::Plane plane, Range<Math::Float3*> vertices );
size_t DiscardVertices( BoundingShapes::Plane plane, float const tolerance, Range<Math::Float3*> vertices );
size_t DiscardVertices( BoundingShapes::AxisAlignedBox const & box, uint8_t reference_face_index, Range<Math::Float3*> vertices );
size_t DiscardVertices( BoundingShapes::AxisAlignedBox const & box, uint8_t reference_face_index, float const tolerance, Range<Math::Float3*> vertices );
size_t DiscardVertices( BoundingShapes::OrientedBox const & box, uint8_t reference_face_index, Range<Math::Float3*> vertices );
size_t DiscardVertices( BoundingShapes::OrientedBox const & box, uint8_t reference_face_index, float const tolerance, Range<Math::Float3*> vertices );