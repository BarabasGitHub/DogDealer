#include "SmdAnimationFile.h"

#include <fstream>

#include <Math\FloatTypes.h>
#include <Math\TransformFunctions.h>
#include <Math\FloatIOStreamOperators.h>
#include <Math\MathFunctions.h>

#include <Windows\WindowsInclude.h>

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

SmdAnimationFile::SmdAnimationFile(std::wstring filename):
    SmdFile( move(filename) )
{
	m_file_stream.seekg( 0, ios::beg );

	if (SkipToFrames())
    {
		ReadAnimationData();
	}

	Close();
}

SmdAnimationFile::~SmdAnimationFile(void)
{
}
/*
Moves m_data_start pointer to the start of the "time " line of the first frame.
Returns true for found line containing "time", returns false otherwise.
*/
bool SmdAnimationFile::SkipToFrames()
{

	string line;

	//Skip to frame data
	if( FindLine("time 0")){

		m_file_stream.seekg( m_data_start, ios::beg );

		//Skip base pose
		while(getline(m_file_stream,line) && line != "end")
		{
			if( line.compare(0, keyword::time.length(), keyword::time )==0 )
			{
				//Set back reading position to start of line
				m_data_start =  m_file_stream.tellg();
				m_data_start -= line.length() + 1; // + 1 for the endline character
				return true;
			}
		}
	}
	return false;
}


/*
Starts reading the frame entries of the file.
Starting at the first "time x" entry after the base pose.
The first frame's time is set.

Next all frames are read in, adding BoneState entries to each until "end" is read.
*/
void SmdAnimationFile::ReadAnimationData()
{
	m_file_stream.seekg( m_data_start, ios::beg );

	string line;
    Keyframe keyframe;

	// Prepare first keyframe's time
	getline(m_file_stream,line);
	//keyframe.time = std::stoul( line.substr( keyword::time.length() ) );
	keyframe.bone_count = 0;

	// Read bone_state data and all further frames
	while( getline(m_file_stream,line) && line.compare( 0, keyword::end.length(), keyword::end )!=0 )
    {
        if( line.compare( 0, keyword::time.length(), keyword::time )==0 )
        {
			// Complete frame entry at encountered start of new frame
			m_keyframes.push_back(keyframe);

			// Create next keyframe
			keyframe = Keyframe();
			keyframe.bone_count = 0;
			//keyframe.time = std::stoul( line.substr( keyword::time.length() ) );
        }
        else
        {
            // Add bone_state data to current KeyFrame
			m_bone_states.emplace_back();
			m_euler_rotations.emplace_back();

            ReadBoneOrientation( move(line), m_bone_states.back(), m_euler_rotations.back());
			keyframe.bone_count++;
		}
	}

	m_keyframes.push_back(keyframe);
}

KeyFrameContainer const & SmdAnimationFile::GetKeyframes() const
{
	return m_keyframes;
}

BoneStateContainer const & SmdAnimationFile::GetBoneSates() const
{
	return m_bone_states;
}

std::vector<Math::Float3> const & SmdAnimationFile::GetBoneStateEulerRotations() const
{
	return m_euler_rotations;
}
