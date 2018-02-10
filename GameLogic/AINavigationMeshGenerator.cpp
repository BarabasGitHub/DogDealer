#include "AINavigationMeshGenerator.h"

#include <Math/MathFunctions.h>

namespace
{
    // Add three indices forming a triangle in right-handed rotational sense
    void AddIndices(std::vector<unsigned>& indices,
        unsigned const a,
        unsigned const b,
        unsigned const c)
    {
        indices.push_back(a);
        indices.push_back(b);
        indices.push_back(c);
    }
}

namespace Logic
{
    // Fill a vector with 2D vertex positions and triangle indices each,
    // Forming a square mesh with a square hole in the center
    void GetHardcodedNavigationMeshA(NavigationMesh & mesh)
    {
        // Define an offset and scale for easier modifications
        Math::Float2 absolute_offset = Math::Float2(10.0f, 10.0f);
        float scale = 7.0f;

        mesh.vertices.clear();
        mesh.vertices.reserve(8);

        mesh.indices.clear();
        mesh.indices.reserve(8 * 3);

        // POSITIONS:
        // Create the outer square, counter-clockwise starting at +x, +y
        float offset = 2.0f;
        mesh.vertices.push_back(Math::Float2(offset, offset));
        mesh.vertices.push_back(Math::Float2(-offset, offset));
        mesh.vertices.push_back(Math::Float2(-offset, -offset));
        mesh.vertices.push_back(Math::Float2(offset, -offset));

        // Create the inner square in the same way
        offset = 1.0f;
        mesh.vertices.push_back(Math::Float2(offset, offset));
        mesh.vertices.push_back(Math::Float2(-offset, offset));
        mesh.vertices.push_back(Math::Float2(-offset, -offset));
        mesh.vertices.push_back(Math::Float2(offset, -offset));

        // Scale and shift all mesh.vertices
        for (auto& pos : mesh.vertices) pos = pos * scale + absolute_offset;

        // INDICES:
        AddIndices(mesh.indices, 0, 1, 5);
        AddIndices(mesh.indices, 0, 5, 4);

        AddIndices(mesh.indices, 0, 4, 7);
        AddIndices(mesh.indices, 0, 7, 3);

        AddIndices(mesh.indices, 7, 6, 2);
        AddIndices(mesh.indices, 7, 2, 3),

        AddIndices(mesh.indices, 1, 2, 5);
        AddIndices(mesh.indices, 5, 2, 6);
    }


    // Create a single, large triangle
    void GetHardcodedNavigationMeshB(NavigationMesh & mesh)
    {
        // The barycentric approach does not support the actual max
        //auto max_value = 0.5f * std::numeric_limits<float>::max();
        auto max_value = 100.0f;

        // INDICES:
        mesh.vertices.push_back(Math::Float2(-max_value, -max_value));
        mesh.vertices.push_back(Math::Float2(max_value, -max_value));
        mesh.vertices.push_back(Math::Float2(0.0f, max_value));

        mesh.indices.push_back(0);
        mesh.indices.push_back(1);
        mesh.indices.push_back(2);
    }

}