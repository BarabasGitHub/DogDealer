#include "ConversionManager.h"

#include "Utilities\StringUtilities.h"

#include <iostream>
#include <string>
#include <vector>


using namespace std;


// Commands:

auto c_command_exit = L"exit";
wstring const c_command_anim_retarget = L"retarget";

enum struct Command
{
    Exit,
    Retarget,
    Mesh,
    Collision,
    Skeleton,
    Animation,
    Invalid
};


wchar_t const * StringFromCommand(Command c)
{
    switch(c)
    {
        case Command::Exit:
            return L"exit";
        case Command::Retarget:
            return L"retarget";
        case Command::Mesh:
            return L"mesh";
        case Command::Collision:
            return L"collision";
        case Command::Skeleton:
            return L"skeleton";
        case Command::Animation:
            return L"animation";
        case Command::Invalid:
        default:
            return nullptr;
    }
}


Command CommandFromString(wstring const & input)
{
    for(Command i = Command(0); i < Command::Invalid; i = Command(std::underlying_type_t<Command>(i) + 1))
    {
        auto command_string = StringFromCommand(i);
        if(input.compare(0, std::wcslen(command_string), command_string) == 0)
        {
            return i;
        }
    }
    return Command::Invalid;
}


void DisplayTooltips()
{
	cout << "#####Available commands: \n";

	cout << "#- <file_path>                                    -- file to be converted\n";
	cout << "#- <folder_path>                                  -- convert all contents\n";
	wcout << "#- " << std::wstring(StringFromCommand(Command::Retarget)) << " <skeleton_smd> <animation_smd_folder> -- retarget animations\n";
    wcout << "#- " << std::wstring(StringFromCommand(Command::Animation)) <<  " <aniamtion> -- create an animation file\n";
    wcout << "#- " << std::wstring(StringFromCommand(Command::Collision)) << " <collision> -- create a collision file\n";
    wcout << "#- " << std::wstring(StringFromCommand(Command::Mesh)) << " <mesh> -- create a mesh file\n";
    wcout << "#- " << std::wstring(StringFromCommand(Command::Skeleton)) << " <skeleton> -- create a skeleton file\n";
	wcout << "#- " << std::wstring(StringFromCommand(Command::Exit)) << "                                           -- exit converter\n";
	cout << "##########\n";
	
}


bool InputIsExitCommand(wstring const & input)
{
	return input.compare(c_command_exit) == 0;
}


bool InputIsAnimationRetargetingCommand(wstring const & input)
{
	return input.compare(0, c_command_anim_retarget.length(), c_command_anim_retarget) == 0;
}


void RunInteractiveMode()
{
    DisplayTooltips();
    wstring input;
    auto const force_convert = true;

    getline( wcin, input );
    std::vector<std::wstring> inputs;
    SplitString(input, inputs);
    auto command = CommandFromString(inputs[0]);
    while( command != Command::Exit )
    {
        switch(command)
        {
            case Command::Retarget:
                RetargetAnimations(inputs[1]);
                break;
            case Command::Mesh:
                ConvertToMesh(inputs[1], force_convert);
                break;
            case Command::Collision:
                ConvertToCollision(inputs[1], force_convert);
                break;
            case Command::Skeleton:
                ConvertToSkeleton(inputs[1], force_convert);
                break;
            case Command::Animation:
                ConvertToAnimation(inputs[1], force_convert);
                break;
            case Command::Invalid:
                if(input.empty())
                {
                    continue;
                }
                else
                {
                    // Convert single file, find out from the file what to convert to
                    ProcessFilePath(input);
                    break;
                }
            case Command::Exit:
                // should never happen
                break;
            default:
                continue;
        }

		getline(wcin, input);
        inputs.clear();
        SplitString(input, inputs);
        command = CommandFromString(inputs[0]);
    }
}


void ReadAllFilesAndFolders(size_t argument_count, wchar_t const * const * arguments )
{
    for( size_t i = 0; i < argument_count; i++ )
    {
        ProcessFilePath( arguments[i] );
    }
}


int wmain(int argc, wchar_t const * argv[])
{
    if( argc == 1 )
    {
        RunInteractiveMode();
    }
    else if (argc > 1 )
    {
        if(argc == 3)
        {
            auto command = CommandFromString(argv[1]);
            switch(command)
            {
                case Command::Exit:
                    return 0;
                case Command::Retarget:
                    RetargetAnimations(argv[2]);
                    break;
                case Command::Mesh:
                    ConvertToMesh(argv[2]);
                    break;
                case Command::Collision:
                    ConvertToCollision(argv[2]);
                    break;
                case Command::Skeleton:
                    ConvertToSkeleton(argv[2]);
                    break;
                case Command::Animation:
                    ConvertToAnimation(argv[2]);
                    break;
                case Command::Invalid:
                default:
                    if(CommandFromString(argv[argc - 1]) == Command::Exit)
                    {
                        ReadAllFilesAndFolders(1, argv + 1);
                    }
                    else
                    {
                        ReadAllFilesAndFolders(2, argv + 1);
                        RunInteractiveMode();
                    }
            }
        }
        else
        {
            ReadAllFilesAndFolders(argc - 1, argv + 1);
            RunInteractiveMode();
        }       
    }
    return 0;
}