#include "ObjReader.h"

#include <Math\FloatIOStreamOperators.h>

#include <string>
#include <sstream>

namespace
{
    void FindLineStartingWith(std::istream & data_stream, std::string const & start)
    {
        std::string line;
        auto position = data_stream.tellg();
        while( getline( data_stream, line ) && line.compare( 0, start.size(), start ) != 0 )
        {
            position = data_stream.tellg();
        }
        data_stream.seekg(position);
    }


    uint32_t CountLinesStartingWith(size_t start_offset, size_t stop_offset, std::istream & data_stream, std::string const & start_string)
    {
        data_stream.seekg(start_offset);
        FindLineStartingWith(data_stream, start_string);
        size_t position = data_stream.tellg();
        std::string line;
        uint32_t count = 0;
        while(position < stop_offset && getline( data_stream, line ))
        {
            count += line.compare( 0, start_string.size(), start_string ) == 0;
            position = data_stream.tellg();
        }
        data_stream.seekg(stop_offset);
        return count;
    }


    uint32_t CountLinesStartingWith(size_t stop, std::istream & data_stream, std::string const & start_string)
    {
        return CountLinesStartingWith(0, stop, data_stream, start_string);
    }


    void FindObject(std::istream & data_stream, std::string & name)
    {
        FindLineStartingWith(data_stream, "o ");
        if(data_stream.good())
        {
            getline(data_stream, name);
            // remove "o "
            name.erase(0, 2);
        }
    }


    void FindFacesStart(std::istream & data_stream)
    {
        FindLineStartingWith(data_stream, "f ");
    }


    void FindLinesStart(std::istream & data_stream)
    {
        FindLineStartingWith(data_stream, "l ");
    }



    void Read(std::istream & data_stream, std::vector<Math::Float3> & data, std::string const & name)
    {
        std::string line;
        auto position = data_stream.tellg();
        while(getline(data_stream, line) && !line.empty() && line.compare(0, name.size(), name) == 0 )
        {
            position = data_stream.tellg();
            // remove "v "
            line.erase(0, 2);
            std::stringstream line_stream( line );
            Math::Float3 vertex;
            line_stream >> vertex;
            data.push_back(vertex);
        }
        data_stream.seekg( position );
    }


    void ReadVertices(std::istream & data_stream, std::vector<Math::Float3> & vertices)
    {
        Read(data_stream, vertices, "v ");
    }


    void ReadNormals(std::istream & data_stream, std::vector<Math::Float3> & normals)
    {
        Read(data_stream, normals, "vn ");
    }


    uint32_t ReadIndices(uint32_t face_index_offset, char line_start /*either f for faces or l for lines*/, std::istream & data_stream, std::vector<uint32_t> & face_indices)
    {
        char line_start_string[3] = {line_start, ' ', '\0'};
        std::string line;
        auto position = data_stream.tellg();
        uint32_t indices_per_face = 0;
        while(getline(data_stream, line) && !line.empty() && line.compare(0, 2, line_start_string) == 0)
        {
            position = data_stream.tellg();
            auto current_indices_per_face = 0u;
            // remove "f "
            line.erase(0, 2);
            for(size_t index = 0; index < line.size(); index = line.find(" ", index + 1))
            {
                auto face_index = std::strtoul(line.c_str() + index, nullptr, 0);
                face_indices.push_back(face_index - face_index_offset);
                current_indices_per_face += 1;
            }
            if( indices_per_face == 0)
            {
                indices_per_face = current_indices_per_face;
            }
            else
            {
                assert(indices_per_face == current_indices_per_face);
            }
        }
        data_stream.seekg( position );
        return indices_per_face;
    }


    uint32_t ReadFaceIndices(uint32_t face_index_offset, std::istream & data_stream, std::vector<uint32_t> & face_indices)
    {
        return ReadIndices(face_index_offset, 'f', data_stream, face_indices);
    }


    uint32_t ReadLineIndices(uint32_t face_index_offset, std::istream & data_stream, std::vector<uint32_t> & face_indices)
    {
        return ReadIndices(face_index_offset, 'l', data_stream, face_indices);
    }


    bool ReadFromObj(uint32_t face_index_offset, std::istream & data_stream, std::vector<Math::Float3> & vertices, std::vector<uint32_t> & face_indices, uint32_t & indices_per_face, std::string & object_name)
    {
        ::ReadFromObj( data_stream, vertices, object_name );
        auto pos = data_stream.tellg();
        FindFacesStart(data_stream);
        auto face_pos = data_stream.tellg();
        auto face_ok = data_stream.good();
        data_stream.clear();
        data_stream.seekg(pos);
        FindLinesStart(data_stream);
        auto line_pos = data_stream.tellg();
        auto line_ok = data_stream.good();
        data_stream.clear();
        if(int(face_pos) != -1 && (int(line_pos) == -1 || face_pos < line_pos))
        {
            data_stream.seekg(face_pos);
            indices_per_face = ReadFaceIndices( face_index_offset, data_stream, face_indices );
            return face_ok;
        }
        else
        {
            data_stream.seekg(line_pos);
            indices_per_face = ReadLineIndices( face_index_offset, data_stream, face_indices );
            return line_ok;
        }
    }
}

bool ReadFromObj(std::istream & data_stream, std::vector<Math::Float3> & vertices, std::vector<uint32_t> & face_indices, uint32_t & indices_per_face, std::string & object_name)
{
    auto offset = CountLinesStartingWith(data_stream.tellg(), data_stream, "v ");
    return ReadFromObj(offset + 1, data_stream, vertices, face_indices, indices_per_face, object_name);
}


// void ReadFromObj(std::istream & data_stream, std::vector<Math::Float3> & vertices, std::vector<Math::Float3> & normals, std::vector<uint32_t> & face_indices)
// {
//     ReadFromObj(data_stream, vertices);
//     ReadNormals(data_stream, normals);
//     // TODO: we somehow have to merge the vertices and normals
//     FindFacesStart(data_stream);
//     ReadFaceIndices(data_stream, face_indices);
// }


bool ReadFromObj(std::istream & data_stream, std::vector<Math::Float3> & vertices, std::string & object_name)
{
    FindObject( data_stream, object_name );
    auto ok = data_stream.good();
    ReadVertices( data_stream, vertices );
    return ok;
}
