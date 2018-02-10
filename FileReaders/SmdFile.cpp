
#include "SmdFile.h"
#include <fstream>

#include "FileStructs.h"

#include <Math\FloatIOStreamOperators.h>
#include <Math\TransformFunctions.h>
#include <Math\MathFunctions.h>

using namespace std;
using namespace FileReading;

SmdFile::SmdFile(std::wstring filename)
{
	Open( filename );
}

SmdFile::~SmdFile(void)
{
	if( IsOpen() )
		Close();
}


void SmdFile::Open( wstring filename )
{
    if( IsOpen() )
        Close();

    ifstream file( filename, ios::binary | ios::in );
    if (file)
    {
        m_file_stream << file.rdbuf();
    }
}

void SmdFile::Close()
{
    m_file_stream.clear();
}

bool SmdFile::IsOpen() const
{
    return !m_file_stream.str().empty();
}

bool SmdFile::FindLine(const string & target_line){

	string line;
	while( getline( m_file_stream, line ) )
    {
		if( line.compare( 0, target_line.length(), target_line)==0 )
        {
			m_data_start = m_file_stream.tellg();
            return true;
        }
	}
	return false;

}


unsigned SmdFile::ReadBoneOrientation(std::string line, Orientation & orientation, Math::Float3 & rotation_angles)
{
    using namespace Math;
    stringstream line_stream( move(line) );
    unsigned bone_index;
    line_stream >> bone_index;
    line_stream >> orientation.position;
    //Float3 rotation_angles;
    line_stream >> rotation_angles;
    auto rotation_x = XAngleToQuaternion( rotation_angles.x );
    auto rotation_y = YAngleToQuaternion( rotation_angles.y );
    auto rotation_z = ZAngleToQuaternion( rotation_angles.z );

    // the right order is x y z
    orientation.rotation = Normalize(rotation_z * (rotation_y * rotation_x));
    return bone_index;
}
