#include "AINavigation.h"

#include "AINavigationMeshGenerator.h"
#include "AINavigationMeshFunctions.h"

#include <array>

#include <Math/MathFunctions.h>
#include <Math/TransformFunctions.h>

namespace
{
    // Return squared norm of vector between a and b
    float GetDistanceBetweenPoints(Math::Float2 const a,
        Math::Float2 const b)
    {
        Math::Float2 offset = b - a;

        return Math::Dot(offset, offset);
    }

    float GetHeuristicCost(Math::Float2 const a,
        Math::Float2 const b)
    {
        return GetDistanceBetweenPoints(a, b);
    }

    // Return the cross product of two vectors spanning a triangle 
    // according to the input points
    float GetTriangleArea(Math::Float2 const a,
        Math::Float2 const b,
        Math::Float2 const c)
    {
        auto ab = b - a;
        auto ac = c - a;

        return Math::Cross(ab, ac);
    }
}

namespace Logic
{  
    // Return index of open node with lowest f cost and remove from open set
    unsigned RetrieveLowestFScoreOpenNode(std::vector<float> const & f_scores,
                                    std::vector<unsigned> & open_set)
    {

        // Initialize minimum cost and index of node
        float minimum = std::numeric_limits<float>::max();
        unsigned minimum_index = unsigned(-1);

        // Initialize node position in open set for later removal
        unsigned open_set_index = unsigned(-1);

        for (auto i = 0; i < open_set.size(); i++)
        {
            auto node = open_set[i];

            if (f_scores[node] < minimum)
            {
                // Store new lowest cost and node
                minimum = f_scores[node];
                minimum_index = node;

                // Store position in open set for removal
                open_set_index = i;
            }
        }

        // Remove node from open set
        open_set.erase(open_set.begin() + open_set_index);

        return minimum_index;
    }


    // Run A* on triangle centers, storing the crossed edges and predecessor
    // of each triangle along which it can be reached with the lowest cost from the start triangle
    void FindShortestTriangleConnection(unsigned const start_triangle,
                                    unsigned const destination_triangle,
                                    NavigationMesh const & mesh,
                                    // Output
                                    std::vector<unsigned> & predecessors,
                                    PortalList & portals)
    {
        unsigned const triangle_count = unsigned(mesh.indices.size() / 3);

        std::vector<unsigned> open_set;
        auto node_is_closed = std::vector<char>(triangle_count, false);

        // Keep track which triangle was reached from which, using which edge portal
        predecessors = std::vector<unsigned>(triangle_count, unsigned(-1));
        portals.vertices_0 = std::vector<unsigned>(triangle_count, unsigned(-1));
        portals.vertices_1 = std::vector<unsigned>(triangle_count, unsigned(-1));

        auto g_scores = std::vector<float>(triangle_count, std::numeric_limits<float>::max());
        auto f_scores = std::vector<float>(triangle_count, std::numeric_limits<float>::max());

        // Get start and end position
        auto start = GetTriangleCentroid(start_triangle, mesh);
        auto destination = GetTriangleCentroid(destination_triangle, mesh);

        // INITIALIZE SEARCH
        open_set.push_back(start_triangle);
        g_scores[start_triangle] = 0.0f;
        f_scores[start_triangle] = GetHeuristicCost(start, destination);
        
        // FURTHER ITERATIONS
        while (!open_set.empty())
        {
            // Get node with lowest f score, sorted to the front of the set
            unsigned current = RetrieveLowestFScoreOpenNode(f_scores, open_set);
            node_is_closed[current] = true;

            // Get position of current triangle
            auto current_position = GetTriangleCentroid(current, mesh);

            // Find neighbouring triangles and shared edges (portals)
            std::vector<unsigned> adjacent_triangles;
            PortalList adjacency_portals;
            GetAdjacentTriangles(current, mesh, adjacent_triangles, adjacency_portals);

            // Terminate if destination triangle is adjacent
            for (auto i = 0; i < adjacent_triangles.size(); i++)
            {
                auto neighbour = adjacent_triangles[i];

                if (neighbour == destination_triangle)
                {
                    portals.vertices_0[destination_triangle] = adjacency_portals.vertices_0[i];
                    portals.vertices_1[destination_triangle] = adjacency_portals.vertices_1[i];

                    predecessors[destination_triangle] = current;

                    return;
                }
            }

            // Otherwise iterate over adjacent vertices
            for (auto i = 0; i < adjacent_triangles.size(); i++)
            {
                auto neighbour = adjacent_triangles[i];

                // Skip vertex if it is in closed set
                if (node_is_closed[neighbour]) continue;

                // Calculate cost of path reaching neighbour from current
                auto neighbour_position = GetTriangleCentroid(neighbour, mesh);
                float new_g_score = g_scores[current] + GetDistanceBetweenPoints(current_position, neighbour_position);

                // Get vertices of portal edge between current and neighbour
                auto portal_vertex_0 = adjacency_portals.vertices_0[i];
                auto portal_vertex_1 = adjacency_portals.vertices_1[i];

                // Update neighbour with new shortest path
                if (new_g_score < g_scores[neighbour])
                {
                    // If current_triangle found for the first time, add to open set
                    if (g_scores[neighbour] == std::numeric_limits<float>::max()) open_set.push_back(neighbour);
                    
                    // Store new shortest path to neighbour from current
                    predecessors[neighbour] = current;

                    // Store which edge, consisting of vertex indices, was crossed 
                    // to reach the current triangle from the preceding one.
                    portals.vertices_0[neighbour] = portal_vertex_0;
                    portals.vertices_1[neighbour] = portal_vertex_1;

                    g_scores[neighbour] = new_g_score;
                    f_scores[neighbour] = new_g_score + GetHeuristicCost(neighbour_position, destination);
                }
            }
        }

        // Search ends unsucessfully        
    }


    // Use the output of the A* search (solved_*), based on the navigation mesh triangles,
    // to reconstruct a list of triangles and crossed edges
    void ReconstructTrianglePath(std::vector<unsigned> const & solved_predecessors,
                            PortalList & solved_portals,
                            unsigned const destination_triangle,
                            // output
                            std::vector<unsigned> & path_triangles,
                            PortalList & path_portals)
    {
        // Get triangles and crossed edges from end to start
        std::vector<unsigned> reverse_triangle_path;
        PortalList reverse_portals;

        // Initialize with the end of the path
        unsigned current = destination_triangle;

        // Store triangles and crossed edges for all predecessors 
        // until reaching start at unsigned(-1)
        while (current != unsigned(-1))
        {
            // Ignore the start triangle
            if(solved_predecessors[current] == unsigned(-1)) break;

            // Store triangle 
            reverse_triangle_path.push_back(current);

            // Get portal vertices
            auto vertex_0 = solved_portals.vertices_0[current];
            auto vertex_1 = solved_portals.vertices_1[current];

            // Store the indices of the vertices forming the portal edge
            reverse_portals.vertices_0.push_back(vertex_0);
            reverse_portals.vertices_1.push_back(vertex_1);

            // Inspect preceding triangle next
            current = solved_predecessors[current];
        }

        // Flip the reverse paths to run from start to end
        auto path_length = reverse_triangle_path.size();

        path_triangles.resize(path_length);

        path_portals.vertices_0.resize(path_length);
        path_portals.vertices_1.resize(path_length);

        for (auto i = 0; i < path_length; i++)
        {
            auto reverse_index = path_length - i - 1;

            path_triangles[i] = reverse_triangle_path[reverse_index];

            path_portals.vertices_0[i] = reverse_portals.vertices_0[reverse_index];
            path_portals.vertices_1[i] = reverse_portals.vertices_1[reverse_index];
        }
    }


    // Use the A* algorithm to find a list of triangles
    // connecting the start to the end triangle on the navigation mesh.
    // Store any crossed edges between two triangles in path_portals
    void FindTrianglePath(unsigned const start_triangle,
                        unsigned const destination_triangle,
                        NavigationMesh const & mesh,
                        // output
                        std::vector<unsigned> & path_triangles,
                        PortalList & path_portals)
    {
        // Reserve storage for the A* output
        std::vector<unsigned> solved_predecessors;
        PortalList solved_portals;

        // Run A*, yielding a list of triangle predecessors and crossed portals per triangle
        FindShortestTriangleConnection(start_triangle, destination_triangle, mesh, solved_predecessors, solved_portals);
        
        // Ensure that the destination was ever reached
        // This should only fail if both start and destination are on disjunct parts of the mesh
        assert(solved_predecessors[destination_triangle] != unsigned(-1) && "Path finding could not reach destination");

        // Use the A* output to reconstruct the list of crossed triangles and portal edges
        ReconstructTrianglePath(solved_predecessors, solved_portals, destination_triangle, path_triangles, path_portals);
    }
    
    
    // ######################################################################
    // ############################## FUNNELING: ############################
    // ######################################################################

    
    // Swap the values of right and left and flip the edge vertex indices in portals
    void FlipAndUpdatePortalEdge(unsigned const portal_index,
                            // output
                            PortalList & portals,
                            Math::Float2 & right,
                            Math::Float2 & left)
    {
        // Flip portal edge indices
        auto temp_index = portals.vertices_0[portal_index];
        portals.vertices_0[portal_index] = portals.vertices_1[portal_index];;
        portals.vertices_1[portal_index] = temp_index;

        // Update right and left vertex position
        auto temp_position = right;
        right = left;
        left = temp_position;
    }
       

    // Construct a path passing through all portal edges and ensure that vertices_0
    // of each edge in path_portals is on the right side of the path
    void PerformFunnelingAndNormalizePortalEdges(Math::Float2 const start,
                                            std::vector<Math::Float2> const & portal_vertices_0,
                                            std::vector<Math::Float2> const & portal_vertices_1,
                                            // output
                                            PortalList & path_portals,
                                            std::vector<Math::Float2> & output_path)
    {
        auto path_length = portal_vertices_0.size();

        // Initialize funnel
        auto apex = start;
        auto right = Math::Float2();
        auto left = Math::Float2();

        auto portal_index = 0;

        while(portal_index < path_length - 1)
        {
            right = portal_vertices_0[portal_index];
            left = portal_vertices_1[portal_index];

            // Determine right and left 
            auto funnel_area = GetTriangleArea(apex, right, left);
            
            // Flip left and right if inverted
            if(funnel_area <= 0.0f)
            {
                FlipAndUpdatePortalEdge(portal_index, path_portals, right, left);
                funnel_area = -funnel_area;
            }

            // Keep tightening the funnel with the next portals
            while(funnel_area >= 0.0f && portal_index < path_length - 1)
            {
                // Otherwise inspect next 
                ++portal_index;

                // Get next portal edge vertices
                auto current_right = portal_vertices_0[portal_index];
                auto current_left = portal_vertices_1[portal_index];

                // Use the apex again to determine left and right vertex
                auto helper_area = GetTriangleArea(apex, current_right, current_left);
                if(helper_area <= 0.0f) FlipAndUpdatePortalEdge(portal_index, path_portals, current_right, current_left);

                // LEFT:
                // Tighten left side
                auto left_area = GetTriangleArea(apex, current_left, left);
                if(left_area > 0.0f) left = current_left;

                // Check for left over right
                funnel_area = GetTriangleArea(apex, right, left);
                if(funnel_area < 0.0f)
                {
                    // Add right to path and make right new apex
                    output_path.push_back(right);
                    apex = right;

                    break;
                }

                // RIGHT:
                // Tighten right side
                auto right_area = GetTriangleArea(apex, right, current_right);
                if(right_area > 0.0f) right = current_right;

                // Check for right over left
                funnel_area = GetTriangleArea(apex, right, left);
                if(funnel_area < 0.0f)
                {
                    // Add left to path and make left new apex
                    output_path.push_back(left);
                    apex = left;

                    break;
                }

                // Update funnel area
                funnel_area = GetTriangleArea(apex, right, left);
            }
        }
    }


    void ExtractPathNodesByFunneling(Math::Float2 const start,
                        Math::Float2 const destination,
                        NavigationMesh const & mesh,
                        PortalList & path_portals,
                        std::vector<Math::Float2> & output_path)
    {  
        auto portal_count = path_portals.vertices_0.size(); 

        // Extract portal vertex positions
        auto portal_vertices_0 = std::vector<Math::Float2>(portal_count);
        auto portal_vertices_1 = std::vector<Math::Float2>(portal_count);
        
        for(auto i = 0; i < portal_count; i++)
        {
            portal_vertices_0[i] = mesh.vertices[path_portals.vertices_0[i]];
            portal_vertices_1[i] = mesh.vertices[path_portals.vertices_1[i]];
        }

        // Ensure that the funneling creates all nodes necessary to reach the destination
        // by adding the destination itself as final portal 'edge'.
        path_portals.vertices_0.push_back(unsigned(-1));
        path_portals.vertices_1.push_back(unsigned(-1));

        // Use a dummy index for the edge flipping to work and remove it later on.
        portal_vertices_0.push_back(destination);
        portal_vertices_1.push_back(destination);

        // Perform funneling
        PerformFunnelingAndNormalizePortalEdges(start, portal_vertices_0, portal_vertices_1, path_portals, output_path);

        // Add destination as final node of the resulting path
        output_path.push_back(destination);

        // Remove dummy index from PortalList
        path_portals.vertices_0.pop_back();
        path_portals.vertices_1.pop_back();
    }


    // ######################################################################
    // ########################### INITIALIZATION: ##########################
    // ######################################################################

    
    // If the start and destination are not on the navigation mesh, 
    // use the closest vertices to each instead, so that the A* algorithm will not fail
    void FindValidStartAndDestination(Math::Float3 const input_start,
                                        Math::Float3 const input_destination,
                                        NavigationMesh const & mesh,
                                        // output
                                        unsigned & out_start_triangle,
                                        unsigned & out_destination_triangle,
                                        Math::Float2 & out_start_2d,
                                        Math::Float2 & out_destination_2d)
    {
        // Work in 3d space to find the closest triangles etc
        Math::Float3 out_start, out_destination;
       
        // Assume that start and destination are valid
        out_start = input_start;
        out_destination = input_destination;
        
        // Identify starting triangle
        out_start_triangle = GetContainingTriangleIndex(input_start, mesh);
        
        // If not on the mesh, start from the closest mesh vertex instead
        if(out_start_triangle == unsigned(-1))
        {            
            GetClosestPointOnMesh(input_start, mesh, out_start, out_start_triangle);
        }

        // Identify destination triangle
        out_destination_triangle = GetContainingTriangleIndex(input_destination, mesh);

        // If not on mesh, move to the closest mesh vertex instead
        if(out_destination_triangle == unsigned(-1))
        { 
            GetClosestPointOnMesh(input_destination, mesh, out_destination, out_destination_triangle);
        }

        // Cast the result positions into 2d.
        // The path finding is still consistent due to working 
        // with the triangle indices and their adjacencies
        out_start_2d = Math::Float2(out_start.x, out_start.y);
        out_destination_2d = Math::Float2(out_destination.x, out_destination.y);
    }


    // Use the A* algorithm on the centroids of a 2d triangle mesh to find a 
    // connected set of triangles connecting the input start and destination.
    // Keep track of which edges were passed and use a funneling algorithm 
    // to simplify the resulting route.
    std::vector<Math::Float2> FindPath(Math::Float3 const input_start,
                                    Math::Float3 const input_destination,
                                    NavigationMesh const & navigation_mesh)
    {
        unsigned start_triangle, destination_triangle;
        Math::Float2 start, destination; // TODO: Cant these be 2D?
        FindValidStartAndDestination(input_start, input_destination, navigation_mesh, start_triangle, destination_triangle, start, destination);
        
        // If on the same triangle try to walk towards destination directly
        if (start_triangle == destination_triangle)
        {
            return std::vector<Math::Float2>(1, destination);
        }

        // Otherwise use the A* algorithm to find a triangle-based path
        PortalList path_portals;
        std::vector<unsigned> path_triangles;
        FindTrianglePath(start_triangle, destination_triangle, navigation_mesh, path_triangles, path_portals);

        // Funneling, transformation to point list
        std::vector<Math::Float2> output_path;
        ExtractPathNodesByFunneling(start,
                        destination,
                        navigation_mesh,
                        path_portals,
                        output_path);

        assert(!output_path.empty() && "No valid path found for following ai entity");

        // Return result path
        return output_path;
    }
}
