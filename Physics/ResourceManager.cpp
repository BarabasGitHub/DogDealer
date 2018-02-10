#pragma once
#include "ResourceManager.h"

#include <BoundingShapes\FileLayout.h>
#include <BoundingShapes\ShapeType.h>
#include <BoundingShapes\Sphere.h>
#include <BoundingShapes\OrientedBox.h>

#include <Utilities\StreamHelpers.h>
#include <Utilities\IndexedHelp.h>

#include <fstream>


using namespace Physics;
using namespace BoundingShapes;
using namespace std;


namespace
{
    string FilePathFromCollisionName( string const & collision_name )
    {
        auto file_path = "Resources\\" + collision_name + ".collision";
        return file_path;
    }
}


bool ResourceManager::GetLoadedCollisionData( std::string const & file_name, StoredCollisionData & collision_data )
{
    auto result = m_loaded_collision_data.find( file_name );
    auto loaded = result != m_loaded_collision_data.end();
    if( loaded )
    {
        collision_data = result->second;
    }
    return loaded;
}


void ResourceManager::LoadCollisionData( std::string const & file_name, NewCollisionData & collision_data )
{
    ifstream data_stream( FilePathFromCollisionName(file_name), std::ios::in | std::ios::binary );
    assert( data_stream.good() );

    auto file_data = ReadObject<CollisionMeshFileHeader>( data_stream );

    collision_data.axis_aligned_box = file_data.axis_aligned_box;

    std::vector<BoundingShapes::OrientedBox> & boxes = collision_data.oriented_boxes;
    std::vector<BoundingShapes::Sphere> & spheres = collision_data.spheres;
    std::vector<BoundingShapes::AxisAlignedBoxHierarchyMesh> & meshes = collision_data.meshes;

    std::vector<BoundingShapes::ShapeType> shape_types(file_data.number_of_shapes);
    ReadVector(data_stream, shape_types);

    for( auto shape_type : shape_types )
    {
        switch(shape_type)
        {
            case ShapeType::Sphere:
            {
                BoundingShapes::Sphere sphere;
                ReadObject(data_stream, sphere);
                spheres.push_back(sphere);
                break;
            }
            case ShapeType::OrientedBox:
            {
                BoundingShapes::OrientedBox box;
                ReadObject(data_stream, box);
                boxes.push_back(box);
                break;
            }
            case ShapeType::AxisAlignedBoxHierarchyMesh:
            {
                AxisAlignedBoxHierarchyMesh mesh;
                auto const node_count = ReadObject<uint32_t>( data_stream );
                assert( node_count > 0 );
                mesh.nodes.resize( node_count );
                ReadVector( data_stream, mesh.nodes );

                auto const position_count = ReadObject<uint32_t>( data_stream );
                assert( position_count > 0 );
                mesh.vertex_positions.resize( position_count );
                ReadVector( data_stream, mesh.vertex_positions );
                meshes.emplace_back(move(mesh));
                break;
            }
            default:
            {
                assert( false );
            }
        }
    }
}


void ResourceManager::StoreCollisionData( std::string file_name, StoredCollisionData const & collision_data )
{
    m_loaded_collision_data.emplace(move(file_name), collision_data);
}