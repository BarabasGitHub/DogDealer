#include "SphereHierarchyMeshFunctions.h"

#include "BoundingShapeHierarchyMeshFunctions.h"

#include "SphereFunctions.h"

#include <Math\MathFunctions.h>
#include <Math\FloatOperators.h>

using namespace Math;

namespace BoundingShapes
{
    namespace
    {
        /// from http://en.wikipedia.org/wiki/Principal_component_analysis#Iterative_computation
        Float3 FindPrincipalComponent( Range<Math::Float3 const *> vertex_positions, Range<unsigned *> indices )
        {
            Float3 r = 1;
            auto last_norm = 1.0f;
            for( auto difference = 1.0f; difference < 1e-6f; )
            {
                Float3 s = 0;
                for( auto const i : indices )
                {
                    auto const x = vertex_positions[i];
                    s += Dot( x, r ) * x;
                }

                auto norm = Norm( s );
                r /= norm;
                difference = last_norm - norm;
                last_norm = norm;
            }
            return r;
        }

    }

    SphereHierarchyMesh CreateSphereHierarchyMesh( std::vector<Math::Float3> vertex_positions, std::vector<unsigned> indices )
    {
        return CreateBoundingShapeHierarchyMesh( move( vertex_positions ), move( indices ), FindPrincipalComponent, CreateSphere );
    }


    SphereHierarchyMesh Transform( SphereHierarchyMesh mesh, Math::Float4x4 const & transform )
    {
        mesh = Transform<>( std::move(mesh), transform );
        auto node_count = Size( mesh.nodes );
        for( auto i : CreateIntegerRange( node_count ) )
        {
            auto & node = mesh.nodes[i];
            if( node.escape_index != i + 1 )
            {
                node.shape = Transform( node.shape, transform );
            }
        }
        return mesh;
    }


    SphereHierarchyMesh TransformByOrientation( SphereHierarchyMesh mesh, Orientation const & orientation )
    {
        mesh = TransformByOrientation<>( std::move( mesh ), orientation );
        auto node_count = Size( mesh.nodes );
        for( auto i : CreateIntegerRange( node_count ) )
        {
            auto & node = mesh.nodes[i];
            if( node.escape_index != i + 1 )
            {
                node.shape = TransformByOrientation( node.shape, orientation );
            }
        }
        return mesh;
    }
}