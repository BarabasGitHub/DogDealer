#pragma once

#include <Math\FloatTypes.h>

#include <istream>
#include <vector>

#include <cstdint>

bool ReadFromObj(std::istream & data_stream, std::vector<Math::Float3> & vertices, std::string & object_name);
bool ReadFromObj(std::istream & data_stream, std::vector<Math::Float3> & vertices, std::vector<uint32_t> & face_indices, uint32_t & indices_per_face, std::string & object_name);
// void ReadFromObj(std::istream & data_stream, std::vector<Math::Float3> & vertices, std::vector<Math::Float3> & normals, std::vector<uint32_t> & face_indices);