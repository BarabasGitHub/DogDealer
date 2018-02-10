#pragma once
#include "FileWriter.h"
#include "FileTypeFunctions.h"
#include "OutputFormatting.h"

#include <BoundingShapes\AxisAlignedBoxFunctions.h>
#include <BoundingShapes\AxisAlignedBoxHierarchyMeshFunctions.h>
#include <BoundingShapes\FileLayout.h>
#include <BoundingShapes\OrientedBoxFunctions.h>
#include <BoundingShapes\SphereFunctions.h>
#include <FileLayout\VertexDataType.h>
#include <Graphics\VertexBufferType.h>
#include <Math\MathFunctions.h> // for equality check
#include <Utilities\FileFinder.h>
#include <Utilities\StreamHelpers.h>
#include <Utilities\StringUtilities.h>


#include <fstream>
#include <string>
#include <set>

using namespace std;

namespace
{
    void ExtractUniqueVertexPositions(std::vector<Math::Float3> const & input_vertex_positions,
                                    std::vector<Math::Float3> & unique_vertex_positions)
    {
        unique_vertex_positions = input_vertex_positions;
        std::sort( begin( unique_vertex_positions ), end( unique_vertex_positions ), Math::LessXYZ );
        auto last = std::unique( begin( unique_vertex_positions ), end( unique_vertex_positions ), []( Math::Float3 const & a, Math::Float3 const & b )
        {
            return Math::Equal( a, b, 1e-6f );
        } );
        unique_vertex_positions.resize( last - begin( unique_vertex_positions ) );
    }

    // WRITING
    // MESH:
    std::array<Graphics::VertexBufferType,c_number_of_vertex_data_types> GetUsedVertexData(FileData const & file_data)
    {
        using namespace Graphics;
        auto types = std::array<Graphics::VertexBufferType,c_number_of_vertex_data_types>();

        for(auto & type : types) type = (VertexBufferType::None);

        if(!file_data.vertex_positions.empty()) types[c_positions_index] = (VertexBufferType::Position);
        if(!file_data.vertex_normals.empty())   types[c_normals_index] = (VertexBufferType::Normal);
        if(!file_data.vertex_uv_data.empty())   types[c_texture_index] = (VertexBufferType::Texture);
        if(!file_data.vertex_tangents.empty())  types[c_tangent_index] = (VertexBufferType::Tangent);
        if(!file_data.vertex_colors.empty())    types[c_colors_index] = (VertexBufferType::Color);
        if(!file_data.bone_weights_and_indices.empty()) types[c_bone_index] = (VertexBufferType::Bone);

        return types;
    }

    // Header structure:
    // vertex count, indices count, used type enums
    void WriteMeshFileHeader(ostream& stream, FileData const & file_data)
    {
        MeshHeader header;
        // get info for header
        header.vertex_count = static_cast<unsigned>( file_data.vertex_positions.size() );
        header.index_count = static_cast<unsigned>( file_data.vertex_indices.size() );
        header.topology = file_data.topology;

        header.vertex_types = GetUsedVertexData(file_data);
        // write header
        WriteObject(stream, header);
    }

    void WriteMeshFileData(ostream& stream, FileData const & file_data)
    {
        // Debug writing for non-binary
        /*
        stream << "Vertex positions: ";
        for (auto i = 0u; i < file_data.vertex_positions.size(); i++)
        {
            auto& vertex = file_data.vertex_positions[i];
            stream << "Vertex " << i << ": ";
            stream << vertex.x << " " << vertex.y << " " << vertex.z << " " << endl;
        }

        stream << "Vertex indices: ";
        for (auto i = 0u; i < file_data.vertex_indices.size(); i++)
        {
            auto& index = file_data.vertex_indices[i];
            stream << "index " << i << ": " << index << endl;
        }
        */

        // write axis aligned bounding box
        auto box = BoundingShapes::CreateAxisAlignedBox( file_data.vertex_positions );
        WriteObject( stream, box );

        // Write obligatory indices and positions
        WriteVector(stream, file_data.vertex_indices);
        WriteVector(stream, file_data.vertex_positions);


        // Write all normals facing up for grass
        bool vertical_normals_up = false;

        if (vertical_normals_up)
        {
            auto vertical_normals = std::vector<Math::Float3>(file_data.vertex_normals.size(), Math::Float3(0.0f, 0.0f, 1.0f));
            if (!file_data.vertex_normals.empty())  WriteVector(stream, vertical_normals);
        }
        else
        {
            // Otherwise write actual mesh normals
            if (!file_data.vertex_normals.empty())  WriteVector(stream, file_data.vertex_normals);
        }

        // Write types
        if(!file_data.vertex_uv_data.empty())   WriteVector(stream, file_data.vertex_uv_data);
        if(!file_data.vertex_tangents.empty())  WriteVector(stream, file_data.vertex_tangents);
        if(!file_data.vertex_colors.empty())    WriteVector(stream, file_data.vertex_colors);
        if(!file_data.bone_weights_and_indices.empty()) WriteVector(stream, file_data.bone_weights_and_indices);

    }

    // SKELETON:
    void WriteSkeletonFileHeader(ostream& stream, FileData const & file_data)
    {
        SkeletonHeader header;
        // get info for header
        header.bone_count = static_cast<unsigned>( file_data.bone_orientations.size() );
        // write header
        WriteObject(stream, header);
    }


    void WriteSkeletonFileData(ostream& stream, FileData const & file_data)
    {
        // Write bone orientations
        WriteVector(stream, file_data.bone_orientations);

        // Write bone parent indices
        WriteVector(stream, file_data.bone_parent_indices);
    }

    // ANIMATION:
    void WriteAnimationFileHeader(ostream& stream, FileData const & file_data)
    {
        AnimationHeader header;

        // Get info for header
        header.frame_count = static_cast<unsigned>(file_data.keyframes.size());

        // Get amount of bones per frame, assume all frames having same amount
        header.bone_count = static_cast<unsigned>(file_data.keyframes[0].bone_count);

        // write header
        WriteObject(stream, header);
    }


    void WriteAnimationFileData(ostream& stream, FileData const & file_data)
    {
        // Write keyframes
        //WriteVector(stream, file_data.keyframes);

        // Write bone states
        WriteVector(stream, file_data.bone_states);
    }

    // COLLISION:

    void WriteCollisionFileData( ostream& stream, Range<FileData const *> file_datas )
    {
        assert(!IsEmpty(file_datas));
        using namespace BoundingShapes;

        // auto axis_aligned_box = CreateAxisAlignedBox(First(file_datas).vertex_positions);
        // for( auto i = 1u; i < Size(file_datas); ++i )
        // {
        //     auto new_box = CreateAxisAlignedBox(file_datas[i].vertex_positions);
        //     AxisAlignedBox boxes[2] = {axis_aligned_box, new_box};
        //     axis_aligned_box = Merge(CreateRange(boxes, 2));
        // }

        CollisionMeshFileHeader collision_file_layout;
        // collision_file_layout.axis_aligned_box = axis_aligned_box;
        collision_file_layout.number_of_shapes = uint8_t(Size(file_datas));

        std::vector<ShapeType> shape_types(collision_file_layout.number_of_shapes);
        for( auto i = 0u; i < Size(file_datas); ++i )
        {
            auto & shape_name = file_datas[i].name;
            if(shape_name.compare(0, 6, "Sphere") == 0 || shape_name.compare(0, 9, "Icosphere") == 0)
            {
                shape_types[i] = ShapeType::Sphere;
            }
            else if(shape_name.compare(0, 4, "Cube") == 0)
            {
                shape_types[i] = ShapeType::OrientedBox;
            }
            else
            {
                // assume it's a mesh
                shape_types[i] = ShapeType::AxisAlignedBoxHierarchyMesh;
            }
        }
        // store the position and write the header, just so we can write other stuff after it
        auto starting_position = stream.tellp();
        WriteObject(stream, collision_file_layout);
        WriteVector(stream, shape_types);
        std::array<AxisAlignedBox, 2> axis_boxes;
        axis_boxes[0].extent = 0;
        axis_boxes[0].center = First(First(file_datas).vertex_positions);

        for( auto i = 0u; i < Size(shape_types); ++i )
        {
            switch(shape_types[i])
            {
                case ShapeType::Sphere:
                {
                    auto sphere = CreateSphere(file_datas[i].vertex_positions);
                    WriteObject(stream, sphere);
                    axis_boxes[1].center = sphere.center;
                    axis_boxes[1].extent = sphere.radius;
                    break;
                }
                case ShapeType::OrientedBox:
                {
                    auto box = ReconstructOrientedBoxFromCorners(file_datas[i].vertex_positions);
                    WriteObject(stream, box);
                    axis_boxes[1].center = box.center;
                    axis_boxes[1].extent = box.extent;
                    axis_boxes[1] = RotateAroundCenter(axis_boxes[1], box.rotation);
                    break;
                }
                case ShapeType::AxisAlignedBoxHierarchyMesh:
                {
                    auto mesh = CreateAxisAlignedBoxHierarchyMesh( file_datas[i].vertex_positions, file_datas[i].vertex_indices);
                    WriteObject( stream, unsigned( mesh.nodes.size() ) );
                    WriteVector( stream, mesh.nodes );
                    WriteObject( stream, unsigned( mesh.vertex_positions.size() ) );
                    WriteVector( stream, mesh.vertex_positions );
                    axis_boxes[1] = CreateAxisAlignedBox(mesh.vertex_positions);
                    break;
                }
                default:
                {
                    assert(false);
                }
            }
            axis_boxes[0] = Merge(axis_boxes);
        }
        // now that we have the actual axis_aligned_box we can write the correct header
        collision_file_layout.axis_aligned_box = axis_boxes[0];
        stream.seekp(starting_position);
        WriteObject(stream, collision_file_layout);
    }


    void WriteCollisionFileDataOrientedBox( ostream& stream, FileData const& file_data )
    {
        using namespace BoundingShapes;


        std::vector<Math::Float3> unique_vertex_positions;
        ExtractUniqueVertexPositions(file_data.vertex_positions, unique_vertex_positions);
        auto bob = CreateOrientedBox(unique_vertex_positions);
        auto abba = CreateAxisAlignedBox(unique_vertex_positions);
        BoundingShapes::OrientedBox box;
        if( Volume(bob) < Volume(abba) )
        {
            box = bob;
        }
        else
        {
            box.center = abba.center;
            box.extent = abba.extent;
            box.rotation = Math::Identity();
        }
        CollisionMeshFileHeader collision_file_layout;
        auto corners = GetCorners(box);
        collision_file_layout.axis_aligned_box = CreateAxisAlignedBox(corners);
        collision_file_layout.number_of_shapes = 1;

        WriteObject(stream, collision_file_layout);
        WriteObject(stream, ShapeType::OrientedBox);
        WriteObject(stream, box);
    }

    // TEST READING
    // MESH:
    template<typename Type>
    void ReadVector( ifstream& stream, vector<Type>& vector)
    {
        auto data = vector.data();
        auto number_of_elements = vector.size();

        stream.read( reinterpret_cast<char*>(data), sizeof(Type) * number_of_elements);
    }

    void TestMeshFile(wstring file_path, FileData& file_data)
    {
        using namespace Graphics;

        bool correct = true;

        MeshHeader header = {};
        FileReading::PositionsContainer vertex_positions;
        FileReading::IndicesContainer indices;
        FileReading::NormalsContainer normals;
        FileReading::TangentContainer tangents;
        FileReading::UVsContainer uv;
        FileReading::ColorsContainer colors;
        FileReading::BoneWeightsAndIndicesContainer bones;

        if (auto file = ifstream(file_path, ios::in | ios::binary))
        {
            header = ReadObject<MeshHeader>(file);
            indices.resize(header.index_count);
            ReadVector(file, indices);
            auto index = 0u;
            if( header.vertex_types[index] == VertexBufferType::Position )
            {
                vertex_positions.resize( header.vertex_count );
                ReadVector( file, vertex_positions );
            }
            ++index;
            if (header.vertex_types[index] == VertexBufferType::Normal)
            {
                normals.resize(header.vertex_count);
                ReadVector(file, normals);
            }
            ++index;
            if (header.vertex_types[index] == VertexBufferType::Texture)
            {
                uv.resize(header.vertex_count);
                ReadVector(file, uv);
            }
            ++index;
            if (header.vertex_types[index] == VertexBufferType::Tangent)
            {
                tangents.resize(header.vertex_count);
                ReadVector(file, tangents);
            }
            ++index;
            if (header.vertex_types[index] == VertexBufferType::Color)
            {
                colors.resize(header.vertex_count);
                ReadVector(file, colors);
            }
            ++index;
            if (header.vertex_types[index] == VertexBufferType::Bone)
            {
                bones.resize(header.vertex_count);
                ReadVector(file, bones);
            }
        }

        correct = correct && (file_data.vertex_positions.size() == header.vertex_count);
        correct = correct && (file_data.topology == header.topology);
        // Compare all vectors
        correct = correct && Math::Equal(indices, file_data.vertex_indices);
        correct = correct && Math::Equal(vertex_positions, file_data.vertex_positions);
        correct = correct && Math::Equal(normals, file_data.vertex_normals);
        correct = correct && Math::Equal(uv, file_data.vertex_uv_data);
        correct = correct && tangents.size() == file_data.vertex_tangents.size() &&
            std::equal(tangents.begin(), tangents.end(), file_data.vertex_tangents.begin(), []
        (TangentDirections const & first, TangentDirections const & second)
        {
            return Math::Equal(first.handedness, second.handedness) &&
                Math::Equal(first.tangent, second.tangent) &&
                Math::Equal(first.bitangent, second.bitangent);
        });
        correct = correct && bones.size() == file_data.bone_weights_and_indices.size() &&
            std::equal(bones.begin(), bones.end(), file_data.bone_weights_and_indices.begin(), []
        (BoneWeightsAndIndices const & first, BoneWeightsAndIndices  & second )
        {
            return Math::Equal(first.m_indices, second.m_indices) &&
                Math::Equal(first.m_weights, second.m_weights);
        });

        assert(correct);
    }
}


void SaveAllFiles( std::vector<FileData> const & file_datas, wstring output_path)
{
    wcout << output_write_prefix << "writing " << output_path << "\n";

    for(auto & file_data : file_datas)
    {
        if(!file_data.vertex_positions.empty())
        {
            auto object_name = utf8utf16Converter().from_bytes(file_data.name);
            SaveMeshFile(file_data, output_path + object_name);
        }
        if (!file_data.bone_orientations.empty())
        {
            auto object_name = utf8utf16Converter().from_bytes(file_data.name);
            SaveSkeletonFile(file_data, output_path + object_name);
        }
        if (!file_data.keyframes.empty())
        {
            auto object_name = utf8utf16Converter().from_bytes(file_data.name);
            SaveAnimationFile(file_data, output_path + object_name);
        }
    }

    // if we have multiple things or it has vertex positions and a name
    if( file_datas.size() > 1 || (!file_datas.empty() && !First(file_datas).vertex_positions.empty() && !First(file_datas).name.empty()))
    {
        SaveCollisionFile(file_datas, output_path);
    }
    // if it is just one with vertex positions and without a name
    else if(file_datas.size() == 1 && !First(file_datas).vertex_positions.empty() && First(file_datas).name.empty())
    {
        SaveCollisionFile(First(file_datas), output_path);
    }
}


void SaveMeshFile(FileData const & file_data, std::wstring output_path)
{
    wstring file_path = output_path + L".mesh";
    ofstream file(file_path, ios::out | ios::binary);

    if (file.is_open())
    {
        wcout << output_sub_prefix << "writing .mesh ... \n";

        WriteMeshFileHeader(file, file_data);
        WriteMeshFileData(file, file_data);
    }
}


void SaveSkeletonFile(FileData const & file_data, std::wstring output_path)
{
    wstring file_path = output_path + L".skel";
    ofstream file (file_path, ios::out | ios::binary);

    if (file.is_open())
    {
        wcout << output_sub_prefix << "writing .skel ... \n";

        WriteSkeletonFileHeader(file, file_data);
        WriteSkeletonFileData(file, file_data);
    }
}


void SaveAnimationFile(FileData const & file_data, std::wstring output_path)
{
    wstring file_path = output_path + L".anim";
    ofstream file (file_path, ios::out | ios::binary);

    if (file.is_open())
    {
        wcout << output_sub_prefix << "writing .anim ... \n";

        WriteAnimationFileHeader(file, file_data);
        WriteAnimationFileData(file, file_data);
    }
}


void SaveCollisionFile(vector<FileData> const & file_datas, std::wstring output_path)
{
    wstring file_path = output_path + L".collision";
    ofstream file (file_path, ios::out | ios::binary );

    if( file.is_open() && file.good() )
    {
        wcout << output_sub_prefix << "writing .collision ... \n";
        WriteCollisionFileData(file, file_datas);
    }
}


void SaveCollisionFile(FileData const & file_data, std::wstring output_path)
{
    wstring file_path = output_path + L".collision";
    ofstream file(file_path, ios::out | ios::binary);

    if(file.is_open() && file.good())
    {
        wcout << output_sub_prefix << "writing .collision ... \n";
        WriteCollisionFileData(file, CreateRange(&file_data, 1));
    }
}


void SaveCollisionFileAsOrientedBox(FileData const & file_data, std::wstring output_path)
{
    wstring file_path = output_path + L".collision";
    ofstream file (file_path, ios::out | ios::binary );

    if( file.is_open() && file.good() )
    {
        wcout << output_sub_prefix << "writing .collision ... \n";
        WriteCollisionFileDataOrientedBox(file, file_data);
    }
}


bool CheckIfOutputFilesAreNewer(FileDescription const & input_file_description)
{
    auto file_type = DetermineFileType(input_file_description.full_name);
    auto file_name = DiscardExtension(input_file_description.full_name);

    wchar_t const * extentions[5];
    size_t number_of_extentions = 0;
    switch(file_type)
    {
        case FileType::Obj:
        {
            extentions[0] = L"*.collision";
            extentions[1] = L"*.mesh";
            number_of_extentions = 2;
            break;
        }

        case FileType::Ply:
        {
            extentions[0] = L".collision";
            extentions[1] = L".mesh";
            number_of_extentions = 2;
            break;
        }

        case FileType::Smd:
        {
            extentions[0] = L".collision";
            extentions[1] = L".anim";
            extentions[2] = L".skel";
            extentions[3] = L".mesh";
            number_of_extentions = 4;
            break;
        }

        case FileType::Unknown:
        case FileType::Directory:
        {
            return true;
        }
    }

    auto newer = true;
    for( auto i = 0u; newer && i < number_of_extentions; ++i )
    {
        auto files = FindFiles(file_name + extentions[i]);
        newer = !files.empty();
        for( auto j = 0u; newer && j < files.size(); ++j )
        {
            newer = input_file_description.last_write_time < files[j].last_write_time;
        }
    }
    return newer;
}
