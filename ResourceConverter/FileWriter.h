#pragma once
#include <FileReaders\FileData.h>
#include <Utilities\FileDescription.h>
#include <vector>

bool CheckIfOutputFilesAreNewer(FileDescription const & input_file_description);
void SaveAllFiles(std::vector<FileData> const & file_datas, std::wstring output_path);
void SaveMeshFile(FileData const & file_data, std::wstring output_path);
void SaveSkeletonFile(FileData const & file_data, std::wstring output_path);
void SaveAnimationFile(FileData const & file_data, std::wstring output_path);
void SaveCollisionFile(std::vector<FileData> const & file_datas, std::wstring output_path);
void SaveCollisionFile(FileData const & file_data, std::wstring output_path);
void SaveCollisionFileAsOrientedBox(FileData const & file_data, std::wstring output_path);