package.path = package.path .. ";" .. basedir .. "/lua/scripts/?.lua"
require "graphics_components"
require "animating_components"
require "physics_components"

local function create_file_filter_from_file_names( file_names )
  local filter_string = "files:"
  for i,v in ipairs(file_names) do
    if i == 1 then
      filter_string = filter_string .. v
    else
      filter_string = filter_string .. " or " .. v
    end
  end
  return filter_string
end


local function names_to_file_names( names )
  for i,v in ipairs(names) do
    names[i] = basedir .. "/Resources/"..v..".*"
  end
  return names
end


project "Resources"
  kind "Utility"
  files { basedir .. "/Resources/*.ply", basedir .. "/Resources/*.smd", basedir .. "/Resources/*.obj",  basedir .. "/Resources/Textures/*.dds" }
  links { "ResourceConverter" }
  local mesh_files = names_to_file_names(GetAllMeshNames())
  local collision_files = names_to_file_names(GetAllCollisionNames())
  local skeleton_files = names_to_file_names(GetAllSkeletonNames())
  local animation_files = names_to_file_names(GetAllAnimationNames())
  vpaths {
    ["Mesh"] = mesh_files,
    ["Collision"] = collision_files,
    ["Skeleton"] = skeleton_files,
    ["Animation"] = animation_files,
    ["Textures"] = "**.dds",
  }
  removeplatforms { "UnitTest" }

  resources_output_dir = basedir .. "/Resources/"

  filter(create_file_filter_from_file_names(mesh_files))
     buildrule {
       description = 'Converting %{file.relpath} to mesh...',
       commands = {
         "\"%{cfg.targetdir}/ResourceConverter\" mesh %{file.abspath}",
       },
       outputs = { resources_output_dir .. "%{file.basename}.mesh"}
     }
  filter(create_file_filter_from_file_names(collision_files))
     buildrule {
       description = 'Converting %{file.relpath} to collision...',
       commands = {
         "\"%{cfg.targetdir}/ResourceConverter\" collision %{file.abspath}",
       },
       outputs = { resources_output_dir .. "%{file.basename}.collision" }
     }
  filter(create_file_filter_from_file_names(skeleton_files))
     buildrule {
       description = 'Converting %{file.relpath} to skeleton...',
       commands = {
         "\"%{cfg.targetdir}/ResourceConverter\" skeleton %{file.abspath}"
       },
       outputs = { resources_output_dir .. "%{file.basename}.skel", }
     }
  filter(create_file_filter_from_file_names(animation_files))
     buildrule {
       description = 'Converting %{file.relpath} to animation...',
       commands = {
         "\"%{cfg.targetdir}/ResourceConverter\" animation %{file.abspath}"
       },
       outputs = { resources_output_dir .. "%{file.basename}.anim" }
     }
