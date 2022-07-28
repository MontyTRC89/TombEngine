local Util = {}

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
