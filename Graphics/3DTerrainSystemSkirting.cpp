#pragma once
#include "3dTerrainSystemSkirting.h"

#include "3DTerrainSystemMarchingSquares.h"

#include <Math\FloatOperators.h>
#include <Utilities\IndexUtilities.h>

namespace Graphics
{
    namespace
    {

        void ExtractBottomFaceValues(std::vector<DensityAndGradient> const & density_and_gradients, Math::Unsigned3 grid_size, std::vector<DensityAndGradient> & values)
        {
            auto count = grid_size.x * grid_size.y;
            values.resize(count);
            for (size_t i = 0; i < count; i++)
            {
                values[i] = density_and_gradients[i];
            }
        }


        void ExtractTopFaceValues(std::vector<DensityAndGradient> const & density_and_gradients, Math::Unsigned3 grid_size, std::vector<DensityAndGradient> & values)
        {
            auto count = grid_size.x * grid_size.y;
            values.resize(count);
            auto grid_offset = Calculate1DindexFrom3D({ 0, 0, grid_size.z - 1 }, grid_size.x, grid_size.y);
            for (size_t i = 0; i < count; i++)
            {
                values[i] = density_and_gradients[grid_offset + i];
            }
        }


        void ExtractLeftFaceValues(std::vector<DensityAndGradient> const & density_and_gradients, Math::Unsigned3 grid_size, std::vector<DensityAndGradient> & values)
        {
            auto count = grid_size.y * grid_size.z;
            values.resize(count);
            for (size_t i = 0; i < count; i++)
            {
                values[i] = density_and_gradients[i * grid_size.x];
            }
        }


        void ExtractRightFaceValues(std::vector<DensityAndGradient> const & density_and_gradients, Math::Unsigned3 grid_size, std::vector<DensityAndGradient> & values)
        {
            auto count = grid_size.y * grid_size.z;
            values.resize(count);
            auto grid_offset = Calculate1DindexFrom3D({ grid_size.x - 1, 0, 0 }, grid_size.x, grid_size.y);
            for (size_t i = 0; i < count; i++)
            {
                values[i] = density_and_gradients[grid_offset + i * grid_size.x];
            }
        }


        void ExtractFrontFaceValues(std::vector<DensityAndGradient> const & density_and_gradients, Math::Unsigned3 grid_size, std::vector<DensityAndGradient> & values)
        {
            auto count = grid_size.x * grid_size.z;
            values.resize(count);
            for (size_t z = 0, i = 0; z < grid_size.y * count; z += grid_size.x * grid_size.y)
            {
                for (size_t x = 0; x < grid_size.x; x++, ++i)
                {
                    values[i] = density_and_gradients[x + z];
                }
            }
        }


        void ExtractBackFaceValues(std::vector<DensityAndGradient> const & density_and_gradients, Math::Unsigned3 grid_size, std::vector<DensityAndGradient> & values)
        {
            auto count = grid_size.x * grid_size.z;
            values.resize(count);
            auto grid_offset = Calculate1DindexFrom3D({ 0, grid_size.y - 1, 0 }, grid_size.x, grid_size.y);
            for (size_t z = grid_offset, i = 0; z < grid_size.y * count + grid_offset; z += grid_size.x * grid_size.y)
            {
                for (size_t x = 0; x < grid_size.x; x++, ++i)
                {
                    values[i] = density_and_gradients[x + z];
                }
            }
        }

        template< bool doFlipWindingOrder, typename To3DFunctionType>
        void ExtractFaceVertices(std::vector<DensityAndGradient> const & face_values,
            Math::Unsigned2 const cell_count,
            Math::Float3 const point_offset,
            Math::Float3 const cell_scale,
            To3DFunctionType to_3d_function,
            VertexData& vd)
        {
            // Re-create border vertices
            std::vector<Math::Float2> vertices_2d;
            std::vector<Math::Float3> normals;
            CreateVerticesForTile(cell_count, face_values, c_surface_threshold, vertices_2d, normals);

            // Abort if no border vertices created
            if (vertices_2d.empty()) return;

            // Sort border vertices along isoline
            std::vector<uint32_t> sorted_indices;
            SortVertices(vertices_2d, sorted_indices);

            auto index_offset = uint32_t(vd.positions.size());

            // Translate border vertices into 3d space
            for (auto i : sorted_indices)
            {
                Math::Float3 vertex_3d = to_3d_function(vertices_2d[i]) * cell_scale + point_offset;

                vd.positions.push_back(vertex_3d);
                vd.normals.push_back(normals[i]);
            }

            // Extract extreme border values
            auto first_index = sorted_indices.front();
            auto last_index = sorted_indices.back();

            Math::Float3 skirt_end_0 = to_3d_function(vertices_2d[first_index]) * cell_scale + point_offset;
            Math::Float3 skirt_end_1 = to_3d_function(vertices_2d[last_index]) * cell_scale + point_offset;

            // Create shifted copies to create skirt
            skirt_end_0.z -= 10.0f;
            skirt_end_1.z -= 10.0f;

            vd.positions.push_back(skirt_end_0);
            vd.normals.emplace_back(normals[first_index]);

            vd.positions.push_back(skirt_end_1);
            vd.normals.emplace_back(normals[last_index]);

            auto skirt_vertex_index = uint32_t(vd.positions.size() - 2);

            // Create triangles for all edges:
            // Flip faces when on positive axis face
            for (auto i = index_offset; i < skirt_vertex_index; i++)
            {
                // do not complain about constant conditional expression, that's the whole point of this one
#pragma warning( suppress : 4127 )
                if (doFlipWindingOrder)
                {
                    vd.indices.push_back(i);
                    vd.indices.push_back(i + 1);
                }
                else
                {
                    vd.indices.push_back(i + 1);
                    vd.indices.push_back(i);
                }
                vd.indices.push_back(skirt_vertex_index);
            }

            // Add second face of skirt
            vd.indices.push_back(skirt_vertex_index - 1);

#pragma warning( suppress : 4127 )
            if (doFlipWindingOrder)
            {
                vd.indices.push_back(skirt_vertex_index + 1);
                vd.indices.push_back(skirt_vertex_index);
            }
            else
            {
                vd.indices.push_back(skirt_vertex_index);
                vd.indices.push_back(skirt_vertex_index + 1);
            }
        }
    }


    void ExtractBorderVertices( std::vector<DensityAndGradient> const & local_densities_and_gradients,
                                Math::Unsigned3 const grid_size,
                                Math::Unsigned3 const cell_cube_count,
                                Math::Float3 const cell_scale,
                                Math::Float3 point_offset,
                                VertexData& vd)
    {
        // bottom:	x -> x, y -> y
        // top:		x -> x, y -> y
        // left:	y -> x, z -> y
        // right:	y -> x, z -> y
        // front:	x -> x, z -> y
        // back:	x -> x, z -> y

        std::vector<DensityAndGradient> face_values;
        //// Bottom face
        //{
        //	ExtractBottomFaceValues(local_densities_and_gradients, grid_size, face_values);
        //          auto to_3d_function = []( Math::Float2 const & in )
        //          {
        //              return Math::Float3{ in.x, in.y, 0.f };
        //          };

        //          auto cube_count = Math::Unsigned2{ cell_cube_count.x, cell_cube_count.y };
        //          ExtractFaceVertices<false>( face_values, cube_count, point_offset, cell_scale, to_3d_function, vd );

        //}

        //// Top face
        //{
        //	face_values.clear();

        //	ExtractTopFaceValues(local_densities_and_gradients, grid_size, face_values);
        //          auto extra_z_offset = cell_cube_count.z * cell_scale.z;
        //          auto to_3d_function = [extra_z_offset]( Math::Float2 const & in )
        //          {
        //              return Math::Float3{ in.x, in.y, extra_z_offset };
        //          };

        //          auto cube_count = Math::Unsigned2{ cell_cube_count.x, cell_cube_count.y };
        //          ExtractFaceVertices<true>( face_values, cube_count, point_offset, cell_scale, to_3d_function, vd );
        //}

        // Left face
        {
            face_values.clear();

            ExtractLeftFaceValues(local_densities_and_gradients, grid_size, face_values);
            auto to_3d_function = [](Math::Float2 const & in)
            {
                return Math::Float3{ 0.f, in.x, in.y };
            };

            auto cube_count = Math::Unsigned2{ cell_cube_count.y, cell_cube_count.z };
            ExtractFaceVertices<true>(face_values, cube_count, point_offset, cell_scale, to_3d_function, vd);
        }

        // Right face
        {
            face_values.clear();

            ExtractRightFaceValues(local_densities_and_gradients, grid_size, face_values);
            auto extra_x_offset = float(cell_cube_count.x);
            auto to_3d_function = [extra_x_offset](Math::Float2 const & in)
            {
                return Math::Float3{ extra_x_offset, in.x, in.y };
            };

            auto cube_count = Math::Unsigned2{ cell_cube_count.y, cell_cube_count.z };
            ExtractFaceVertices<false>(face_values, cube_count, point_offset, cell_scale, to_3d_function, vd);
        }

        // Front face
        {
            face_values.clear();

            ExtractFrontFaceValues(local_densities_and_gradients, grid_size, face_values);
            auto to_3d_function = [](Math::Float2 const & in)
            {
                return Math::Float3{ in.x, 0.f, in.y };
            };

            auto cube_count = Math::Unsigned2{ cell_cube_count.x, cell_cube_count.z };
            ExtractFaceVertices<false>(face_values, cube_count, point_offset, cell_scale, to_3d_function, vd);
        }

        // Back face
        {
            face_values.clear();

            ExtractBackFaceValues(local_densities_and_gradients, grid_size, face_values);

            auto extra_y_offset = float(cell_cube_count.y);
            auto to_3d_function = [extra_y_offset](Math::Float2 const & in)
            {
                return Math::Float3{ in.x, extra_y_offset, in.y };
            };

            auto cube_count = Math::Unsigned2{ cell_cube_count.x, cell_cube_count.z };
            ExtractFaceVertices<true>(face_values, cube_count, point_offset, cell_scale, to_3d_function, vd);

        }
    }
}