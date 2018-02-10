#include "Structures.h"

namespace Logic
{
    // ########################### Data extraction:

    void GetMeshTriangleVertices2D(unsigned const triangle_index,
        NavigationMesh const & mesh,
        Math::Float2 & vertex_0,
        Math::Float2 & vertex_1,
        Math::Float2 & vertex_2);
    
    void GetMeshTriangleVertices3D(unsigned const triangle_index,
        NavigationMesh const & mesh,
        Math::Float3 & vertex_0,
        Math::Float3 & vertex_1,
        Math::Float3 & vertex_2);
    // ########################### GEOMETRY:

    Math::Float2 GetTriangleCentroid(unsigned const triangle_index, NavigationMesh const & mesh);


    // Check how far the input point c is from an edge formed by b - a
    // by projecting (c - a) onto the edge and determining the point
    // it was projected to along the edge.
    Math::Float2 GetClosestPointOnEdge(Math::Float2 const a,
        Math::Float2 const b,
        Math::Float2 const c);

    void GetClosestPointOnTriangle2D(Math::Float2 const position,
        Math::Float2 const vertex_0,
        Math::Float2 const vertex_1,
        Math::Float2 const vertex_2,
        Math::Float2 & closest_point,
        float & minimum_distance);


    // Make the triangle formed by the vertices relative to 
    // the input positions space and rotate it so that it 
    // is flat on the X-Y layer.
    // Take the closest point on the transformed triangle 
    // to the origin in 2d space. Extend it by the offset 
    // between the origin and transformed triangle. 
    // Finally apply the inverse of the transformation of the
    // triangle onto the found point to make it absolute.
    void GetClosestPointOnTriangle3D(Math::Float3 position,
        Math::Float3 vertex_0,
        Math::Float3 vertex_1,
        Math::Float3 vertex_2,
        Math::Float3 & closest_point_3d,
        float & minimum_distance);


    // Use a constrained projection of the input point on all triangle edges
    // to determine their closest point. Return the triangle and position
    // of the closest found one.
    void GetClosestPointOnMesh(Math::Float3 const input_position,
        NavigationMesh const & mesh,
        Math::Float3 & closest_point,
        unsigned & closest_triangle);


    // Return true if the input position is inside the triangle
    // defined by the three input vertices in a right-handed rotational sense
    bool TriangleContainsPosition(Math::Float2 const position,
        Math::Float2 const vertex_0,
        Math::Float2 const vertex_1,
        Math::Float2 const vertex_2);


    // This does the same as GetClosestPointOnTriangle3D() 
    // but without inverting the transformation on the triangle,
    // which is not needed for a mere distance calculation.
    float GetPointDistanceFromTriangle3D(Math::Float3 const position,
        Math::Float3 vertex_0,
        Math::Float3 vertex_1,
        Math::Float3 vertex_2);


    // If any triangle of the mesh contains the input position in the X-Y layer,
    // return its index. If multiple triangles do, return the closest one in 3D-Space.
    // Otherwise return unsigned(-1)
    unsigned GetContainingTriangleIndex(Math::Float3 const position_3D,
        NavigationMesh const & mesh);


    // Find all triangles sharing an edge with the input triangle
    void GetAdjacentTriangles(unsigned const input_triangle,
        NavigationMesh const & mesh,
        // output
        std::vector<unsigned> & adjacent_triangles,
        PortalList & adjacency_portals);
}
