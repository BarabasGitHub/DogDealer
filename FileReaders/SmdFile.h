#pragma once

#include <string>
#include <sstream>

#include <Math\FloatTypes.h>

struct Orientation;

class SmdFile
{
protected:

	bool	FindLine(const std::string & target_line);

	void	Open( std::wstring filename );
    void	Close();
    bool	IsOpen() const;

	std::stringstream					m_file_stream;
	std::streamoff						m_data_start;

    // returns the bone index
    static unsigned ReadBoneOrientation(std::string line, Orientation& output, Math::Float3 & euler_rotation);

	// Prohibit copying to avoid issues with the streams
	SmdFile(const SmdFile& other) = delete;

public:

    SmdFile( std::wstring filename );
	~SmdFile(void);

};