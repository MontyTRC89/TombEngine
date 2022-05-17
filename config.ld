file = "TombEngine/Scripting/Internal"
new_type("mem", "Members", false)
new_type("files", "Script Files", true)
new_type("entityclass", "Entity classes", true)
new_type("pregameclass", "Pre-game script classes", true)
new_type("miscclass", "Misc classes", true)
new_type("tentable", "1 Modules", true)
new_type("tenclass", "2 Classes", true)
new_type("tenprimitive", "3 Primitive Classes", true)
new_type("enum", "4 Enums", true)
project = "TombEngine"
title = "TombEngine Lua API"
description = "TombEngine scripting interface"
full_description = [[Welcome to the TombEngine scripting API. This is a work in progress and some information might be wrong or outdated. Please also note that this is primarily a reference document, not a tutorial, so expect descriptions to be fairly sparse. For some examples of gameplay setups using these functions, check the example scripts.
####Module Hierarchy (boring but important)
Other than the "special tables" (GameVars, LevelVars and LevelFuncs), every module described herein is held in a master table called TEN.
For example, to call GetMoveableByName, you would have to do:
	local door = TEN.Objects.GetMoveableByName("door_type4_14")
To save on typing, you can put the following at the start of a Lua file:
	local Util = require("Util")
	Util.ShortenTENCalls()
This will put the modules and classes in the global table. In other words, it means you can do the following:
	local door = GetMoveableByName("door_type4_14")

Enjoy.

\- _squidshire_
]]
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
