-----
--- Misc Util functions
-- @luautil Util
local Util = {}

--- Adds all built-in functions and types to the global environment.
-- Put simply, this means that you do not have to write out the full name of a function.
-- e.g. Instead of writing
--	local door = TEN.Objects.GetMoveableByName("door_type4_14")
-- You can write
--	local door = GetMoveableByName("door_type4_14")
Util.ShortenTENCalls = function()
	local ShortenInner 

	ShortenInner = function(tab)
		for k, v in pairs(tab) do
			if _G[k] then
				print("WARNING! Key " .. k .. " already exists in global environment!")
			else
				_G[k] = v
				if "table" == type(v) then
					if nil == v.__type then
						ShortenInner(v)
					end
				end
			end
		end
	end
	ShortenInner(TEN)
end

return Util
