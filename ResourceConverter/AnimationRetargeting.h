#pragma once

class FileData;

FileData CalculateAnimationTransformation(FileData const & file_data);

FileData ApplyTransformationToSkeleton(FileData skeleton_data, 
									FileData const & transformation_data);