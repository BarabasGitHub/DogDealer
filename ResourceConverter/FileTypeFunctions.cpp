#include "FileTypeFunctions.h"

#include <Utilities\StringUtilities.h>
#include <Utilities\FileFinder.h>

FileType DetermineFileType(std::wstring const & file_name)
{
    // Check file type
    auto extention = GetExtension(file_name);
    if (extention == L".ply")
    {
        return FileType::Ply;
    }

    if (extention == L".smd")
    {
        return FileType::Smd;
    }

    if(extention == L".obj")
    {
        return FileType::Obj;
    }

    auto result = FindDirectories( file_name );
    if( !result.empty() )
    {
        return FileType::Directory;
    }

    return FileType::Unknown;
}
