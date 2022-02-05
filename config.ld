file = "Scripting/src"
new_type("mem", "Members", false)
new_type("files", "Script Files", true)
new_type("entityclass", "Entity classes", true)
new_type("pregameclass", "Pre-game script classes", true)
new_type("miscclass", "Misc classes", true)
new_type("tentable", "1 Modules", true)
new_type("tenclass", "2 Classes", true)
new_type("enum", "3 Enums", true)
project = "TombEngine"
title = "TombEngine Lua API"
description = "TombEngine scripting interface"
style = true
wrap = true
template = true
format = 'markdown'
no_space_before_args = true

custom_display_name_handler = function(item, default_handler)
  if item.type == 'files' then
    return item.name .. ' scripts'
  end
  local hand = default_handler(item)
  return hand
end
