#include "SmdFile.h"

#include "FileStructs.h"

class SmdSkeletonFile : public SmdFile
{
private:
    FileReading::BoneOrientationContainer   m_bone_orientations;
	FileReading::BoneParentIndexContainer	m_bone_parent_indices;
	std::vector<Math::Float3>				m_bone_euler_rotations;

	std::vector<std::string> m_bone_names;

public:

    SmdSkeletonFile( std::wstring filename );
	~SmdSkeletonFile(void);

	FileReading::BoneOrientationContainer const & GetBoneOrientations() const;
	FileReading::BoneParentIndexContainer const & GetBoneParentIndices() const;
	std::vector<Math::Float3> const & GetBoneOrientationEulerRotations() const;
	std::vector<std::string> const & GetBoneNames() const;

private:
	void				ReadBoneData();
	void				ReadBasePose();
};