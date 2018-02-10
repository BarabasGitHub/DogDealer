#include "NavigationMeshContainer.h"

#include <Math/FloatOperators.h>
#include <FileLayout\VertexDataType.h>
#include <BoundingShapes\AxisAlignedBox.h>

#include <Utilities\StreamHelpers.h>

namespace Logic{

    NavigationMeshID NavigationMeshContainer::LoadNavigationMesh(std::istream& data_stream)
    {
        using namespace std;

        auto header = ReadObject<MeshHeader>(data_stream);

        // This could probably be done better by using the container helpers
        NavigationMeshID id;
        id.index = static_cast<NavigationMeshID::index_t>(m_navigation_meshes.size());

        // Read and discard bounding box
        BoundingShapes::AxisAlignedBox box;
        box = ReadObject<BoundingShapes::AxisAlignedBox>(data_stream);

        m_navigation_meshes.emplace_back();
        auto& mesh = m_navigation_meshes.back();

        // Read indices
        mesh.indices.resize(header.index_count);
        ReadVector(data_stream, mesh.indices);

        // Stream file contents into temporary vector
        std::vector<Math::Float3> vertices3d(header.vertex_count);
        ReadVector(data_stream, vertices3d);

        // Split 3d vertices into 2d and z coordinates
        mesh.vertices.resize(header.vertex_count);
        mesh.vertices_z.resize(header.vertex_count);

        for (auto i = 0u; i < header.vertex_count; i++)
        {
            auto& vertex3d = vertices3d[i];

            mesh.vertices[i] = Math::Float2(vertex3d.x, vertex3d.y);
            mesh.vertices_z[i] = vertex3d.z;
        }

		// Safety measure: Warn against duplicate vertices
		//					as they might indicate an accidentally disjunct mesh part
		std::vector<unsigned> duplicate_indices;
		for (auto i = 0u; i < header.vertex_count; i++)
		{
			auto& vertexA = vertices3d[i];

			for (auto j = i + 1; j < header.vertex_count; j++)
			{
				if (i == j) continue;

				auto& vertexB = vertices3d[j];

				if (vertexA == vertexB) duplicate_indices.push_back(i);
				
			}
		}
		assert(duplicate_indices.empty() && "Duplicate vertices in navigation mesh.");

        return id;
    }
}