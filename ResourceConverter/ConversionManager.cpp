#pragma once
#include "ConversionManager.h"

#include "MeshProcessing.h"
#include "FileTypeFunctions.h"
#include "FileWriter.h"

#include "AnimationRetargeting.h"

#include <Utilities\FileFinder.h>
#include <Utilities\StringUtilities.h>

#include <FileReaders\FileStructs.h>

#include <FileReaders\PlyFile.h>
#include <FileReaders\ObjReader.h>

#include <FileReaders\SmdMeshFile.h>
#include <FileReaders\SmdSkeletonFile.h>
#include <FileReaders\SmdAnimationFile.h>
#include <FileReaders\FileData.h>

#include <FileWriters\SMDFileWriter.h>

#include "OutputFormatting.h"

#include <fstream>
#include <iostream>

#include <windows.h> // For creation of retargeting subfolder

using namespace std;

namespace
{

	FileData ReadPlyFile(wstring file_name)
	{
		auto file_data = FileData();
		// MESH:
		{
			PlyFile mesh(file_name);
			file_data.vertex_positions = mesh.GetVertexPositions();
			file_data.vertex_uv_data = mesh.GetVertexUV();
			file_data.vertex_normals = mesh.GetVertexNormals();
			file_data.vertex_indices = mesh.GetIndices();

			file_data.vertex_colors = mesh.GetVertexColors();
			file_data.topology = mesh.GetPrimitiveTopology();
			file_data.bone_weights_and_indices = mesh.GetBoneWeightsAndIndices();

	        // Oh lord, protect us from the fury of the non-triangulated meshes
	        assert(file_data.vertex_indices.size() % 3 == 0 && "Non-triangulated mesh! Not good!");
		}
		return file_data;
	}


	void ReadObjFiles(wstring file_name, std::vector<FileData> & file_datas)
	{
		std::ifstream file( move(file_name), ios::binary | ios::in );
		FileData file_data;
		uint32_t indices_per_face;
		while(ReadFromObj(file, file_data.vertex_positions, file_data.vertex_indices, indices_per_face, file_data.name))
		{
			switch (indices_per_face)
			{
			case 2:
			    file_data.topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
			    break;
			case 3:
			    file_data.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			    break;
			default:
			    file_data.topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
			    break;
			}
			file_datas.emplace_back(move(file_data));
	        file_data = FileData();
		}
	}


	FileData ReadSmdFile(wstring file_name)
	{
		auto file_data = FileData();

		// MESH:
		{
			SmdMeshFile mesh(file_name);
			file_data.vertex_positions = mesh.GetVertexPositions();
			file_data.vertex_uv_data = mesh.GetVertexUV();
			file_data.vertex_normals = mesh.GetVertexNormals();
			file_data.vertex_indices = mesh.GetIndices();

			file_data.vertex_colors = mesh.GetVertexColors();
			file_data.topology = mesh.GetPrimitiveTopology();
			file_data.bone_weights_and_indices = mesh.GetBoneWeightsAndIndices();
		}
		// SKELETON:
		{
			SmdSkeletonFile skeleton(file_name);
			file_data.bone_orientations = skeleton.GetBoneOrientations();
			file_data.bone_parent_indices = skeleton.GetBoneParentIndices();
			
			file_data.bone_orientation_euler_rotations = skeleton.GetBoneOrientationEulerRotations();

			file_data.bone_names = skeleton.GetBoneNames();
		}
		// ANIMATION:
		{
			SmdAnimationFile animation(file_name);
			file_data.keyframes = animation.GetKeyframes();
			file_data.bone_states = animation.GetBoneSates();

			file_data.bone_state_euler_rotations = animation.GetBoneStateEulerRotations();
		}
		return file_data;
	}

	wstring CleanFilePath(wstring file_name)
	{
		// remove apostrophes if found
		if (file_name.front() == '\"' && file_name.back() == '\"')
		{
			file_name.pop_back();
			file_name.erase(begin(file_name));
		}
		return file_name;
	}


	void ReadFile(wstring const & file_name, std::vector<FileData> & file_datas)
	{
		auto type = DetermineFileType(file_name);

		wcout << output_process_prefix << "reading " << file_name << endl;

		switch(type)
		{
			case FileType::Ply:
			{
				file_datas.push_back(ReadPlyFile(file_name));
				break;
			}
			case FileType::Smd:
			{
				file_datas.push_back(ReadSmdFile(file_name));
				break;
			}
			case FileType::Obj:
			{
				ReadObjFiles(file_name, file_datas);
				break;
			}
			default:
			{
				return;
			}
		}

		for( auto & file_data : file_datas )
		{
	    	file_data = ProcessMesh( move(file_data) );
	    };

		wcout << output_process_prefix << "finished." << endl;
	}


	void SplitInputAtSpaces(wstring const input, std::vector<wstring> & strings)
	{
		// Find all occurences of the delimiting character
		std::vector<size_t> delimit_indices;
		auto delimiter = L" ";

		// Search first space
		size_t found = input.find(delimiter);
		if (found != std::string::npos) delimit_indices.push_back(found);

		// Search consecutive spaces
		while (found != std::string::npos)
		{
			found = input.find(delimiter, found + 1);
			if (found != std::string::npos) delimit_indices.push_back(found);
		}

		// Add the end of the input as final delimiting index
		delimit_indices.push_back(input.size());
				
		// Extract substrings		
		int last_index = -1;
		for (auto index : delimit_indices)
		{
			auto string_length = int(index) - last_index - 1;
			auto substring = input.substr(last_index + 1, string_length);

			last_index = int(index);

			strings.push_back(substring);
		}	
	}

	// TODO: This could be moved into the StringUtilities
	wstring GenerateRetargetedOutputPath(wstring input)
	{
		auto path_ending = input.rfind(L"\\");
		
		// Get file name without slash
		auto file_name = input.substr(path_ending + 1, input.length() - path_ending - 1);

		input.resize(path_ending + 1);

		input.append(L"retargeted\\");
		input.append(file_name);

		return input;
	}

	void GenerateRetargetedOutputFolder(wstring input)
	{
		input.append(L"\\retargeted\\");
		CreateDirectoryW(input.c_str(), NULL);
	}

    // Return a vector containing all the files contained in the input path
    // excludes folders named "retarget" in order to not re-retarget them accidentally
    void ExtractDirectoryFilePaths(wstring path, std::vector<FileDescription> & files)
    {
        path = CleanFilePath(move(path));
        FindFiles(path, files);

        // Inspect all subdirectories
        auto directories = FindDirectories(path);
        for(auto & directory : directories)
        {
            // Skip the parent and this directory
            if(directory.compare(directory.length() - 3, 3, L"\\..") == 0 ||
               directory.compare(directory.length() - 2, 2, L"\\.") == 0)
            {
                continue;
            }

            // Exclude retarget directory
            if(directory.compare(directory.length() - 11, 11, L"\\retargeted") == 0) continue;

            // Ensure directory ends with '\'
            if(directory.back() != L'\\') directory += L"\\";

            // Add *, to find all files & folders in the folder
            directory += L"*";
            ExtractDirectoryFilePaths(directory, files);
        }
    }

}


void ProcessFilePath(std::wstring directory)
{
    std::vector<FileDescription> files;
    ExtractDirectoryFilePaths(CleanFilePath(move(directory)), files);

    std::vector<FileData> file_data;
	for( auto i = 0u; i < files.size(); ++i )
	{
	    if(!CheckIfOutputFilesAreNewer(files[i]))
	    {
	    	ReadFile(files[i].full_name, file_data);
	    	SaveAllFiles(file_data, DiscardExtension(files[i].full_name));
            file_data.clear();
	    }
	}
}


namespace
{
    void ConvertToSomeFile(std::wstring file_name, bool force, wchar_t const * extention /*including point*/, void(*save_mesh_file_function)(FileData const & file_data, std::wstring output_path))
    {
        auto file_description = GetFileDescription(file_name);
        auto output_file_name = DiscardExtension(move(file_name));
        if(!force)
        {
            auto output_file_description = GetFileDescription(output_file_name + extention);
            if(file_description.last_write_time < output_file_description.last_write_time)
            {
                return;
            }
        }
        std::vector<FileData> file_data;
        ReadFile(file_description.full_name, file_data);
        save_mesh_file_function(file_data.front(), output_file_name);    
    }
}

void ConvertToMesh(std::wstring file_name, bool force)
{
    ConvertToSomeFile(move(file_name), force, L".mesh", SaveMeshFile);
}


void ConvertToCollision(std::wstring file_name, bool force)
{
    auto file_description = GetFileDescription(file_name);
    auto output_file_name = DiscardExtension(move(file_name));
    if(!force)
    {
        auto output_file_description = GetFileDescription(output_file_name + L".collision");
        if(file_description.last_write_time < output_file_description.last_write_time)
        {
            return;
        }
    }
    std::vector<FileData> file_data;
    ReadFile(file_description.full_name, file_data);
    if(file_data.size() == 1)
    {
        if(GetExtension(file_description.full_name) == L".obj")
        {
            SaveCollisionFile(file_data.front(), output_file_name);
        }
        else
        {
            SaveCollisionFileAsOrientedBox(file_data.front(), output_file_name);
        }
        
    }
    else
    {
        SaveCollisionFile(file_data, output_file_name);
    }
}



void ConvertToSkeleton(std::wstring file_name, bool force)
{
    ConvertToSomeFile(move(file_name), force, L".skel", SaveSkeletonFile);
}


void ConvertToAnimation(std::wstring file_name, bool force)
{
    ConvertToSomeFile(move(file_name), force, L".anim", SaveAnimationFile);
}


// Input:
//	-path to new skeleton smd
//  -path to animation folder
void RetargetAnimations(wstring input)
{
	// Get individual string segments from input
	std::vector<wstring> input_strings;
	SplitInputAtSpaces(input, input_strings);

	assert(input_strings.size() == 3);

	auto skeleton_file_path = CleanFilePath(move(input_strings[1]));
	auto target_folder_path = CleanFilePath(move(input_strings[2]));

	// Read the new rig that the animations should be retargeted to 
	FileData skeleton_data = ReadSmdFile(skeleton_file_path);
	
	// Find all animations in the input directory
	std::vector<FileDescription> animation_file_paths;
	ExtractDirectoryFilePaths(target_folder_path, animation_file_paths);
	
	// Create output folder if it does not exist yet
	GenerateRetargetedOutputFolder(target_folder_path);
	
	// Retarget animations
	for (auto const & file : animation_file_paths)
	{
		// Skip files that are no smds 		
		auto file_type = DetermineFileType(file.full_name);
		if (file_type != FileType::Smd) continue;

		// Otherwise get animation data and skip if none exists
		FileData animation_data = ReadSmdFile(file.full_name);
		if (animation_data.keyframes.size() == 0) continue;

		// Otherwise determine the transformation represented by the animation
		auto transformation_data = CalculateAnimationTransformation(animation_data);

		// Apply the transformation to the new skeleton
		auto retargeted_animation = ApplyTransformationToSkeleton(skeleton_data, transformation_data);

		// Write retargeted animation to file
		auto output_file_path = GenerateRetargetedOutputPath(file.full_name);

		wcout << "Writing " << output_file_path << "..." << std::endl;
		SaveSMDFile(output_file_path, retargeted_animation);
	}

	wcout << "All found animations have been retargeted." << std::endl;
}