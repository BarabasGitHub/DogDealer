#pragma once
#include <string>

// Wololooo

void ProcessFilePath(std::wstring file_name);

void RetargetAnimations(std::wstring file_name);

void ConvertToMesh(std::wstring file_name, bool force = false);
void ConvertToCollision(std::wstring file_name, bool force = false);
void ConvertToSkeleton(std::wstring file_name, bool force = false);
void ConvertToAnimation(std::wstring file_name, bool force = false);