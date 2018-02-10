-- premake5.lua

-- base directory of the DogDealer
basedir = ".."

function create_cpp_file_names_in_dir_and_subdirs( directory )
  return {basedir .. "/" .. directory .. "/**.h", basedir .. "/" .. directory .. "/**.cpp", basedir .. "/" .. directory .. "/**.inl"}
end


function create_cpp_file_names_in_dir_only( directory )
  return {basedir .. "/" .. directory .. "/*.h", basedir .. "/" .. directory .. "/*.cpp", basedir .. "/" .. directory .. "/*.inl"}
end


function create_natvis_file_names_in_dir( directory )
  return {basedir .. "/" .. directory .. "/*.natvis"}
end


workspace "DogDealer"
   configurations { "Debug", "ReleaseWithAssert", "Release" }
   platforms { "Application", "UnitTest" }

   system "Windows"
   architecture "x64"

   language "C++"
   location(basedir.."/Project")
   targetdir(basedir .. "/bin/%{cfg.platform}/%{cfg.buildcfg}")
   objdir(basedir .. "/build/%{cfg.platform}/%{cfg.buildcfg}")
   debugdir(basedir)

   flags { "FatalWarnings", "MultiProcessorCompile", "LinkTimeOptimization", "NoIncrementalLink", "NoPCH" }
   warnings "Extra"
   symbols "Default"
   -- entrypoint "WinMainCRTStartup"
   -- entrypoint "WinMain"

   includedirs {basedir .. ""}

   -- disable dll warnings
   disablewarnings {4251}
   -- disable unsecure function warnings
   disablewarnings {4996}

   filter "configurations:Debug"
      defines { "DEBUG" }
      optimize "Debug"

   filter "configurations:ReleaseWithAssert"
      -- defines { "NDEBUG" }
      optimize "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"


   project "DogDealer"
      kind "WindowedApp"
      files(create_cpp_file_names_in_dir_only("."))
      links { "DogDealerScripting", "DogDealerInput", "Resources" }
      libdirs { basedir .. "/lua/lib" }
      links { "lua52" }
      removeplatforms { "UnitTest" }

   project "ResourceConverter"
      kind "ConsoleApp"
      files(create_cpp_file_names_in_dir_and_subdirs("ResourceConverter"))
      files(create_cpp_file_names_in_dir_and_subdirs("FileWriters"))
      files(create_cpp_file_names_in_dir_and_subdirs("FileReaders"))
      links { "DogDealerBoundingShapes", "DogDealerUtilities", "DogDealerMath" }
      removeplatforms { "UnitTest" }

   dofile("resources.lua")

   project "DogDealerAnimating"
      kind "SharedLib"
      files(create_cpp_file_names_in_dir_and_subdirs("Animating"))
      removefiles {basedir .. "/Animating/UnitTests/**"}
      links { "DogDealerConventions", "DogDealerUtilities", "DogDealerMath" }
      defines { "%{prj.name}_DLL_EXPORT" }
      filter "platforms:UnitTest"
        kind "StaticLib"

   project "DogDealerGameLogic"
      kind "SharedLib"
      files(create_cpp_file_names_in_dir_and_subdirs("GameLogic"))
      removefiles {basedir .. "/GameLogic/UnitTests/**"}
      links { "DogDealerConventions", "DogDealerUtilities", "DogDealerMath", "DogDealerInput" }
      defines { "%{prj.name}_DLL_EXPORT" }
      filter "platforms:UnitTest"
        kind "StaticLib"

   project "DogDealerGraphics"
      kind "SharedLib"
      files(create_cpp_file_names_in_dir_and_subdirs("Graphics"))
      files(create_cpp_file_names_in_dir_and_subdirs("external"))
      links { "DogDealerConventions", "DogDealerBoundingShapes", "DogDealerUtilities", "DogDealerMath" }
      defines {"%{prj.name}_DLL_EXPORT"}
      removeplatforms { "UnitTest" }
      -- why does this vpaths stuff not work here!?
      vpaths {
        ["Shaders/Buffers"] = {"**Buffer.hlsli"},
        ["Shaders/Vertex"] = {"**VertexShader.hlsl"},
        ["Shaders/Pixel"] = {"**PixelShader.hlsl"},
      }
      configuration "windows"
         files { basedir .. "/Shaders/**.hlsl", basedir .. "/Shaders/**.hlsli" }
         -- direct x stuff
         directx_libs = {"d2d1", "dwrite", "d3d11", "dxguid", "d3dcompiler"}
         for _, l in ipairs(directx_libs) do
            libdirs { os.findlib(l) }
            links { l }
         end

         filter "files:**VertexShader.hlsl"
            buildrule {
              description = 'Compiling %{file.relpath}',
              dependencies = {},  -- None for this case, but they would go here
              commands = {
                'fxc.exe /E"main" /O3 /Fo' .. basedir .. '"\\bin\\%{cfg.targetdir}\\Shaders\\%{file.basename}.cso" /T"vs_5_0" /nologo %{file.abspath}'
              },
              outputs = { basedir .. "\\bin\\%{cfg.targetdir}\\Shaders\\%{file.basename}.cso" }
            }
         filter "files:**PixelShader.hlsl"
            buildrule {
              description = 'Compiling %{file.relpath}',
              dependencies = {},  -- None for this case, but they would go here
              commands = {
                'fxc.exe /E"main" /O3 /Fo' .. basedir .. '"\\bin\\%{cfg.targetdir}\\Shaders\\%{file.basename}.cso" /T"ps_5_0" /nologo %{file.abspath}'
              },
              outputs = { basedir .. "\\bin\\%{cfg.targetdir}\\Shaders\\%{file.basename}.cso" }
            }

   project "DogDealerPhysics"
      kind "SharedLib"
      files(create_cpp_file_names_in_dir_and_subdirs("Physics"))
      removefiles {basedir .. "/Physics/UnitTests/**"}
      links { "DogDealerConventions", "DogDealerBoundingShapes", "DogDealerUtilities", "DogDealerMath" }
      defines { "%{prj.name}_DLL_EXPORT" }
      filter "platforms:UnitTest"
        kind "StaticLib"

   project "DogDealerBoundingShapes"
      kind "StaticLib"
      files(create_cpp_file_names_in_dir_and_subdirs("BoundingShapes"))
      removefiles { basedir .. "/BoundingShapes/UnitTests/**" }
      links { "DogDealerUtilities", "DogDealerMath" }

   project "DogDealerConventions"
      kind "StaticLib"
      files(create_cpp_file_names_in_dir_and_subdirs("Conventions"))
      removefiles { basedir .. "/Conventions/UnitTests/**" }
      links { "DogDealerMath" }

   project "DogDealerInput"
      kind "StaticLib"
      files(create_cpp_file_names_in_dir_and_subdirs("Input"))
      links { "DogDealerMath" }

   project "DogDealerMath"
      kind "StaticLib"
      files(create_cpp_file_names_in_dir_and_subdirs("Math"))
      files(create_natvis_file_names_in_dir("Math"))
      removefiles { basedir .. "/Math/UnitTests/**" }
      links { "DogDealerUtilities" }

   project "DogDealerScripting"
      kind "StaticLib"
      files(create_cpp_file_names_in_dir_and_subdirs("Scripting"))
      files { basedir .. "/lua/**.lua" }
      files { basedir .. "/lua/lib/lua52.*" }
      files { basedir .. "/lua/include/*.h", basedir .. "/lua/include/*.hpp" }
      links { "DogDealerConventions", "DogDealerWorld", "DogDealerMath" }
      filter("files:**.dll")
         buildrule {
           description = 'Copying %{file.relpath} to %{cfg.targetdir}...',
           commands = { "copy \"%{file.relpath}\" \"%{cfg.targetdir}/%{file.name}\" /Y" },
           outputs = { "%{cfg.targetdir}/%{file.name}"}
         }

   project "DogDealerUtilities"
      kind "StaticLib"
      files(create_cpp_file_names_in_dir_and_subdirs("Utilities"))
      files(create_cpp_file_names_in_dir_and_subdirs("Windows"))
      files(create_natvis_file_names_in_dir("Utilities"))
      removefiles {basedir .. "/Utilities/UnitTest/*"}

   project "DogDealerWorld"
      kind "StaticLib"
      files(create_cpp_file_names_in_dir_and_subdirs("World"))
      links { "DogDealerConventions", "DogDealerBoundingShapes", "DogDealerGraphics", "DogDealerGameLogic", "DogDealerUtilities", "DogDealerMath", "DogDealerInput", "DogDealerPhysics", "DogDealerAnimating" }

   project "DogDealerAnimatingUnitTests"
      kind "SharedLib"
      files(create_cpp_file_names_in_dir_and_subdirs("/Animating/UnitTests/"))
      includedirs { "$(VCInstallDir)UnitTest/include" }
      libdirs { "$(VCInstallDir)UnitTest/lib" }
      links { "DogDealerAnimating" }
      removeplatforms { "Application" }

   project "DogDealerGameLogicUnitTests"
      kind "SharedLib"
      files(create_cpp_file_names_in_dir_and_subdirs("/GameLogic/UnitTests/"))
      includedirs { "$(VCInstallDir)UnitTest/include" }
      libdirs { "$(VCInstallDir)UnitTest/lib" }
      links { "DogDealerGameLogic" }
      removeplatforms { "Application" }

   project "DogDealerPhysicsUnitTests"
      kind "SharedLib"
      files(create_cpp_file_names_in_dir_and_subdirs("/Physics/UnitTests/"))
      includedirs { "$(VCInstallDir)UnitTest/include" }
      libdirs { "$(VCInstallDir)UnitTest/lib" }
      links { "DogDealerPhysics" }
      removeplatforms { "Application" }

   project "DogDealerBoundingShapesUnitTests"
      kind "SharedLib"
      files(create_cpp_file_names_in_dir_and_subdirs("/BoundingShapes/UnitTests/"))
      includedirs { "$(VCInstallDir)UnitTest/include" }
      libdirs { "$(VCInstallDir)UnitTest/lib" }
      links { "DogDealerBoundingShapes" }
      removeplatforms { "Application" }

   project "DogDealerConventionsUnitTests"
      kind "SharedLib"
      files(create_cpp_file_names_in_dir_and_subdirs("/Conventions/UnitTests/"))
      includedirs { "$(VCInstallDir)UnitTest/include" }
      libdirs { "$(VCInstallDir)UnitTest/lib" }
      links { "DogDealerConventions" }
      removeplatforms { "Application" }

   project "DogDealerMathUnitTests"
      kind "SharedLib"
      files(create_cpp_file_names_in_dir_and_subdirs("/Math/UnitTests/"))
      includedirs { "$(VCInstallDir)UnitTest/include" }
      libdirs { "$(VCInstallDir)UnitTest/lib" }
      links { "DogDealerMath" }
      removeplatforms { "Application" }

