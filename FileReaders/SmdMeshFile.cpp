#include "SmdMeshFile.h"

#include <array>
#include <cctype>
#include <fstream>

#include <iostream>

#include <Math\FloatIOStreamOperators.h>
#include <Math\FloatTypes.h>
#include <Math\FloatOperators.h>

using namespace std;
using namespace FileReading;

SmdMeshFile::SmdMeshFile(std::wstring filename):
m_vertex_count(0),
SmdFile(filename)
{
	Open( filename );

	m_file_stream.seekg( 0, ios::beg );

	//Skip to triangle data start
	if (FindLine("triangles\r")){
		ReadVertexData();
	}

	Close();
}


void SmdMeshFile::ReadVertexData(){

	string line;
	//Skip to triangle data start
	m_file_stream.seekg( m_data_start, ios::beg );

	m_vertex_count = 0;

	//////Read Triangle Entries
	while(m_file_stream.good()){

		//Check whether material entry or file end
		m_file_stream >> line;
		if(line == "end") //maybe this should be end\r?
			break;
		getline(m_file_stream,line);

		//////Read Vertex Entries
		for(int i = 0; i<3; i++){
			getline(m_file_stream,line);
			AddVertexEntry(line);
		}
	}
	m_vertex_set.clear();
}

void SmdMeshFile::AddVertexEntry(std::string line){
	Vertex vertex = ReadVertexEntry(line);
	auto result = m_vertex_set.insert(vertex);

	auto index = m_vertex_count;

	// Insertion failed due to duplicate
	// Use duplicate vertex in face index list
	if(!result.second){
		vertex = *(result.first);
		index = vertex.index;

		//scampy removal of redundant weight entry
		m_bone_weights_and_indices.pop_back();
	}else
	{
		m_vertex_count++;
		m_vertices.push_back(vertex);
	}

	m_indices.push_back(index);
}

Vertex SmdMeshFile::ReadVertexEntry(string line){

		Vertex vertex;
		stringstream line_stream(line);

		//Skip root bone index
		line_stream >> line;

		//Read position, normals and UV data of a vertex
		line_stream >> vertex.pos;
		line_stream >> vertex.norm;
		line_stream >> vertex.uv;

		ReadVertexWeightMapping(line_stream);

		vertex.index = m_vertex_count;
		return vertex;
}


void SmdMeshFile::ReadVertexWeightMapping(stringstream& line_stream){

	int mapped_bones = 0;

	line_stream >> mapped_bones;
	if (mapped_bones > 4){

        string line_string = line_stream.str();
        cout << "Invalid bone weights for line: " << line_string.c_str();

		MessageBoxW( nullptr, L"Vertex rigged to more than 4 bones found in smd file.", L"Error", MB_OK );
	}

	//Read each bone index and weight for mapping
    Math::Float4 bone_weight_array(1,0,0,0);
	Math::Unsigned4 bone_index_array(0,0,0,0);

	for (int i = 0; i<mapped_bones; i++)
    {
        line_stream >> bone_index_array[i];
		line_stream >> bone_weight_array[i];
	}
    bone_weight_array = NormalizeWeightset( bone_weight_array );
    BoneWeightsAndIndices bwai = { bone_index_array, bone_weight_array };
	m_bone_weights_and_indices.push_back( bwai );

}


PositionsContainer SmdMeshFile::GetVertexPositions() const
{
	PositionsContainer vector;

	for (auto &vertex : m_vertices)
    {
        vector.push_back( vertex.pos );
	}
	return vector;
}


NormalsContainer SmdMeshFile::GetVertexNormals() const
{
	NormalsContainer vector;

	for (auto &vertex : m_vertices)
    {
		vector.push_back( vertex.norm );
	}
	return vector;
}

UVsContainer SmdMeshFile::GetVertexUV() const{

	UVsContainer vector;

	for (auto &vertex : m_vertices)
    {
		// It seems the smd uses a different layout for uv data,
		// so that the y component has to be inverted
        vector.emplace_back( vertex.uv.x, -vertex.uv.y );
	}

	return vector;
}

ColorsContainer SmdMeshFile::GetVertexColors() const
{
	return ColorsContainer( m_vertices.size(), ColorsContainer::value_type(1,1,1,1) );
}

IndicesContainer const & SmdMeshFile::GetIndices() const {
	return m_indices;
}


D3D_PRIMITIVE_TOPOLOGY SmdMeshFile::GetPrimitiveTopology()
{
	return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}


// Ensures that the weights sum up to 1.0
Math::Float4 SmdMeshFile::NormalizeWeightset(Math::Float4 weight)
{
	weight /= weight.x + weight.y + weight.z + weight.w;
	return weight;
}