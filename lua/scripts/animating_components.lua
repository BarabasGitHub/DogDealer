AnimatingComponents = {
}


function GetAllSkeletonNames()
    local skeleton_files = {}
    for k,v in pairs(AnimatingComponents) do
      if v.skeleton ~= nil then
        skeleton_files[#skeleton_files + 1] = v.skeleton
      end
    end
    return skeleton_files
end


function GetAllAnimationNames()
    local animation_files = {}
    for _,v in pairs(AnimatingComponents) do
      for _,v in pairs(v.animations) do
         if v.sequence ~= nil then
          animation_files[#animation_files + 1] = v.sequence
        else
          for _,v in pairs(v.sequences) do
            animation_files[#animation_files + 1] = v
          end
        end
      end
    end
    return animation_files
end
