#include "SMDFileWriter.h"

#include <FileReaders\FileData.h>

//#include <Utilities\StreamHelpers.h>

#include <Math\TransformFunctions.h>
#include <Math\MathFunctions.h>

#include <fstream>

using namespace std;

namespace
{
	void WriteFloat3(ostream& stream, Math::Float3 const values)
	{
		stream << values[0] << " ";
		stream << values[1] << " ";
		stream << values[2];
	}


	// I have no idea what I'm doing.
	// According to: http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/
	Math::Float3 QuaternionToEuler(Math::Quaternion const q)
	{
		Math::Float3 result;

		auto sqw = q.w * q.w;
		auto sqx = q.x * q.x;
		auto sqy = q.y * q.y;
		auto sqz = q.z * q.z;

		auto unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
		auto test = q.x*q.y + q.z*q.w;
		if (test > 0.499*unit) { // singularity at north pole
			result.y = 2 * atan2(q.x, q.w);
			result.z = float(Math::c_PI) / 2.0f;
			result.x = 0;
			return result;
		}
		if (test < -0.499*unit) { // singularity at south pole
			result.y = -2 * atan2(q.x, q.w);
			result.z = -float(Math::c_PI) / 2.0f;
			result.x = 0;
			return result;
		}

		result.y = atan2(2.0f * q.y*q.w - 2.0f * q.x*q.z, sqx - sqy - sqz + sqw);
		result.z = asin(2.0f * test / unit);
		result.x = atan2(2.0f * q.x*q.w - 2.0f * q.y*q.z, -sqx + sqy - sqz + sqw);

		return result;
	}
}

void WriteSMDFileHeader(ostream& stream, FileData const& file_data)
{	
	stream << "version 1" << std::endl;
	stream << "nodes" << std::endl;

	// Write bone indices, names and parent indices
	for (auto i = 0; i < file_data.bone_orientations.size(); i++)
	{
		auto parent_index = file_data.bone_parent_indices[i];

		stream << i << " " << file_data.bone_names[i] << " " << parent_index << std::endl;
	}

	stream << "end" << std::endl;
	stream << "skeleton" << std::endl;

	stream.flags(std::ios::fixed);
	stream.precision(6);


	stream << "time 0" << std::endl;
		
	// Write rest pose bone states
	for (auto i = 0; i < file_data.bone_orientations.size(); i++)
	{
		stream << i << " ";
			
		auto position = file_data.bone_orientations[i].position;
		//auto rotation = QuaternionToEuler(file_data.bone_orientations[i].rotation);
		auto rotation = file_data.bone_orientation_euler_rotations[i];

		WriteFloat3(stream, position);
		stream << " ";
		WriteFloat3(stream, rotation);

		stream << std::endl;
	}
}



void WriteSMDFileBoneStates(ostream& stream, FileData const& file_data)
{
	auto const frame_count = file_data.keyframes.size();
	auto const bone_count = file_data.keyframes.front().bone_count;

	for (auto f = 0u; f < frame_count; f++)
	{		
		stream << "time " << (f + 1) << std::endl;

		// Write bone states for current frame
		for (auto b = 0u; b < bone_count; b++)
		{
			stream << b << " ";

			auto bone_state_index = f * bone_count + b;

			auto position = file_data.bone_states[bone_state_index].position;
			//auto rotation = QuaternionToEuler(file_data.bone_states[bone_state_index].rotation);
			auto rotation = file_data.bone_state_euler_rotations[bone_state_index];

			WriteFloat3(stream, position);
			stream << " ";
			WriteFloat3(stream, rotation);

			stream << std::endl;
		}
	}

	stream << "end" << std::endl;
}


void WriteSMDData(ostream& stream, FileData const& file_data)
{
	/*
	// TEST:
	auto euler1 = Math::Float3(0.25f * float(Math::c_PI), 0.0f, 0.0f);
	auto quaternion1 = Math::EulerToQuaternion(euler1);
	auto euler_result1 = QuaternionToEuler(quaternion1);

	auto euler2 = Math::Float3(0.0f, 0.25f * float(Math::c_PI), 0.0f);
	auto quaternion2 = Math::EulerToQuaternion(euler2);
	auto euler_result2 = QuaternionToEuler(quaternion2);

	auto euler3 = Math::Float3(0.0f, 0.0f, 0.25f * float(Math::c_PI));
	auto quaternion3 = Math::EulerToQuaternion(euler3);
	auto euler_result3 = QuaternionToEuler(quaternion3);
	*/

	WriteSMDFileHeader(stream, file_data);
	WriteSMDFileBoneStates(stream, file_data);

}


// Only does animations!
void SaveSMDFile(std::wstring const file_path, FileData const & file_data)
{
	ofstream file(file_path, ios::out);

	if (file.is_open() && file.good())
	{		
		WriteSMDData(file, file_data);
	}

	file.close();
}