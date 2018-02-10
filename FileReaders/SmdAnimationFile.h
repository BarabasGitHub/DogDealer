#include "SmdFile.h"

#include "FileStructs.h"

class SmdAnimationFile : public SmdFile
{
private:

	FileReading::KeyFrameContainer	    m_keyframes;
	FileReading::BoneStateContainer		m_bone_states;
	std::vector<Math::Float3>			m_euler_rotations;

public:
    SmdAnimationFile( std::wstring filename );
	~SmdAnimationFile(void);

	FileReading::KeyFrameContainer const & GetKeyframes() const;
	FileReading::BoneStateContainer	const & GetBoneSates() const;

	std::vector<Math::Float3> const & GetBoneStateEulerRotations() const;

private:
    void				ReadHeader(bool read_keyframes);
	bool				ReadBoneData();
	void				ReadAnimationData();
	bool				SkipToFrames();
};