#include "AINavigationMeshFunctions.h"

#include <Math/FloatTypes.h>
#include <Math/FloatOperators.h>

#include <Math/MathFunctions.h>

#include <array>


namespace Logic{

    // ########################### Data extraction:

    void GetMeshTriangleVertices2D(unsigned const triangle_index,
        NavigationMesh const & mesh,
        Math::Float2 & vertex_0,
        Math::Float2 & vertex_1,
        Math::Float2 & vertex_2)
    {
        auto vertices_start = triangle_index * 3;

        vertex_0 = mesh.vertices[mesh.indices[vertices_start]];
        vertex_1 = mesh.vertices[mesh.indices[vertices_start + 1]];
        vertex_2 = mesh.vertices[mesh.indices[vertices_start + 2]];
    };


    void GetMeshTriangleVertices3D(unsigned const triangle_index,
        NavigationMesh const & mesh,
        Math::Float3 & vertex_0,
        Math::Float3 & vertex_1,
        Math::Float3 & vertex_2)
    {
        auto vertices_start = triangle_index * 3;

        auto vertex_0_index = mesh.indices[vertices_start];
        auto vertex_1_index = mesh.indices[vertices_start + 1];
        auto vertex_2_index = mesh.indices[vertices_start + 2];

        vertex_0 = Math::Float3(mesh.vertices[vertex_0_index].x,
            mesh.vertices[vertex_0_index].y,
            mesh.vertices_z[vertex_0_index]);

        vertex_1 = Math::Float3(mesh.vertices[vertex_1_index].x,
            mesh.vertices[vertex_1_index].y,
            mesh.vertices_z[vertex_1_index]);

        vertex_2 = Math::Float3(mesh.vertices[vertex_2_index].x,
            mesh.vertices[vertex_2_index].y,
            mesh.vertices_z[vertex_2_index]);
    };

    // ########################### GEOMETRY:

    Math::Float2 GetTriangleCentroid(unsigned const triangle_index, NavigationMesh const & mesh)
    {
        Math::Float2 vertex_0, vertex_1, vertex_2;
        GetMeshTriangleVertices2D(triangle_index, mesh, vertex_0, vertex_1, vertex_2);

        Math::Float2 centroid = (vertex_0 + vertex_1 + vertex_2) / 3;

        return centroid;
    }


    // Check how far the input point c is from an edge formed by b - a
    // by projecting (c - a) onto the edge and determining the point
    // it was projected to along the edge.
    Math::Float2 GetClosestPointOnEdge(Math::Float2 const a,
        Math::Float2 const b,
        Math::Float2 const c)
    {
        // Get edge vector (b - a) and its length
        Math::Float2 edge = b - a;
        auto edge_length = Math::Norm(edge);

        // Get relative position of input to a
        auto input_offset = c - a;

        // Project input offset from a onto edge
        auto projection = Math::Dot(Math::Normalize(edge), input_offset);

        // Determine how far the projected point is along the edge
        auto factor = projection / edge_length;

        // Constrain factor to 0 and 1
        factor = std::min(1.0f, std::max(factor, 0.0f));

        // Get the absolte point that input_offset was projected to
        Math::Float2 projection_target = a + edge * factor;

        return projection_target;
    }


    void GetClosestPointOnTriangle2D(Math::Float2 const position,
        Math::Float2 const vertex_0,
        Math::Float2 const vertex_1,
        Math::Float2 const vertex_2,
        Math::Float2 & closest_point,
        float & minimum_distance)
    {
        // Get closest points on all three edges
        auto closest_0 = GetClosestPointOnEdge(vertex_0, vertex_1, position);
        auto closest_1 = GetClosestPointOnEdge(vertex_1, vertex_2, position);
        auto closest_2 = GetClosestPointOnEdge(vertex_2, vertex_0, position);

        // Get distances of input position to all three points
        auto distance_0 = Math::Norm(position - closest_0);
        auto distance_1 = Math::Norm(position - closest_1);
        auto distance_2 = Math::Norm(position - closest_2);

        // Check which edge the point is closest to
        minimum_distance = std::numeric_limits<float>::max();

        // Now its getting redundant!
        if (distance_0 < minimum_distance)
        {
            minimum_distance = distance_0;
            closest_point = closest_0;
        }

        if (distance_1 < minimum_distance)
        {
            minimum_distance = distance_1;
            closest_point = closest_1;
        }

        if (distance_2 < minimum_distance)
        {
            minimum_distance = distance_2;
            closest_point = closest_2;
        }
    }


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
        float & minimum_distance)
    {
        // Make triangle relative to input point
        vertex_0 -= position;
        vertex_1 -= position;
        vertex_2 -= position;

        // Get normal of triangle
        auto edge_0 = vertex_1 - vertex_0;
        auto edge_1 = vertex_2 - vertex_0;

        auto normal = Math::Cross(edge_0, edge_1);
        normal = Math::Normalize(normal);

        // Get difference between normal and (0,0,1)
        auto rotation = Math::RotationBetweenAxes(normal, Math::Float3(0, 0, 1));

        // Check whether rotation is #ind, so that the axes are at a 180 degree angle
        // and the triangle merely needs to be flipped
        if (rotation != rotation)
        {
            // Flip triangle
            auto temp = vertex_2;
            vertex_2 = vertex_1;
            vertex_1 = temp;
        }
        else
        {
            // Rotate triangle to be on X-Y plane
            vertex_0 = Math::Rotate(vertex_0, rotation);
            vertex_1 = Math::Rotate(vertex_1, rotation);
            vertex_2 = Math::Rotate(vertex_2, rotation);
        }

        // Project points into 2d, with input position being the origin
        auto position_projected = Math::Float2(0, 0);
        auto vertex_0_projected = Math::Float2(vertex_0.x, vertex_0.y);
        auto vertex_1_projected = Math::Float2(vertex_1.x, vertex_1.y);
        auto vertex_2_projected = Math::Float2(vertex_2.x, vertex_2.y);

        // Get closest point in 2d
        Math::Float2 closest_point;
        float distance_2D;

        if (TriangleContainsPosition(position_projected,
            vertex_0_projected,
            vertex_1_projected,
            vertex_2_projected))
        {
            // Distance in 2D is 0 if position is contained in projected triangle            
            closest_point = position_projected;
        }
        else
        {
            // Otherwise get closest point on triangle
            GetClosestPointOnTriangle2D(Math::Float2(0, 0),
                vertex_0_projected,
                vertex_1_projected,
                vertex_2_projected,
                closest_point,
                distance_2D);
        }

        // Extend distance by z offset to triangle
        auto triangle_z = vertex_0.z;
        auto closest_point_3d_transformed = Math::Float3(closest_point.x, closest_point.y, triangle_z);

        minimum_distance = Math::Norm(closest_point_3d_transformed);

        // Rotate point back from the original transformation 
        // of the triangle into the X-Y layer
        auto inverse_rotation = Math::Inverse(rotation);
        closest_point_3d = Math::Rotate(closest_point_3d_transformed, inverse_rotation);

        // Shift point back to no longer be relative to input position
        closest_point_3d += position;
    }


    // Use a constrained projection of the input point on all triangle edges
    // to determine their closest point. Return the triangle and position
    // of the closest found one.
    void GetClosestPointOnMesh(Math::Float3 const input_position,
        NavigationMesh const & mesh,
        Math::Float3 & closest_point,
        unsigned & closest_triangle)
    {
        float minimum_distance = std::numeric_limits<float>::max();

        // Iterate over triangles
        for (auto i = 0; i < mesh.indices.size() / 3; i++)
        {
            // Get the three involved vertices
            Math::Float3 vertex_0, vertex_1, vertex_2;
            GetMeshTriangleVertices3D(i, mesh, vertex_0, vertex_1, vertex_2);

            auto current_distance = std::numeric_limits<float>::max();
            Math::Float3 current_closest_point;

            // TODO: Convert to 3D
            GetClosestPointOnTriangle3D(input_position,
                vertex_0,
                vertex_1,
                vertex_2,
                current_closest_point,
                current_distance);

            // Check if point is closest to current triangle so far
            if (current_distance < minimum_distance)
            {
                minimum_distance = current_distance;
                closest_point = current_closest_point;

                closest_triangle = i;
            }
        }
    }


    // Return true if the input position is inside the triangle
    // defined by the three input vertices in a right-handed rotational sense
    bool TriangleContainsPosition(Math::Float2 const position,
        Math::Float2 const vertex_0,
        Math::Float2 const vertex_1,
        Math::Float2 const vertex_2)
    {
        // Use Barycentric Technique, following the description (and code }X) on this page:
        // http://www.blackpawn.com/texts/pointinpoly/default.html 

        // Compute vectors        
        Math::Float2 v_0 = vertex_1 - vertex_0;
        Math::Float2 v_1 = vertex_2 - vertex_0;
        Math::Float2 v_2 = position - vertex_0;

        // Compute dot products
        float dot00 = Math::Dot(v_0, v_0);
        float dot01 = Math::Dot(v_0, v_1);
        float dot02 = Math::Dot(v_0, v_2);
        float dot11 = Math::Dot(v_1, v_1);
        float dot12 = Math::Dot(v_1, v_2);

        // Compute barycentric coordinates
        float inv_denom = 1 / (dot00 * dot11 - dot01 * dot01);
        float v = (dot00 * dot12 - dot01 * dot02) * inv_denom;
        float u = (dot11 * dot02 - dot01 * dot12) * inv_denom;

        // Check if point is in triangle
        return (u >= 0.0f) && (v >= 0.0f) && (u + v <= 1.0f);
    }


    // This does the same as GetClosestPointOnTriangle3D() 
    // but without inverting the transformation on the triangle,
    // which is not needed for a mere distance calculation.
    float GetPointDistanceFromTriangle3D(Math::Float3 const position,
        Math::Float3 vertex_0,
        Math::Float3 vertex_1,
        Math::Float3 vertex_2)
    {
        // Make triangle relative to input point
        vertex_0 -= position;
        vertex_1 -= position;
        vertex_2 -= position;

        // Get normal of triangle
        auto edge_0 = vertex_1 - vertex_0;
        auto edge_1 = vertex_2 - vertex_0;

        auto normal = Math::Cross(edge_0, edge_1);
        normal = Math::Normalize(normal);

        if(!(normal.x == 0 && normal.y == 0))
        {
            // Get difference between normal and (0,0,1)
            auto rotation = Math::RotationBetweenAxes(normal, Math::Float3(0, 0, 1));
            // Check whether rotation is #ind, so that the axes are at a 180 degree angle
            // and the triangle merely needs to be flipped
            if(rotation != rotation)
            {
                // Flip triangle
                auto temp = vertex_2;
                vertex_2 = vertex_1;
                vertex_1 = temp;
            }
            else
            {
                // Rotate triangle to be on X-Y plane
                vertex_0 = Math::Rotate(vertex_0, rotation);
                vertex_1 = Math::Rotate(vertex_1, rotation);
                vertex_2 = Math::Rotate(vertex_2, rotation);
            }
        }

        // Project points into 2d, with input position being the origin
        auto position_projected = Math::Float2(0, 0);
        auto vertex_0_projected = Math::Float2(vertex_0.x, vertex_0.y);
        auto vertex_1_projected = Math::Float2(vertex_1.x, vertex_1.y);
        auto vertex_2_projected = Math::Float2(vertex_2.x, vertex_2.y);

        // Get closest point in 2d
        Math::Float2 closest_point;
        float distance_2D;

        if (TriangleContainsPosition(position_projected,
            vertex_0_projected,
            vertex_1_projected,
            vertex_2_projected))
        {
            // Distance in 2D is 0 if position is contained in projected triangle            
            closest_point = position_projected;
        }
        else
        {
            // Otherwise get closest point on triangle
            GetClosestPointOnTriangle2D(Math::Float2(0, 0),
                vertex_0_projected,
                vertex_1_projected,
                vertex_2_projected,
                closest_point,
                distance_2D);
        }


        // Extend distance by z offset to triangle
        auto triangle_z = vertex_0.z;
        Math::Float3 closest_point_unprojected = Math::Float3(closest_point.x, closest_point.y, triangle_z);

        float distance_3D = Math::Norm(closest_point_unprojected);

        return distance_3D;
    }


    // If any triangle of the mesh contains the input position in the X-Y layer,
    // return its index. If multiple triangles do, return the closest one in 3D-Space.
    // Otherwise return unsigned(-1)
    unsigned GetContainingTriangleIndex(Math::Float3 const position_3D,
        NavigationMesh const & mesh)
    {
        // Store triangles containing the position in 2D
        std::vector<unsigned> containing_triangles;
        auto position_2D = Math::Float2(position_3D.x, position_3D.y);

        // Naively check against all triangles
        for (auto i = 0; i < mesh.indices.size() / 3; i++)
        {
            Math::Float2 vertex_0, vertex_1, vertex_2;
            GetMeshTriangleVertices2D(i, mesh, vertex_0, vertex_1, vertex_2);

            if (TriangleContainsPosition(position_2D, vertex_0, vertex_1, vertex_2))
            {
                containing_triangles.push_back(i);
            }
        }

		// Return containing triangle if unambiguous in 2d
		if (containing_triangles.size() == 1) return containing_triangles.front();

        // Otherwise return invalid index if position is not contained in any triangle
        if (containing_triangles.size() == 0) return unsigned(-1);

        // Otherwise multiple triangles contain the position, 
        // so that the closest one must be chosen
        auto minimum_distance = std::numeric_limits<float>::max();
        auto minimum_triangle = unsigned(-1);

        for (auto triangle : containing_triangles)
        {
            Math::Float3 vertex_0, vertex_1, vertex_2;
            GetMeshTriangleVertices3D(triangle, mesh, vertex_0, vertex_1, vertex_2);

            auto current_distance = GetPointDistanceFromTriangle3D(position_3D, vertex_0, vertex_1, vertex_2);

            if (current_distance < minimum_distance)
            {
                minimum_distance = current_distance;
                minimum_triangle = triangle;
            }
        }

        return minimum_triangle;
    }


    // Find all triangles sharing an edge with the input triangle
    void GetAdjacentTriangles(unsigned const input_triangle,
        NavigationMesh const & mesh,
        // output
        std::vector<unsigned> & adjacent_triangles,
        PortalList & adjacency_portals)
    {
        auto triangle_count = mesh.indices.size() / 3;

        // Keep track of which vertices are potentially shared per triangle
        std::array<unsigned, 3> shared_vertices;

        // Find all triangles sharing exactly two vertices with the input triangle
        for (auto i = 0; i < triangle_count; i++)
        {
            auto current_triangle = unsigned(i);
            auto shared_vertex_count = 0;

            // Iterate over vertex indices
            for (auto j = 0; j < 3; j++)
            {
                // Get vertex of input triangle
                auto input_vertex = mesh.indices[input_triangle * 3 + j];

                // Compare to all vertices of current triangle
                for (auto k = 0; k < 3; k++)
                {
                    auto current_vertex = mesh.indices[current_triangle * 3 + k];

                    if (current_vertex == input_vertex)
                    {
                        shared_vertices[shared_vertex_count] = current_vertex;
                        ++shared_vertex_count;
                        break;
                    }
                }
            }

            // If two vertices are shared, the triangle is adjacent
            if (shared_vertex_count == 2)
            {
                // Store the index of the adjacent triangle
                adjacent_triangles.push_back(current_triangle);

                // Store the indices of the two vertices of the shared edge
                adjacency_portals.vertices_0.push_back(shared_vertices[0]);
                adjacency_portals.vertices_1.push_back(shared_vertices[1]);
            }
        }
    }
}