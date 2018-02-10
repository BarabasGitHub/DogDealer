#include "SmdSkeletonFile.h"

#include <Math\MathFunctions.h>
#include <Math\FloatIOStreamOperators.h>
#include <Math\TransformFunctions.h>

#include <string>
#include <codecvt>

using namespace std;
using namespace FileReading;

namespace
{
    namespace keyword
    {
        const string time = "time";
        const string end  = "end";
    }
}

SmdSkeletonFile::SmdSkeletonFile(std::wstring filename):
    SmdFile( move(filename) )
{
    m_file_stream.seekg( 0, ios::beg );

	if (FindLine("nodes"))
    {
		ReadBoneData();
		if(FindLine("time 0"))
        {
            ReadBasePose();
        }
	}

	Close();
}

SmdSkeletonFile::~SmdSkeletonFile()
{
}

//Read file header until end of node list
void SmdSkeletonFile::ReadBoneData()
{
	m_file_stream.seekg( m_data_start, ios::beg );
	string line;

	while((getline(m_file_stream,line) &&  line.compare( 0, keyword::end.length(), keyword::end ) != 0 ))
    {
		stringstream line_stream(line);
        // discard index and bone name
        unsigned index;
		line_stream >> index;
        auto & bone_name = line;
        line_stream >> bone_name;
		m_bone_names.push_back(bone_name);

        int parent_index;
        line_stream >> parent_index;
		m_bone_parent_indices.push_back( parent_index );
	}
}

//Read all of frame 0
void SmdSkeletonFile::ReadBasePose()
{
	m_file_stream.seekg( m_data_start, ios::beg );
	string line;

	while(getline(m_file_stream,line) &&
		line.compare( 0, keyword::end.length(), keyword::end )!=0 &&
		line.compare( 0, keyword::time.length(), keyword::time )!=0)
	{
		Orientation orientation;
		Math::Float3 euler_rotation;
        ReadBoneOrientation( move(line), orientation, euler_rotation);
		m_bone_orientations.push_back(orientation);
		m_bone_euler_rotations.push_back(euler_rotation);
	}
}


BoneOrientationContainer const & SmdSkeletonFile::GetBoneOrientations() const
{
	return m_bone_orientations;
}

BoneParentIndexContainer const & SmdSkeletonFile::GetBoneParentIndices() const
{
	return m_bone_parent_indices;
}

std::vector<Math::Float3> const & SmdSkeletonFile::GetBoneOrientationEulerRotations() const
{
	return m_bone_euler_rotations;
}

std::vector<std::string> const & SmdSkeletonFile::GetBoneNames() const
{
	return m_bone_names;
}